/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <qglobal.h>

#include "Archive.h"
#include "SceneLoader.h"
#include "World.h"
#include "util.h"

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>
#include <vpvl/vpvl.h>

using namespace vpvl;
using namespace vpvl::gl2;

/*
 * Renderer と Project の二重管理を行うため、メモリ解放にまつわる実装がややこしいことになっている。
 * 解放手順として以下を必ず行うようにする ( Asset と Model のみ)。
 *
 * 1. Project#remove*() を呼び出す
 * 2. Renderer#delete*() を呼び出す
 *
 * このようにするのは Renderer が独自のデータを保持しているため。Project は独自のデータを持たないので、
 * remove*() を呼び出して論理削除しても問題ないが、Renderer は上記の理由により delete*() しか呼び出せない。
 * また、delete*() は引数のポインタを解放後 0 にするという特性を持つため、先に remove*() を呼び出す理由になっている。
 */

namespace
{

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

static const QRegExp &kAssetLoadable = QRegExp(".(bmp|jpe?g|png|sp[ah]|tga|x)$");
static const QRegExp &kAssetExtensions = QRegExp(".x$");
static const QRegExp &kModelLoadable = QRegExp(".(bmp|jpe?g|pmd|png|sp[ah]|tga)$");
static const QRegExp &kModelExtensions = QRegExp(".pmd$");

class UIDelegate : public Project::IDelegate, public Renderer::IDelegate
{
public:
    UIDelegate()
        : m_archive(0),
          m_codec(0)
    {
        m_codec = QTextCodec::codecForName("Shift-JIS");
    }
    virtual ~UIDelegate()
    {
        delete m_archive;
    }

    bool uploadTexture(const std::string &path, GLuint &textureID, bool isToon) {
        QImage image;
        QString pathString = QString::fromLocal8Bit(path.c_str());
        QScopedArrayPointer<uint8_t> ptr(new uint8_t[1]);
        QFileInfo info(pathString);
        pathString.replace(QChar(0xa5), QChar('/')).replace("\\", "/");
        /* ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く) */
        if (m_archive && !pathString.startsWith(":/")) {
            QByteArray suffix = info.suffix().toLower().toUtf8();
            if (suffix == "sph" || suffix == "spa")
                suffix.setRawData("bmp", 3);
            const QByteArray &bytes = m_archive->data(pathString);
            image.loadFromData(bytes, suffix.constData());
            if (image.isNull() && suffix == "tga" && bytes.length() > 18) {
                image = loadTGA(bytes, ptr);
            }
            else {
                image = image.rgbSwapped();
            }
            if (image.isNull()) {
                qWarning("Loading texture %s (zipped) cannot decode", qPrintable(info.fileName()));
                return false;
            }
        }
        /* 通常の読み込み */
        else {
            if (info.isDir()) {
                /* ディレクトリの場合は警告は出さない */
                return false;
            }
            else if (!info.exists()) {
                qWarning("Loading texture %s doesn't exists", qPrintable(info.absoluteFilePath()));
                return false;
            }
            if (image.isNull() && info.suffix().toLower() == "tga") {
                image = loadTGA(pathString, ptr);
            }
            else {
                image = QImage(pathString).rgbSwapped();
            }
            if (image.isNull()) {
                qWarning("Loading texture %s cannot decode", qPrintable(info.absoluteFilePath()));
                return false;
            }
        }
        /* スフィアテクスチャの場合一旦反転する */
        if (pathString.endsWith(".sph") || pathString.endsWith(".spa")) {
            QTransform transform;
            transform.scale(1, -1);
            image = image.transformed(transform);
        }
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        textureID = context->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D,
                                         image.depth() == 32 ? GL_RGBA : GL_RGB, options);
        if (!isToon) {
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID,
               m_archive ? qPrintable(info.fileName()) : qPrintable(pathString));
        return textureID != 0;
    }
    bool uploadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        const QString &filename = QString::fromLocal8Bit(name.c_str());
        QString path = QString::fromLocal8Bit(dir.c_str()) + "/" + filename;
        path.replace("\\", "/");
        /* アーカイブ内にある場合はシステム側のテクスチャ処理とごっちゃにならないように uploadTexture に流して返す */
        if (m_archive && !m_archive->data(path).isEmpty())
            return uploadTexture(std::string(path.toLocal8Bit()), textureID, true);
        /* ファイルが存在しない場合はシステム側のテクスチャと仮定 */
        if (!QFile::exists(path))
            path = QString(":/textures/%1").arg(filename);
        return uploadTexture(std::string(path.toLocal8Bit()), textureID, true);
    }
    const std::string loadShader(Renderer::ShaderType type) {
        QString filename;
        switch (type) {
        case Renderer::kAssetVertexShader:
            filename = "asset.vsh";
            break;
        case Renderer::kAssetFragmentShader:
            filename = "asset.fsh";
            break;
        case Renderer::kEdgeVertexShader:
            filename = "edge.vsh";
            break;
        case Renderer::kEdgeFragmentShader:
            filename = "edge.fsh";
            break;
        case Renderer::kModelVertexShader:
            filename = "model.vsh";
            break;
        case Renderer::kModelFragmentShader:
            filename = "model.fsh";
            break;
        case Renderer::kShadowVertexShader:
            filename = "shadow.vsh";
            break;
        case Renderer::kShadowFragmentShader:
            filename = "shadow.fsh";
            break;
        case Renderer::kZPlotVertexShader:
            filename = "zplot.vsh";
            break;
        case Renderer::kZPlotFragmentShader:
            filename = "zplot.fsh";
            break;
        }
        const QString path = QString(":/shaders/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            file.close();
            qDebug("Loaded a shader: %s", qPrintable(path));
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            qWarning("Failed loading a shader: %s", qPrintable(path));
            return std::string();
        }
    }
    const std::string loadKernel(Renderer::KernelType type) {
        QString filename;
        switch (type) {
        case Renderer::kModelSkinningKernel:
            filename = "skinning.cl";
            break;
        }
        const QString path = QString(":/kernels/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            file.close();
            qDebug("Loaded a kernel: %s", qPrintable(path));
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            qWarning("Failed loading a kernel: %s", qPrintable(path));
            return std::string();
        }
    }
    void log(Renderer::LogLevel level, const char *format, va_list ap) {
        QString message;
        message.vsprintf(format, ap);
        switch (level) {
        case Renderer::kLogInfo:
        default:
            qDebug("%s", qPrintable(message));
            break;
        case Renderer::kLogWarning:
            qWarning("%s", qPrintable(message));
            break;
        }
    }
    const std::string toUnicode(const std::string &value) const {
        return m_codec->toUnicode(value.c_str()).toStdString();
    }
    const std::string toUnicode(const uint8_t *value) const {
        return std::string(internal::toQString(value).toUtf8());
    }
    const std::string fromUnicode(const std::string &value) const {
        const QByteArray &bytes = m_codec->fromUnicode(value.c_str());
        return std::string(bytes.constData(), bytes.length());
    }
    void error(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }
    void warning(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }

    void setArchive(Archive *value) {
        delete m_archive;
        m_archive = value;
    }

private:
    QImage loadTGA(const QString &path, QScopedArrayPointer<uint8_t> &dataPtr) {
        QFile file(path);
        if (file.open(QFile::ReadOnly) && file.size() > 18) {
            return loadTGA(file.readAll(), dataPtr);
        }
        else {
            qWarning("Cannot open file %s: %s", qPrintable(path), qPrintable(file.errorString()));
            return QImage();
        }
    }
    QImage loadTGA(QByteArray data, QScopedArrayPointer<uint8_t> &dataPtr) {
        uint8_t *ptr = reinterpret_cast<uint8_t *>(data.data());
        uint8_t field = *reinterpret_cast<uint8_t *>(ptr);
        uint8_t type = *reinterpret_cast<uint8_t *>(ptr + 2);
        if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
            qWarning("Loaded TGA image type is not full color");
            return QImage();
        }
        uint16_t width = *reinterpret_cast<uint16_t *>(ptr + 12);
        uint16_t height = *reinterpret_cast<uint16_t *>(ptr + 14);
        uint8_t depth = *reinterpret_cast<uint8_t *>(ptr + 16); /* 24 or 32 */
        uint8_t flags = *reinterpret_cast<uint8_t *>(ptr + 17);
        if (width == 0 || height == 0 || (depth != 24 && depth != 32)) {
            qWarning("Invalid TGA image (width=%d, height=%d, depth=%d)",
                     width, height, depth);
            return QImage();
        }
        int component = depth >> 3;
        uint8_t *body = ptr + 18 + field;
        /* if RLE compressed, uncompress it */
        size_t datalen = width * height * component;
        ByteArrayPtr uncompressedPtr(new uint8_t[datalen]);
        if (type == 10) {
            uint8_t *uncompressed = uncompressedPtr.data();
            uint8_t *src = body;
            uint8_t *dst = uncompressed;
            while (static_cast<size_t>(dst - uncompressed) < datalen) {
                int16_t len = (*src & 0x7f) + 1;
                if (*src & 0x80) {
                    src++;
                    for (int i = 0; i < len; i++) {
                        memcpy(dst, src, component);
                        dst += component;
                    }
                    src += component;
                }
                else {
                    src++;
                    memcpy(dst, src, component * len);
                    dst += component * len;
                    src += component * len;
                }
            }
            /* will load from uncompressed data */
            body = uncompressed;
        }
        /* prepare texture data area */
        datalen = (width * height) * 4;
        dataPtr.reset(new uint8_t[datalen]);
        ptr = dataPtr.data();
        for (uint16_t h = 0; h < height; h++) {
            uint8_t *line = NULL;
            if (flags & 0x20) /* from up to bottom */
                line = body + h * width * component;
            else /* from bottom to up */
                line = body + (height - 1 - h) * width * component;
            for (uint16_t w = 0; w < width; w++) {
                uint32_t index = 0;
                if (flags & 0x10)/* from right to left */
                    index = (width - 1 - w) * component;
                else /* from left to right */
                    index = w * component;
                /* BGR or BGRA -> ARGB */
                *ptr++ = line[index + 2];
                *ptr++ = line[index + 1];
                *ptr++ = line[index + 0];
                *ptr++ = (depth == 32) ? line[index + 3] : 255;
            }
        }
        return QImage(dataPtr.data(), width, height, QImage::Format_ARGB32);
    }

    Archive *m_archive;
    QTextCodec *m_codec;
};

/* 文字列を解析して Vector3 を構築する */
static const Vector3 UIGetVector3(const std::string &value, const Vector3 &def)
{
    if (!value.empty()) {
        QStringList gravity = QString::fromStdString(value).split(",");
        if (gravity.length() == 3) {
            float x = gravity.at(0).toFloat();
            float y = gravity.at(1).toFloat();
            float z = gravity.at(2).toFloat();
            return Vector3(x, y, z);
        }
    }
    return def;
}

/* 文字列を解析して Quaternion を構築する */
static const Quaternion UIGetQuaternion(const std::string &value, const Quaternion &def)
{
    if (!value.empty()) {
        QStringList gravity = QString::fromStdString(value).split(",");
        if (gravity.length() == 4) {
            float x = gravity.at(0).toFloat();
            float y = gravity.at(1).toFloat();
            float z = gravity.at(2).toFloat();
            float w = gravity.at(3).toFloat();
            return Quaternion(x, y, z, w);
        }
    }
    return def;
}

#ifdef GL_MULTISAMPLE
static inline void UIEnableMultisample()
{
    glEnable(GL_MULTISAMPLE);
}
#else
#define UIEnableMultisample() (void) 0
#endif

/* レンダリング順序を決定するためのクラス。基本的にプロジェクトの order の設定に依存する */
class UIRenderOrderPredication
{
public:
    UIRenderOrderPredication(Project *project, const Transform &modelViewTransform, bool useOrderAttr)
        : m_project(project),
          m_modelViewTransform(modelViewTransform),
          m_useOrderAttr(useOrderAttr)
    {
    }
    ~UIRenderOrderPredication() {
    }

    bool operator()(const QUuid &left, const QUuid &right) const {
        const Project::UUID &luuid = left.toString().toStdString(), &ruuid = right.toString().toStdString();
        Asset *lasset = m_project->asset(luuid), *rasset = m_project->asset(ruuid);
        PMDModel *lmodel = m_project->model(luuid), *rmodel = m_project->model(ruuid);
        bool lok, rok;
        if (lasset && rasset) {
            int lorder = QString::fromStdString(m_project->assetSetting(lasset, "order")).toInt(&lok);
            int rorder = QString::fromStdString(m_project->assetSetting(rasset, "order")).toInt(&rok);
            if (lok && rok && m_useOrderAttr)
                return lorder < rorder;
        }
        else if (lasset && rmodel) {
            int lorder = QString::fromStdString(m_project->assetSetting(lasset, "order")).toInt(&lok);
            int rorder = QString::fromStdString(m_project->modelSetting(rmodel, "order")).toInt(&rok);
            if (lok && rok && m_useOrderAttr)
                return lorder < rorder;
        }
        else if (lmodel && rasset) {
            int lorder = QString::fromStdString(m_project->modelSetting(lmodel, "order")).toInt(&lok);
            int rorder = QString::fromStdString(m_project->assetSetting(rasset, "order")).toInt(&rok);
            if (lok && rok && m_useOrderAttr)
                return lorder < rorder;
        }
        else if (lmodel && rmodel) {
            int lorder = QString::fromStdString(m_project->modelSetting(lmodel, "order")).toInt(&lok);
            int rorder = QString::fromStdString(m_project->modelSetting(rmodel, "order")).toInt(&rok);
            if (lok && rok && m_useOrderAttr) {
                return lorder < rorder;
            }
            else {
                Bone *lcenter = Bone::centerBone(&lmodel->bones()), *rcenter = Bone::centerBone(&rmodel->bones());
                const Vector3 &positionLeft = m_modelViewTransform * lcenter->localTransform().getOrigin();
                const Vector3 &positionRight = m_modelViewTransform * rcenter->localTransform().getOrigin();
                return positionLeft.z() < positionRight.z();
            }
        }
        return false;
    }

private:
    Project *m_project;
    const Transform m_modelViewTransform;
    const bool m_useOrderAttr;
};

/*
 * ZIP またはファイルを読み込む。複数のファイルが入る ZIP の場合 extensions に
 * 該当するもので一番先に見つかったファイルのみを読み込む
 */
const QByteArray UILoadFile(const QString &filename,
                            const QRegExp &loadable,
                            const QRegExp &extensions,
                            UIDelegate *delegate)
{
    QByteArray bytes;
    if (filename.endsWith(".zip")) {
        QStringList files;
        Archive *archive = new Archive();
        if (archive->open(filename, files)) {
            const QStringList &filtered = files.filter(loadable);
            if (!filtered.isEmpty() && archive->uncompress(filtered)) {
                const QStringList &target = files.filter(extensions);
                if (!target.isEmpty()) {
                    const QString &filenameToLoad = target.first();
                    bytes = archive->data(filenameToLoad);
                    QFileInfo fileToLoadInfo(filenameToLoad), fileInfo(filename);
                    archive->replaceFilePath(fileToLoadInfo.path(), fileInfo.path() + "/");
                    delegate->setArchive(archive);
                }
            }
        }
    }
    else {
        QFile file(filename);
        if (file.open(QFile::ReadOnly))
            bytes = file.readAll();
    }
    return bytes;
}

}

bool SceneLoader::isAccelerationSupported()
{
    return Renderer::isAcceleratorSupported();
}

SceneLoader::SceneLoader(int width, int height, int fps)
    : QObject(),
      m_world(0),
      m_renderer(0),
      m_renderDelegate(0),
      m_project(0),
      m_projectDelegate(0),
      m_model(0),
      m_asset(0),
      m_camera(0)
{
    m_world = new internal::World();
    m_renderDelegate = new UIDelegate();
    m_renderer = new Renderer(m_renderDelegate, width, height, fps);
    m_projectDelegate = new UIDelegate();
    createProject();
}

SceneLoader::~SceneLoader()
{
    release();
    delete m_renderer;
    m_renderer = 0;
    delete m_renderDelegate;
    m_renderDelegate = 0;
    delete m_projectDelegate;
    m_projectDelegate = 0;
}

void SceneLoader::addModel(PMDModel *model, const QString &baseName, const QDir &dir, QUuid &uuid)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    const QString &key = internal::toQString(model).trimmed();
    if (key.isEmpty()) {
        const QByteArray &bytes = internal::fromQString(baseName);
        model->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
    }
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    m_renderer->uploadModel(model, std::string(dir.absolutePath().toLocal8Bit()));
    /* モデルを SceneLoader にヒモ付けする */
    const QString &path = dir.absoluteFilePath(baseName);
    uuid = QUuid::createUuid();
    m_project->addModel(model, uuid.toString().toStdString());
    m_project->setModelSetting(model, Project::kSettingNameKey, key.toStdString());
    m_project->setModelSetting(model, Project::kSettingURIKey, path.toStdString());
    m_project->setModelSetting(model, "selected", "false");
    m_renderOrderList.add(uuid);
    static_cast<UIDelegate *>(m_renderDelegate)->setArchive(0);
    emit modelDidAdd(model, uuid);
}

QList<PMDModel *> SceneLoader::allModels() const
{
    const Project::UUIDList &uuids = m_project->modelUUIDs();
    QList<PMDModel *> models;
    Project::UUIDList::const_iterator it = uuids.begin(), end = uuids.end();
    while (it != end) {
        models.append(m_project->model(*it));
        it++;
    }
    return models;
}

void SceneLoader::commitAssetProperties()
{
    if (m_asset) {
        setAssetPosition(m_asset, m_asset->position());
        setAssetRotation(m_asset, m_asset->rotation());
        setAssetOpacity(m_asset, m_asset->opacity());
        setAssetScaleFactor(m_asset, m_asset->scaleFactor());
        setAssetParentModel(m_asset, m_asset->parentModel());
        setAssetParentBone(m_asset, m_asset->parentBone());
    }
}

void SceneLoader::createProject()
{
    if (!m_project) {
        m_project = new Project(m_projectDelegate);
        /*
         * デフォルトではグリッド表示と物理演算を有効にするため、設定後強制的に dirty フラグを無効にする
         * これによってアプリケーションを起動して何もしないまま終了する際の保存ダイアログを抑制する
         */
        m_project->setGlobalSetting("grid.visible", "true");
        m_project->setGlobalSetting("physics.enabled", "true");
        m_project->setDirty(false);
    }
}

void SceneLoader::deleteAsset(Asset *asset)
{
    /* アクセサリをレンダリングエンジンから削除し、SceneLoader のヒモ付けも解除する */
    if (asset && m_project->containsAsset(asset)) {
        const QUuid uuid(m_project->assetUUID(asset).c_str());
        emit assetWillDelete(asset, uuid);
        m_project->removeAsset(asset);
        m_renderer->deleteAsset(asset);
        m_renderOrderList.remove(uuid);
        m_asset = 0;
    }
}

void SceneLoader::deleteCameraMotion()
{
    /* カメラモーションをシーンから解除及び削除し、最初の視点に戻しておく */
    Scene *scene = m_renderer->scene();
    scene->setCameraMotion(0);
    scene->resetCamera();
    m_project->removeMotion(m_camera, 0);
    delete m_camera;
    m_camera = 0;
}

void SceneLoader::deleteModel(PMDModel *&model)
{
    /*
     * メモリ解放に関しては最初の部分を参照
     *
     * Project にひもづけられるモーションの削除の観点を忘れていたので、
     * モデルに属するモーションを Project から解除するように変更
     */
    if (m_project->containsModel(model)) {
        const QUuid uuid(m_project->modelUUID(model).c_str());
        emit modelWillDelete(model, uuid);
        const Array<VMDMotion *> &motions = model->motions();
        int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            VMDMotion *motion = motions[i];
            m_project->deleteMotion(motion, model);
        }
        m_project->removeModel(model);
        m_renderer->deleteModel(model);
        m_renderOrderList.remove(uuid);
        m_model = 0;
    }
}

void SceneLoader::deleteMotion(VMDMotion *&motion)
{
    const QUuid uuid(m_project->motionUUID(motion).c_str());
    emit motionWillDelete(motion, uuid);
    m_project->deleteMotion(motion, motion->parentModel());
}

Asset *SceneLoader::findAsset(const QUuid &uuid) const
{
    return m_project->asset(uuid.toString().toStdString());
}

PMDModel *SceneLoader::findModel(const QUuid &uuid) const
{
    return m_project->model(uuid.toString().toStdString());
}

VMDMotion *SceneLoader::findMotion(const QUuid &uuid) const
{
    return m_project->motion(uuid.toString().toStdString());
}

const QUuid SceneLoader::findUUID(Asset *asset) const
{
    return QUuid(m_project->assetUUID(asset).c_str());
}

const QUuid SceneLoader::findUUID(PMDModel *model) const
{
    return QUuid(m_project->modelUUID(model).c_str());
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

bool SceneLoader::loadAsset(const QString &filename, QUuid &uuid, Asset *&asset)
{
    UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
    const QByteArray &bytes = UILoadFile(filename, kAssetLoadable, kAssetExtensions, delegate);
    /*
     * アクセサリをファイルから読み込み、レンダリングエンジンに渡してレンダリング可能な状態にする
     */
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        bool allocated = false;
        if (!asset) {
            asset = new Asset();
            allocated = true;
        }
        if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
            QFileInfo fileInfo(filename);
            const QByteArray &assetName = fileInfo.baseName().toUtf8();
            asset->setName(assetName.constData());
            const std::string &name = std::string(fileInfo.absolutePath().toLocal8Bit());
            m_renderer->uploadAsset(asset, name);
            delegate->setArchive(0);
            uuid = QUuid::createUuid();
            m_project->addAsset(asset, uuid.toString().toStdString());
            m_project->setAssetSetting(asset, Project::kSettingNameKey, fileInfo.baseName().toStdString());
            m_project->setAssetSetting(asset, Project::kSettingURIKey, filename.toStdString());
            m_project->setAssetSetting(asset, "selected", "false");
            m_renderOrderList.add(uuid);
            setAssetPosition(asset, asset->position());
            setAssetRotation(asset, asset->rotation());
            setAssetOpacity(asset, asset->opacity());
            setAssetScaleFactor(asset, asset->scaleFactor());
            emit assetDidAdd(asset, uuid);
        }
        else if (allocated) {
            delete asset;
            asset = 0;
        }
    }
    return !isNullData && asset != 0;
}

Asset *SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid)
{
    QFile file(dir.absoluteFilePath(baseName));
    /* VAC 形式からアクセサリを読み込む。VAC は Shift_JIS で読み込む必要がある */
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        /* 1行目: アクセサリ名 */
        const QString &name = stream.readLine();
        /* 2行目: ファイル名 */
        const QString &filename = stream.readLine();
        /* 3行目: アクセサリの拡大率 */
        float scaleFactor = stream.readLine().toFloat();
        /* 4行目: アクセサリの位置パラメータ */
        const QStringList &position = stream.readLine().split(',');
        /* 5行目: アクセサリの回転パラメータ */
        const QStringList &rotation = stream.readLine().split(',');
        /* 6行目: アクセサリに紐付ける親ボーン(未実装) */
        const QString &bone = stream.readLine();
        /* 7行目: 影をつけるかどうか(未実装) */
        bool enableShadow = stream.readLine().toInt() == 1;
        Asset *asset = 0;
        if (loadAsset(dir.absoluteFilePath(filename), uuid, asset)) {
            if (!name.isEmpty()) {
                const QByteArray &bytes = internal::fromQString(name);
                asset->setName(bytes.constData());
            }
            if (!filename.isEmpty()) {
                m_name2assets.insert(filename, asset);
            }
            if (scaleFactor > 0)
                asset->setScaleFactor(scaleFactor);
            if (position.count() == 3) {
                float x = position.at(0).toFloat();
                float y = position.at(1).toFloat();
                float z = position.at(2).toFloat();
                asset->setPosition(Vector3(x, y, z));
            }
            if (rotation.count() == 3) {
                float x = rotation.at(0).toFloat();
                float y = rotation.at(1).toFloat();
                float z = rotation.at(2).toFloat();
                asset->setRotation(Quaternion(x, y, z));
            }
            if (!bone.isEmpty() && m_model) {
                const QByteArray &bytes = internal::fromQString(name);
                Bone *bone = m_model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
                asset->setParentBone(bone);
            }
            Q_UNUSED(enableShadow);
        }
        return asset;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(baseName), qPrintable(file.errorString()));
        return 0;
    }
}

VMDMotion *SceneLoader::loadCameraMotion(const QString &path)
{
    /* カメラモーションをファイルから読み込み、場面オブジェクトに設定する */
    QFile file(path);
    VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        motion = new VMDMotion();
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())
                && motion->cameraAnimation().countKeyframes() > 0) {
            setCameraMotion(motion);
            m_project->addMotion(motion, 0, QUuid::createUuid().toString().toStdString());
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

bool SceneLoader::loadModel(const QString &filename, PMDModel *&model)
{
    /*
     * モデルをファイルから読み込む。レンダリングエンジンに送るには addModel を呼び出す必要がある
     * (確認ダイアログを出す必要があるので、読み込みとレンダリングエンジンへの追加は別処理)
     */
    UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
    const QByteArray &bytes = UILoadFile(filename, kModelLoadable, kModelExtensions, delegate);
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        bool allocated = false;
        if (!model) {
            model = new PMDModel();
            allocated = true;
        }
        if (!model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            if (allocated) {
                delete model;
                model = 0;
            }
            delegate->setArchive(0);
        }
    }
    return !isNullData && model != 0;
}

VMDMotion *SceneLoader::loadModelMotion(const QString &path)
{
    /* モーションをファイルから読み込む。モデルへの追加は setModelMotion を使う必要がある */
    QFile file(path);
    VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        motion = new VMDMotion();
        if (!motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

VMDMotion *SceneLoader::loadModelMotion(const QString &path, QList<PMDModel *> &models)
{
    /* モーションをファイルから読み込み、対象の全てのモデルに対してモーションを適用する */
    VMDMotion *motion = loadModelMotion(path);
    if (motion) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            PMDModel *model = m_project->model(modelUUIDs[i]);
            setModelMotion(motion, model);
            models.append(model);
        }
    }
    return motion;
}

VMDMotion *SceneLoader::loadModelMotion(const QString &path, PMDModel *model)
{
    /* loadModelMotion に setModelMotion の追加が入ったショートカット的なメソッド */
    VMDMotion *motion = loadModelMotion(path);
    if (motion)
        setModelMotion(motion, model);
    return motion;
}

VPDFilePtr SceneLoader::loadModelPose(const QString &path, PMDModel *model)
{
    /* ポーズをファイルから読み込む。処理の関係上 makePose は呼ばない */
    QFile file(path);
    VPDFilePtr ptr(new VPDFile());
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        if (ptr.data()->load(stream)) {
            emit modelDidMakePose(ptr, model);
        }
        else {
            ptr.clear();
        }
    }
    return ptr;
}

void SceneLoader::loadProject(const QString &path)
{
    release();
    createProject();
    bool ret = m_project->load(path.toLocal8Bit().constData());
    if (ret) {
        /* 光源設定 */
        const vpvl::Vector3 &color = UIGetVector3(m_project->globalSetting("light.color"), Vector3(0.6, 0.6, 0.6));
        const vpvl::Vector3 &position = UIGetVector3(m_project->globalSetting("light.position"), Vector3(0.5, 1.0, 0.5));
        setLightColor(Color(color.x(), color.y(), color.z(), 1.0));
        setLightPosition(position);
        /* アクセラレーションの有効化(有効にしている場合) */
        if (isAccelerationEnabled()) {
            if (m_renderer->initializeAccelerator())
                m_renderer->scene()->setSoftwareSkinningEnable(false);
        }
        int progress = 0;
        QList<PMDModel *> lostModels;
        QList<Asset *> lostAssets;
        UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        const Project::UUIDList &assetUUIDs = m_project->assetUUIDs();
        const Project::UUIDList &motionUUIDs = m_project->motionUUIDs();
        emit projectDidCount(modelUUIDs.size() + assetUUIDs.size());
        /* Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        int nmodels = modelUUIDs.size();
        Quaternion rotation;
        for (int i = 0; i < nmodels; i++) {
            const Project::UUID &modelUUIDString = modelUUIDs[i];
            PMDModel *model = m_project->model(modelUUIDString);
            const std::string &name = m_project->modelSetting(model, Project::kSettingNameKey);
            const std::string &uri = m_project->modelSetting(model, Project::kSettingURIKey);
            const QString &filename = QString::fromStdString(uri);
            if (loadModel(filename, model)) {
                m_renderer->uploadModel(model, QFileInfo(filename).dir().absolutePath().toStdString());
                delegate->setArchive(0);
                /* ModelInfoWidget でエッジ幅の値を設定するので modelDidSelect を呼ぶ前に設定する */
                const Vector3 &color = UIGetVector3(m_project->modelSetting(model, "edge.color"), kZeroV);
                model->setEdgeColor(Color(color.x(), color.y(), color.z(), 1.0));
                model->setEdgeOffset(QString::fromStdString(m_project->modelSetting(model, "edge.offset")).toFloat());
                model->setPositionOffset(UIGetVector3(m_project->modelSetting(model, "offset.position"), kZeroV));
                /* 角度で保存されるので、オイラー角を用いて Quaternion を構築する */
                const Vector3 &angle = UIGetVector3(m_project->modelSetting(model, "offset.rotation"), kZeroV);
                rotation.setEulerZYX(vpvl::radian(angle.x()), vpvl::radian(angle.y()), vpvl::radian(angle.z()));
                model->setRotationOffset(rotation);
                const QUuid modelUUID(modelUUIDString.c_str());
                m_renderOrderList.add(modelUUID);
                emit modelDidAdd(model, modelUUID);
                if (isModelSelected(model))
                    setSelectedModel(model);
                /* モデルに属するモーションを取得し、追加する */
                const Array<VMDMotion *> &motions = model->motions();
                const int nmotions = motions.count();
                for (int i = 0; i < nmotions; i++) {
                    VMDMotion *motion = motions[i];
                    const Project::UUID &motionUUIDString = m_project->motionUUID(motion);
                    const QUuid motionUUID(motionUUIDString.c_str());
                    motion->reload();
                    emit motionDidAdd(motion, model, motionUUID);
                }
                emit projectDidProceed(++progress);
                continue;
            }
            /* 読み込みに失敗したモデルは後で Project から削除するため失敗したリストに追加する */
            qWarning("Model(uuid=%s, name=%s, path=%s) cannot be loaded",
                     modelUUIDString.c_str(),
                     name.c_str(),
                     qPrintable(filename));
            lostModels.append(model);
            emit projectDidProceed(++progress);
        }
        /* Project はアクセサリのインスタンスを作成しか行わないので、ここでアクセサリとそのリソースの読み込みを行う */
        int nassets = assetUUIDs.size();
        for (int i = 0; i < nassets; i++) {
            const Project::UUID &assetUUIDString = assetUUIDs[i];
            Asset *asset = m_project->asset(assetUUIDString);
            const std::string &uri = m_project->assetSetting(asset, Project::kSettingURIKey);
            const QString &filename = QString::fromStdString(uri);
            const QByteArray &bytes = UILoadFile(filename, kAssetLoadable, kAssetExtensions, delegate);
            if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                const QFileInfo fileInfo(filename);
                const QByteArray &baseName = fileInfo.baseName().toUtf8();
                asset->setName(baseName.constData());
                m_renderer->uploadAsset(asset, fileInfo.dir().absolutePath().toStdString());
                delegate->setArchive(0);
                const QUuid assetUUID(assetUUIDString.c_str());
                m_renderOrderList.add(assetUUID);
                emit assetDidAdd(asset, assetUUID);
                if (isAssetSelected(asset))
                    setSelectedAsset(asset);
                emit projectDidProceed(++progress);
                continue;
            }
            /* 読み込みに失敗したアクセサリは後で Project から削除するため失敗したリストに追加する */
            qWarning("Asset(uuid=%s, name=%s, path=%s) cannot be loaded",
                     assetUUIDString.c_str(),
                     qPrintable(internal::toQString(asset)),
                     qPrintable(filename));
            lostAssets.append(asset);
            emit projectDidProceed(++progress);
        }
        /* カメラモーションの読み込み(親モデルがないことが前提。複数存在する場合最後に読み込まれたモーションが適用される) */
        int nmotions = motionUUIDs.size();
        for (int i = 0; i < nmotions; i++) {
            VMDMotion *motion = m_project->motion(motionUUIDs[i]);
            if (!motion->parentModel() && motion->cameraAnimation().countKeyframes() > 1)
                setCameraMotion(motion);
        }
        /* 読み込みに失敗したモデルとアクセサリを Project から削除する */
        foreach (PMDModel *model, lostModels) {
            m_project->removeModel(model);
            delete model;
        }
        foreach (Asset *asset, lostAssets) {
            m_project->removeAsset(asset);
            delete asset;
        }
        sort(true);
        m_project->setDirty(false);
        emit projectDidLoad(true);
    }
    else {
        qDebug("Failed loading project %s", qPrintable(path));
        delete m_project;
        m_project = 0;
        createProject();
        emit projectDidLoad(false);
    }
}

VMDMotion *SceneLoader::newCameraMotion() const
{
    /* 0番目に空のキーフレームが入ったカメラのモーションを作成する */
    VMDMotion *newCameraMotion = new VMDMotion();
    CameraAnimation *cameraAnimation = newCameraMotion->mutableCameraAnimation();
    CameraKeyframe *frame = new CameraKeyframe();
    Scene *scene = m_renderer->scene();
    frame->setDefaultInterpolationParameter();
    frame->setPosition(scene->cameraPosition());
    frame->setAngle(scene->cameraAngle());
    frame->setFovy(scene->fovy());
    frame->setDistance(scene->cameraDistance());
    cameraAnimation->addKeyframe(frame);
    return newCameraMotion;
}

VMDMotion *SceneLoader::newModelMotion(PMDModel *model) const
{
    /* 全ての可視ボーンと頂点モーフに対して0番目に空のキーフレームが入ったモデルのモーションを作成する */
    VMDMotion *newModelMotion = 0;
    if (model) {
        newModelMotion = new VMDMotion();
        const BoneList &bones = model->bones();
        const int nbones = bones.count();
        BoneAnimation *boneAnimation = newModelMotion->mutableBoneAnimation();
        for (int i = 0; i < nbones; i++) {
            Bone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable()) {
                BoneKeyframe *frame = new BoneKeyframe();
                frame->setDefaultInterpolationParameter();
                frame->setName(bone->name());
                boneAnimation->addKeyframe(frame);
            }
        }
        const FaceList &faces = model->faces();
        const int nfaces = faces.count();
        FaceAnimation *faceAnimation = newModelMotion->mutableFaceAnimation();
        for (int i = 0; i < nfaces; i++) {
            Face *face = faces[i];
            FaceKeyframe *frame = new FaceKeyframe();
            frame->setName(face->name());
            faceAnimation->addKeyframe(frame);
        }
    }
    return newModelMotion;
}

void SceneLoader::release()
{
    /* やっていることは削除ではなくシグナル発行すること以外 Renderer::releaseProject と同じ */
    const Project::UUIDList &assetUUIDs = m_project->assetUUIDs();
    for (Project::UUIDList::const_iterator it = assetUUIDs.begin(); it != assetUUIDs.end(); it++) {
        const Project::UUID &assetUUID = *it;
        emit assetWillDelete(m_project->asset(assetUUID), QUuid(assetUUID.c_str()));
    }
    const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
    for (Project::UUIDList::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        const Project::UUID &modelUUID = *it;
        if (PMDModel *model = m_project->model(modelUUID)) {
            const Array<VMDMotion *> &motions = model->motions();
            const int nmotions = motions.count();
            for (int i = 0; i < nmotions; i++) {
                VMDMotion *motion = motions[i];
                const Project::UUID &motionUUID = m_project->motionUUID(motion);
                emit motionWillDelete(motion, QUuid(motionUUID.c_str()));
            }
            emit modelWillDelete(model, QUuid(modelUUID.c_str()));
        }
    }
    /*
      releaseProject は Project 内にある全ての Asset と PMDModel のインスタンスを Renderer クラスから物理削除し、
      Project クラスから論理削除 (remove*) を行う。Project が物理削除 (delete*) を行なってしまうと Renderer クラスで
      物理削除した時二重削除となってしまい不正なアクセスが発生するため、Project 側は論理削除だけにとどめておく必要がある。
     */
    m_renderer->releaseProject(m_project);
    m_renderOrderList.clear();
    deleteCameraMotion();
    delete m_project;
    m_project = 0;
    m_asset = 0;
    m_model = 0;
}

void SceneLoader::render()
{
    UIEnableMultisample();
    glEnable(GL_DEPTH_TEST);
    m_renderer->clear();
    /* モデル行列とビュー行列の乗算を QMatrix4x4 を用いて行う */
    QMatrix4x4 modelView4x4, projection4x4;
    vpvl::Scene *scene = m_renderer->scene();
    float modelViewMatrixf[16], projectionMatrixf[16], normalMatrixf[9];
    scene->getModelViewMatrix(modelViewMatrixf);
    scene->getProjectionMatrix(projectionMatrixf);
    m_renderer->setModelViewMatrix(modelViewMatrixf);
    m_renderer->setProjectionMatrix(projectionMatrixf);
    for (int i = 0; i < 16; i++) {
        modelView4x4.data()[i] = modelViewMatrixf[i];
        projection4x4.data()[i] = projectionMatrixf[i];
    }
    scene->getNormalMatrix(normalMatrixf);
    const QMatrix4x4 &modelViewProjection4x4 = projection4x4 * modelView4x4;
    for (int i = 0; i < 16; i++)
        modelViewMatrixf[i] = modelViewProjection4x4.constData()[i];
    m_renderer->setModelViewProjectionMatrix(modelViewMatrixf);
    m_renderer->setNormalMatrix(normalMatrixf);
    /* 順番にそってレンダリング開始 */
    const int nobjects = m_renderOrderList.count();
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (PMDModel *model = m_project->model(uuidString)) {
            if (isProjectiveShadowEnabled(model)) {
                glCullFace(GL_FRONT);
                m_renderer->renderModelShadow(model);
                glCullFace(GL_BACK);
            }
            m_renderer->renderModelEdge(model);
            m_renderer->renderModel(model);
        }
        else if (Asset *asset = m_project->asset(uuidString))
            m_renderer->renderAsset(asset);
    }
}

const QList<QUuid> SceneLoader::renderOrderList() const
{
    QList<QUuid> r;
    int n = m_renderOrderList.count();
    for (int i = 0; i < n; i++)
        r.append(m_renderOrderList[i]);
    return r;
}

void SceneLoader::saveMetadataFromAsset(const QString &path, Asset *asset)
{
    /* 現在のアセットの位置情報からファイルに書き出す。行毎の意味は loadMetadataFromAsset を参照 */
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        const char lineSeparator[] = "\r\n";
        stream << internal::toQString(asset) << lineSeparator;
        stream << m_name2assets.key(asset) << lineSeparator;
        stream << asset->scaleFactor() << lineSeparator;
        const Vector3 &position = asset->position();
        stream << QString("%1,%2,%3").arg(position.x(), 0, 'f', 1)
                  .arg(position.y(), 0, 'f', 1).arg(position.z(), 0, 'f', 1) << lineSeparator;
        const Quaternion &rotation = asset->rotation();
        stream << QString("%1,%2,%3").arg(rotation.x(), 0, 'f', 1)
                  .arg(rotation.y(), 0, 'f', 1).arg(rotation.z(), 0, 'f', 1) << lineSeparator;
        const Bone *bone = asset->parentBone();
        stream << (bone ? internal::toQString(bone) : "地面") << lineSeparator;
        stream << 1 << lineSeparator;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(path), qPrintable(file.errorString()));
    }
}

void SceneLoader::saveProject(const QString &path)
{
    commitAssetProperties();
    m_project->save(path.toLocal8Bit().constData());
    emit projectDidSave();
}

void SceneLoader::setCameraMotion(VMDMotion *motion)
{
    const QUuid &uuid = QUuid::createUuid();
    deleteCameraMotion();
    m_camera = motion;
    m_project->addMotion(motion, 0, uuid.toString().toStdString());
    m_renderer->scene()->setCameraMotion(motion);
    emit cameraMotionDidSet(motion, uuid);
}

void SceneLoader::setLightColor(const Color &color)
{
    Scene *scene = renderEngine()->scene();
    scene->setLightSource(color, scene->lightPosition());
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", color.x(), color.y(), color.z());
        m_project->setGlobalSetting("light.color", str.toStdString());
    }
    emit lightColorDidSet(color);
}

void SceneLoader::setLightPosition(const Vector3 &position)
{
    Scene *scene = renderEngine()->scene();
    scene->setLightSource(scene->lightColor(), position);
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", position.x(), position.y(), position.z());
        m_project->setGlobalSetting("light.position", str.toStdString());
    }
    emit lightPositionDidSet(position);
}

void SceneLoader::setModelMotion(VMDMotion *motion, PMDModel *model)
{
    const QUuid &uuid = QUuid::createUuid();
#ifndef QMA_ENABLE_MULTIPLE_MOTION
    const Array<VMDMotion *> &motions = model->motions();
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        /* 先にプロジェクトからモーションを論理削除した後にモデルから物理削除する */
        VMDMotion *motion = motions[i];
        m_project->removeMotion(motion, model);
        model->deleteMotion(motion);
    }
#endif
    model->addMotion(motion);
    m_project->addMotion(motion, model, uuid.toString().toStdString());
    emit motionDidAdd(motion, model, uuid);
}

void SceneLoader::setRenderOrderList(const QList<QUuid> &value)
{
    int i = 1;
    foreach (const QUuid &uuid, value) {
        const Project::UUID &u = uuid.toString().toStdString();
        const std::string &n = QVariant(i).toString().toStdString();
        if (PMDModel *model = m_project->model(u)) {
            m_project->setModelSetting(model, "order", n);
        }
        else if (Asset *asset = m_project->asset(u)) {
            m_project->setAssetSetting(asset, "order", n);
        }
        i++;
    }
    sort(true);
}

void SceneLoader::sort(bool useOrderAttr)
{
    if (m_project)
        m_renderOrderList.sort(UIRenderOrderPredication(m_project, m_renderer->scene()->modelViewTransform(), useOrderAttr));
}

void SceneLoader::startPhysicsSimulation()
{
    /* 物理暴走を防ぐために少し進めてから開始する */
    if (isPhysicsEnabled()) {
        btDiscreteDynamicsWorld *world = m_world->mutableWorld();
        m_renderer->scene()->setWorld(world);
        world->stepSimulation(1, 60);
    }
}

void SceneLoader::stopPhysicsSimulation()
{
    Scene *scene = m_renderer->scene();
    scene->setWorld(0);
    const Array<PMDModel *> &models = scene->getRenderingOrder();
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = models[i];
        model->resetAllBones();
    }
}

const Vector3 SceneLoader::worldGravity() const
{
    static const Vector3 defaultGravity(0.0, -9.8, 0.0);
    return m_project ? UIGetVector3(m_project->globalSetting("physics.gravity"), defaultGravity) : defaultGravity;
}

const QColor SceneLoader::screenColor() const
{
    QColor color(255, 255, 255);
    if (m_project) {
        const Vector3 &value = UIGetVector3(m_project->globalSetting("screen.color"), Vector3(1.0, 1.0, 1.0));
        color.setRedF(value.x());
        color.setGreenF(value.y());
        color.setBlueF(value.z());
    }
    return color;
}

void SceneLoader::setWorldGravity(const Vector3 &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_world->setGravity(value);
        m_project->setGlobalSetting("physics.gravity", str.toStdString());
    }
}

bool SceneLoader::isProjectiveShadowEnabled(const PMDModel *model) const
{
    return m_project ? m_project->modelSetting(model, "shadow.projective") == "true" : false;
}

void SceneLoader::setProjectiveShadowEnable(const PMDModel *model, bool value)
{
    if (m_project)
        m_project->setModelSetting(model, "shadow.projective", value ? "true" : "false");
}

PMDModel *SceneLoader::selectedModel() const
{
    return m_model;
}

bool SceneLoader::isModelSelected(const PMDModel *value) const
{
    return m_project ? m_project->modelSetting(value, "selected") == "true" : false;
}

void SceneLoader::setSelectedModel(PMDModel *value)
{
    if (m_project) {
        m_model = value;
        m_project->setModelSetting(value, "selected", "true");
        emit modelDidSelect(value, this);
    }
}

void SceneLoader::setModelEdgeOffset(PMDModel *model, float value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f", value);
        model->setEdgeOffset(value);
        m_project->setModelSetting(model, "edge.offset", str.toStdString());
    }
}

void SceneLoader::setModelPosition(PMDModel *model, const Vector3 &value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        model->setPositionOffset(value);
        m_project->setModelSetting(model, "offset.position", str.toStdString());
    }
}

const vpvl::Vector3 SceneLoader::modelRotation(PMDModel *value) const
{
    return m_project ? UIGetVector3(m_project->modelSetting(value, "offset.rotation"), kZeroV) : kZeroV;
}

void SceneLoader::setModelRotation(PMDModel *model, const Vector3 &value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_project->setModelSetting(model, "offset.rotation", str.toStdString());
        Quaternion rotation;
        rotation.setEulerZYX(vpvl::radian(value.x()), vpvl::radian(value.y()), vpvl::radian(value.z()));
        model->setRotationOffset(rotation);
    }
}

void SceneLoader::setModelEdgeColor(PMDModel *model, const QColor &value)
{
    if (m_project) {
        QString str;
        float red = value.redF(), green = value.greenF(), blue = value.blueF();
        str.sprintf("%.5f,%.5f,%.5f", red, green, blue);
        model->setEdgeColor(Color(red, green, blue, 1.0));
        m_project->setModelSetting(model, "edge.color", str.toStdString());
    }
}

bool SceneLoader::isGridVisible() const
{
    return globalSetting("grid.visible", true);
}

void SceneLoader::setGridVisible(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isGridVisible() != value)
        m_project->setGlobalSetting("grid.visible", value ? "true" : "false");
}

bool SceneLoader::isPhysicsEnabled() const
{
    return globalSetting("physics.enabled", false);
}

void SceneLoader::setPhysicsEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isPhysicsEnabled() != value)
        m_project->setGlobalSetting("physics.enabled", value ? "true" : "false");
}

bool SceneLoader::isAccelerationEnabled() const
{
    return globalSetting("acceleration.enabled", false);
}

void SceneLoader::setAccelerationEnabled(bool value)
{
    /* アクセレーションをサポートする場合のみ有効にする。しない場合は常に無効に設定 */
    if (isAccelerationSupported()) {
        if (value) {
            if (m_renderer->initializeAccelerator()) {
                m_renderer->scene()->setSoftwareSkinningEnable(false);
                if (m_project && !isAccelerationEnabled())
                    m_project->setGlobalSetting("acceleration.enabled", "true");
                return;
            }
            else {
                qWarning("%s", qPrintable(tr("Failed enabling acceleration and set fallback.")));
            }
        }
    }
    else {
        qWarning("%s", qPrintable(tr("Acceleration is not supported on this platform and set fallback.")));
    }
    m_renderer->scene()->setSoftwareSkinningEnable(true);
    if (m_project && isAccelerationEnabled())
        m_project->setGlobalSetting("acceleration.enabled", "false");
}

/* 再生設定及びエンコード設定の場合は同値チェックを行わない。こちらは値を確実に保存させる必要があるため */
int SceneLoader::frameIndexPlayFrom() const
{
    return globalSetting("play.frame_index.from", 0);
}

void SceneLoader::setFrameIndexPlayFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexPlayTo() const
{
    return globalSetting("play.frame_index.to", int(m_renderer->scene()->maxFrameIndex()));
}

void SceneLoader::setFrameIndexPlayTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForPlay() const
{
    return globalSetting("play.fps", 60);
}

void SceneLoader::setSceneFPSForPlay(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoFrom() const
{
    return globalSetting("video.frame_index.from", 0);
}

void SceneLoader::setFrameIndexEncodeVideoFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoTo() const
{
    return globalSetting("video.frame_index.to", int(m_renderer->scene()->maxFrameIndex()));
}

void SceneLoader::setFrameIndexEncodeVideoTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForEncodeVideo() const
{
    return globalSetting("video.fps", 60);
}

void SceneLoader::setSceneFPSForEncodeVideo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneWidth() const
{
    return globalSetting("video.width", 0);
}

void SceneLoader::setSceneWidth(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.width", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneHeight() const
{
    return globalSetting("video.height", 0);
}

void SceneLoader::setSceneHeight(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.height", QVariant(value).toString().toStdString());
}

bool SceneLoader::isLoop() const
{
    return globalSetting("play.loop", false);
}

void SceneLoader::setLoop(bool value)
{
    if (m_project)
        m_project->setGlobalSetting("play.loop", value ? "true" : "false");
}

bool SceneLoader::isGridIncluded() const
{
    return globalSetting("grid.video", false);
}

void SceneLoader::setGridIncluded(bool value)
{
    if (m_project)
        m_project->setGlobalSetting("grid.video", value ? "true" : "false");
}

const QString SceneLoader::backgroundAudio() const
{
    return m_project ? QString::fromStdString(m_project->globalSetting("audio.path")) : "";
}

void SceneLoader::setBackgroundAudioPath(const QString &value)
{
    if (m_project)
        m_project->setGlobalSetting("audio.path", value.toStdString());
}

const Vector3 SceneLoader::assetPosition(const Asset *asset)
{
    return m_project ? UIGetVector3(m_project->assetSetting(asset, "position"), kZeroV) : kZeroV;
}

void SceneLoader::setAssetPosition(const Asset *asset, const Vector3 &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_project->setAssetSetting(asset, "position", str.toStdString());
    }
}

const Quaternion SceneLoader::assetRotation(const Asset *asset)
{
    return m_project ? UIGetQuaternion(m_project->assetSetting(asset, "rotation"), kZeroQ) : kZeroQ;
}

void SceneLoader::setAssetRotation(const Asset *asset, const Quaternion &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
        m_project->setAssetSetting(asset, "rotation", str.toStdString());
    }
}

float SceneLoader::assetOpacity(const Asset *asset)
{
    if (m_project) {
        float value = QString::fromStdString(m_project->assetSetting(asset, "opacity")).toFloat();
        return qBound(0.0f, value, 1.0f);
    }
    return 1.0f;
}

void SceneLoader::setAssetOpacity(const Asset *asset, float value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f", value);
        m_project->setAssetSetting(asset, "opacity", str.toStdString());
    }
}

float SceneLoader::assetScaleFactor(const Asset *asset)
{
    if (m_project) {
        float value = QString::fromStdString(m_project->assetSetting(asset, "scale")).toFloat();
        return qBound(0.0001f, value, 10000.0f);
    }
    return 10.0f;
}

void SceneLoader::setAssetScaleFactor(const Asset *asset, float value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f", value);
        m_project->setAssetSetting(asset, "scale", str.toStdString());
    }
}

PMDModel *SceneLoader::assetParentModel(Asset *asset) const
{
    return m_project ? m_project->model(m_project->assetSetting(asset, "parent.model")) : 0;
}

void SceneLoader::setAssetParentModel(const Asset *asset, PMDModel *model)
{
    if (m_project)
        m_project->setAssetSetting(asset, "parent.model", m_project->modelUUID(model));
}

Bone *SceneLoader::assetParentBone(Asset *asset) const
{
    PMDModel *model = 0;
    if (m_project && (model = assetParentModel(asset))) {
        const QString &name = QString::fromStdString(m_project->assetSetting(asset, "parent.bone"));
        const QByteArray &bytes = internal::fromQString(name);
        return model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
    }
    return 0;
}

void SceneLoader::setAssetParentBone(const Asset *asset, Bone *bone)
{
    if (m_project)
        m_project->setAssetSetting(asset, "parent.bone", internal::toQString(bone).toStdString());
}

Asset *SceneLoader::selectedAsset() const
{
    return m_asset;
}

bool SceneLoader::isAssetSelected(const Asset *value) const
{
    return m_project ? m_project->assetSetting(value, "selected") == "true" : false;
}

void SceneLoader::setSelectedAsset(Asset *value)
{
    if (m_project) {
        commitAssetProperties();
        m_asset = value;
        m_project->setAssetSetting(value, "selected", "true");
        emit assetDidSelect(value, this);
    }
}

void SceneLoader::setPreferredFPS(int value)
{
    m_world->setPreferredFPS(value);
    m_renderer->scene()->setPreferredFPS(value);
}

void SceneLoader::setScreenColor(const QColor &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.redF(), value.greenF(), value.blueF());
        m_project->setGlobalSetting("screen.color", str.toStdString());
    }
}

bool SceneLoader::globalSetting(const char *key, bool def) const
{
    return m_project ? m_project->globalSetting(key) == "true" : def;
}

int SceneLoader::globalSetting(const char *key, int def) const
{
    if (m_project) {
        bool ok = false;
        int value = QString::fromStdString(m_project->globalSetting(key)).toInt(&ok);
        return ok ? value : def;
    }
    return def;
}

internal::World *SceneLoader::world() const
{
    return m_world;
}


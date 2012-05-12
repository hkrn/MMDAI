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
#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>

using namespace vpvl2;

namespace
{

typedef QScopedArrayPointer<uint8_t> ByteArrayPtr;

static const QRegExp &kAssetLoadable = QRegExp(".(bmp|jpe?g|png|sp[ah]|tga|x)$");
static const QRegExp &kAssetExtensions = QRegExp(".x$");
static const QRegExp &kModelLoadable = QRegExp(".(bmp|jpe?g|pm[dx]|png|sp[ah]|tga)$");
static const QRegExp &kModelExtensions = QRegExp(".pm[dx]$");

class UIDelegate : public Project::IDelegate, public IRenderDelegate
{
public:
    struct PrivateContext {
        QHash<QString, GLuint> textureCache;
    };
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

    void allocateContext(const IModel *model, void *&context) {
        PrivateContext *c = new(std::nothrow) PrivateContext();
        context = c;
        if (const IString *name = model->name())
            qDebug("Allocated a context object: %s", name->toByteArray() );
    }
    void releaseContext(const IModel *model, void *&context) {
        delete static_cast<PrivateContext *>(context);
        context = 0;
        if (const IString *name = model->name())
            qDebug("Released a context object: %s", name->toByteArray());
    }

    bool uploadTexture(void *context, const char *name, const IString *dir, void *texture, bool isToon) {
        const QDir d(static_cast<const internal::String *>(dir)->value());
        const QString &path = QDir::cleanPath(d.absoluteFilePath(QString::fromStdString(name)));
        return uploadTexture0(context, path, texture, isToon);
    }
    bool uploadTexture(void *context, const IString *name, const IString *dir, void *texture, bool isToon) {
        const QDir d(static_cast<const internal::String *>(dir)->value());
        const QString &s = static_cast<const internal::String *>(name)->value();
        const QString &path = QDir::cleanPath(d.absoluteFilePath(s));
        return uploadTexture0(context, path, texture, isToon);
    }
    bool uploadToonTexture(void *context, const char *name, const IString *dir, void *texture) {
        const QDir d(static_cast<const internal::String *>(dir)->value());
        return uploadToonTexture0(context, QString::fromStdString(name), d, texture);
    }
    bool uploadToonTexture(void *context, const IString *name, const IString *dir, void *texture) {
        const QDir d(static_cast<const internal::String *>(dir)->value());
        const QString &s = static_cast<const internal::String *>(name)->value();
        return uploadToonTexture0(context, s, d, texture);
    }
    bool uploadToonTexture(void *context, int index, void *texture) {
        QString format;
        const QString &pathString = format.sprintf("toon%02d.bmp", index + 1);
        return uploadToonTexture0(context, pathString, QDir(), texture);
    }
    IString *loadShaderSource(IRenderDelegate::ShaderType type, const IModel *model, void *context) {
        Q_UNUSED(context)
        QString filename;
        switch (model->type()) {
        case IModel::kAsset:
            filename += "asset.";
            break;
        case IModel::kPMD:
            filename += "pmd.";
            break;
        case IModel::kPMX:
            filename += "pmx.";
            break;
        }
        switch (type) {
        case IRenderDelegate::kEdgeVertexShader:
            filename += "edge.vsh";
            break;
        case IRenderDelegate::kEdgeFragmentShader:
            filename += "edge.fsh";
            break;
        case IRenderDelegate::kModelVertexShader:
            filename += "model.vsh";
            break;
        case IRenderDelegate::kModelFragmentShader:
            filename += "model.fsh";
            break;
        case IRenderDelegate::kShadowVertexShader:
            filename += "shadow.vsh";
            break;
        case IRenderDelegate::kShadowFragmentShader:
            filename += "shadow.fsh";
            break;
        case IRenderDelegate::kZPlotVertexShader:
            filename += "zplot.vsh";
            break;
        case IRenderDelegate::kZPlotFragmentShader:
            filename += "zplot.fsh";
            break;
        }
        const QString &path = QString(":/shaders/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            file.close();
            qDebug("Loaded a shader: %s", qPrintable(path));
            return new(std::nothrow) internal::String(bytes);
        }
        else {
            qWarning("Failed loading a shader: %s", qPrintable(path));
            return 0;
        }
    }
    IString *loadKernelSource(IRenderDelegate::KernelType type, void * /* context */) {
        QString filename;
        switch (type) {
        case IRenderDelegate::kModelSkinningKernel:
            filename = "skinning.cl";
            break;
        }
        const QString &path = QString(":/kernels/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            file.close();
            qDebug("Loaded a kernel: %s", qPrintable(path));
            return new(std::nothrow) internal::String(bytes);
        }
        else {
            qWarning("Failed loading a kernel: %s", qPrintable(path));
            return 0;
        }
    }
    void log(void * /* context */, IRenderDelegate::LogLevel level, const char *format, va_list ap) {
        QString message;
        message.vsprintf(format, ap);
        switch (level) {
        case IRenderDelegate::kLogInfo:
        default:
            qDebug("%s", qPrintable(message));
            break;
        case IRenderDelegate::kLogWarning:
            qWarning("%s", qPrintable(message));
            break;
        }
    }
    const std::string toStdFromString(const IString *value) const {
        return static_cast<const internal::String *>(value)->value().toStdString();
    }
    const IString *toStringFromStd(const std::string &value) const {
        return new(std::nothrow) internal::String(QString::fromStdString(value));
    }
    IString *toUnicode(const uint8_t *value) const {
        return new(std::nothrow) internal::String(internal::toQString(value));
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
    static void setTextureID(GLuint textureID, bool isToon, void *texture) {
        *static_cast<GLuint *>(texture) = textureID;
        if (!isToon) {
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    bool uploadTexture0(void *context, const QString &path, void *texture, bool isToon) {
        QImage image;
        QString mutablePath = path;
        mutablePath.replace(QChar(0xa5), QChar('/')).replace("\\", "/");
        /* 読み込み済みのキャッシュを検索し、あったらそれを利用する */
        PrivateContext *ctx = static_cast<PrivateContext *>(context);
        if (ctx->textureCache.contains(path)) {
            setTextureID(ctx->textureCache[path], isToon, texture);
            return true;
        }
        QScopedArrayPointer<uint8_t> ptr(new uint8_t[1]);
        QFileInfo info(mutablePath);
        if (info.isDir()) {
            /* ディレクトリの場合は警告は出さず強制的に読み込み失敗に設定する */
            return false;
        }
        /* ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く) */
        if (m_archive && !mutablePath.startsWith(":/")) {
            QByteArray suffix = info.suffix().toLower().toUtf8();
            if (suffix == "sph" || suffix == "spa")
                suffix.setRawData("bmp", 3);
            const QByteArray &bytes = m_archive->data(mutablePath);
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
            if (!info.exists()) {
                qWarning("Loading texture %s doesn't exists", qPrintable(info.absoluteFilePath()));
                return false;
            }
            if (image.isNull() && info.suffix().toLower() == "tga") {
                image = loadTGA(mutablePath, ptr);
            }
            else {
                image = QImage(mutablePath).rgbSwapped();
            }
            if (image.isNull()) {
                qWarning("Loading texture %s cannot decode", qPrintable(info.absoluteFilePath()));
                return false;
            }
        }
        /* スフィアテクスチャの場合一旦反転する */
        if (mutablePath.endsWith(".sph") || mutablePath.endsWith(".spa")) {
            QTransform transform;
            transform.scale(1, -1);
            image = image.transformed(transform);
        }
        QGLContext *glcontext = const_cast<QGLContext *>(QGLContext::currentContext());
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        GLuint textureID = glcontext->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D, GL_RGBA, options);
        setTextureID(textureID, isToon, texture);
        ctx->textureCache.insert(path, textureID);
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID,
               m_archive ? qPrintable(info.fileName()) : qPrintable(mutablePath));
        return textureID != 0;
    }
    bool uploadToonTexture0(void *context, const QString &name, const QDir &dir, void *texture) {
        QString mutableName = name;
        mutableName.replace("\\", "/");
        /* アーカイブ内にある場合はシステム側のテクスチャ処理とごっちゃにならないように uploadTexture に流して返す */
        const QString &path = QDir::cleanPath(dir.absoluteFilePath(mutableName));
        if (m_archive && !m_archive->data(path).isEmpty())
            return uploadTexture0(context, path, texture, true);
        /* ファイルが存在しない場合はシステム側のテクスチャと仮定 */
        if (!QFile::exists(path))
            return uploadTexture0(context, QString(":/textures/%1").arg(name), texture, true);
        else
            return uploadTexture0(context, path, texture, true);
    }
    static QImage loadTGA(const QString &path, QScopedArrayPointer<uint8_t> &dataPtr) {
        QFile file(path);
        if (file.open(QFile::ReadOnly) && file.size() > 18) {
            return loadTGA(file.readAll(), dataPtr);
        }
        else {
            qWarning("Cannot open file %s: %s", qPrintable(path), qPrintable(file.errorString()));
            return QImage();
        }
    }
    static QImage loadTGA(QByteArray data, QScopedArrayPointer<uint8_t> &dataPtr) {
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
            const Scalar &x = gravity.at(0).toFloat();
            const Scalar &y = gravity.at(1).toFloat();
            const Scalar &z = gravity.at(2).toFloat();
            return Vector3(x, y, z);
        }
    }
    return def;
}
/* 文字列を解析して Vector4 を構築する */
static const Vector4 UIGetVector4(const std::string &value, const Vector4 &def)
{
    if (!value.empty()) {
        QStringList gravity = QString::fromStdString(value).split(",");
        if (gravity.length() == 4) {
            const Scalar &x = gravity.at(0).toFloat();
            const Scalar &y = gravity.at(1).toFloat();
            const Scalar &z = gravity.at(2).toFloat();
            const Scalar &w = gravity.at(3).toFloat();
            return Vector4(x, y, z, w);
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
            const Scalar &x = gravity.at(0).toFloat();
            const Scalar &y = gravity.at(1).toFloat();
            const Scalar &z = gravity.at(2).toFloat();
            const Scalar &w = gravity.at(3).toFloat();
            return Quaternion(x, y, z, w);
        }
    }
    return def;
}

static inline bool UIIsPowerOfTwo(int value) {
    return (value & (value - 1)) == 0;
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
        IModel *lmodel = m_project->model(luuid), *rmodel = m_project->model(ruuid);
        bool lok, rok;
        if (lmodel && rmodel) {
            int lorder = QString::fromStdString(m_project->modelSetting(lmodel, "order")).toInt(&lok);
            int rorder = QString::fromStdString(m_project->modelSetting(rmodel, "order")).toInt(&rok);
            if (lok && rok && m_useOrderAttr) {
                return lorder < rorder;
            }
            else {
                const Vector3 &positionLeft = m_modelViewTransform * lmodel->position();
                const Vector3 &positionRight = m_modelViewTransform * rmodel->position();
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
void UISetModelType(const QString &filename, IModel::Type &type)
{
    if (filename.endsWith(".pmx"))
        type = IModel::kPMX;
    else if (filename.endsWith(".pmd"))
        type = IModel::kPMD;
    else
        type = IModel::kAsset;
}

const QByteArray UILoadFile(const QString &filename,
                            const QRegExp &loadable,
                            const QRegExp &extensions,
                            IModel::Type &type,
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
                    /* ここではパスを置換して uploadTexture で読み込めるようにする */
                    const QString &filenameToLoad = target.first();
                    bytes = archive->data(filenameToLoad);
                    QFileInfo fileToLoadInfo(filenameToLoad), fileInfo(filename);
                    archive->replaceFilePath(fileToLoadInfo.path(), fileInfo.path() + "/");
                    delegate->setArchive(archive);
                    UISetModelType(filenameToLoad, type);
                }
            }
        }
    }
    else {
        QFile file(filename);
        if (file.open(QFile::ReadOnly))
            bytes = file.readAll();
        UISetModelType(filename, type);
    }
    return bytes;
}

}

SceneLoader::SceneLoader(IEncoding *encoding, Factory *factory)
    : QObject(),
      m_world(0),
      m_depthBuffer(0),
      m_encoding(encoding),
      m_renderDelegate(0),
      m_factory(factory),
      m_project(0),
      m_projectDelegate(0),
      m_model(0),
      m_asset(0),
      m_camera(0),
      m_depthBufferID(0)
{
    m_world = new internal::World();
    m_renderDelegate = new UIDelegate();
    m_projectDelegate = new UIDelegate();
    createProject();
}

SceneLoader::~SceneLoader()
{
    release();
    m_depthBufferID = 0;
    delete m_factory;
    m_factory = 0;
    delete m_renderDelegate;
    m_renderDelegate = 0;
    delete m_projectDelegate;
    m_projectDelegate = 0;
    delete m_depthBuffer;
    m_depthBuffer = 0;
    delete m_world;
    m_world = 0;
    delete m_encoding;
    m_encoding = 0;
}

void SceneLoader::addModel(IModel *model, const QString &baseName, const QDir &dir, QUuid &uuid)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    const QString &key = internal::toQString(model).trimmed();
    if (key.isEmpty()) {
        internal::String s(key);
        model->setName(&s);
    }
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    IRenderEngine *engine = m_project->createRenderEngine(m_renderDelegate, model);
    internal::String d(dir.absolutePath());
    engine->upload(&d);
    /* モデルを SceneLoader にヒモ付けする */
    const QString &path = dir.absoluteFilePath(baseName);
    uuid = QUuid::createUuid();
    m_project->addModel(model, engine, uuid.toString().toStdString());
    m_project->setModelSetting(model, Project::kSettingNameKey, key.toStdString());
    m_project->setModelSetting(model, Project::kSettingURIKey, path.toStdString());
    m_project->setModelSetting(model, "selected", "false");
#ifndef IS_QMA2
    if (isPhysicsEnabled())
        model->joinWorld(m_world->mutableWorld());
#endif
    m_renderOrderList.add(uuid);
    static_cast<UIDelegate *>(m_renderDelegate)->setArchive(0);
    emit modelDidAdd(model, uuid);
}

QList<IModel *> SceneLoader::allModels() const
{
    const Project::UUIDList &uuids = m_project->modelUUIDs();
    QList<IModel *> models;
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
        m_project = new Project(m_projectDelegate, m_factory);
        /*
         * デフォルトではグリッド表示と物理演算を有効にするため、設定後強制的に dirty フラグを無効にする
         * これによってアプリケーションを起動して何もしないまま終了する際の保存ダイアログを抑制する
         */
        m_project->setGlobalSetting("grid.visible", "true");
        m_project->setGlobalSetting("physics.enabled", "true");
        m_project->setDirty(false);
    }
}

void SceneLoader::deleteAsset(IModel *asset)
{
    /* アクセサリをレンダリングエンジンから削除し、SceneLoader のヒモ付けも解除する */
    if (asset && m_project->containsModel(asset)) {
        const QUuid uuid(m_project->modelUUID(asset).c_str());
        emit assetWillDelete(asset, uuid);
        m_project->removeModel(asset);
        m_renderOrderList.remove(uuid);
        delete asset;
        m_asset = 0;
    }
}

void SceneLoader::deleteCameraMotion()
{
    /* カメラモーションをシーンから解除及び削除し、最初の視点に戻しておく */
    Scene::ICamera *camera = m_project->camera();
    camera->setMotion(0);
    camera->resetDefault();
    m_project->removeMotion(m_camera);
    delete m_camera;
    m_camera = 0;
}

void SceneLoader::deleteModel(IModel *&model)
{
    if (m_project->containsModel(model)) {
        const QUuid uuid(m_project->modelUUID(model).c_str());
        emit modelWillDelete(model, uuid);
        /*
         * motionWillDelete を発行するため、削除対象のモデルに所属するモーションを削除する。
         * なお、データの物理削除を伴うため、配列に削除前のモーション配列をコピーしてから処理する必要がある。
         * そうしないと削除したモーションデータを参照してクラッシュしてしまう。
         */
        Array<IMotion *> motions;
        motions.copy(m_project->motions());
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i];
            if (motion->parentModel() == model) {
                deleteMotion(motion);
            }
        }
        m_project->removeModel(model);
        m_project->deleteModel(model);
        m_renderOrderList.remove(uuid);
        m_model = 0;
    }
}

void SceneLoader::deleteMotion(IMotion *&motion)
{
    const QUuid uuid(m_project->motionUUID(motion).c_str());
    emit motionWillDelete(motion, uuid);
    m_project->removeMotion(motion);
    delete motion;
    motion = 0;
}

IModel *SceneLoader::findAsset(const QUuid &uuid) const
{
    return m_project->model(uuid.toString().toStdString());
}

IModel *SceneLoader::findModel(const QUuid &uuid) const
{
    return m_project->model(uuid.toString().toStdString());
}

IMotion *SceneLoader::findMotion(const QUuid &uuid) const
{
    return m_project->motion(uuid.toString().toStdString());
}

const QUuid SceneLoader::findUUID(IModel *model) const
{
    return QUuid(m_project->modelUUID(model).c_str());
}

void SceneLoader::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    const Array<IModel *> &models = m_project->models();
    const int nmodels = models.count();
    Array<Scalar> radiusArray;
    Array<Vector3> centerArray;
    center.setZero();
    radius = 0;
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        if (model->isVisible()) {
            Vector3 c;
            Scalar r;
            model->getBoundingSphere(c, r);
            radiusArray.add(r);
            centerArray.add(c);
            center += c;
        }
    }
    if (nmodels > 0)
        center /= nmodels;
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        if (model->isVisible()) {
            const Vector3 &c = centerArray[i];
            const Scalar &r = radiusArray[i];
            const Scalar &d = center.distance(c) + r;
            btSetMax(radius, d);
        }
    }
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

bool SceneLoader::loadAsset(const QString &filename, QUuid &uuid, IModel *&asset)
{
    UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
    IModel::Type type; /* unused */
    const QByteArray &bytes = UILoadFile(filename, kAssetLoadable, kAssetExtensions, type, delegate);
    /*
     * アクセサリをファイルから読み込み、レンダリングエンジンに渡してレンダリング可能な状態にする
     */
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        bool allocated = false;
        if (!asset) {
            asset = m_factory->createModel(IModel::kAsset);
            allocated = true;
        }
        if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
            QFileInfo fileInfo(filename);
            internal::String name(fileInfo.baseName());
            asset->setName(&name);
            IRenderEngine *engine = m_project->createRenderEngine(m_renderDelegate, asset);
            internal::String s(fileInfo.absoluteDir().path());
            engine->upload(&s);
            delegate->setArchive(0);
            uuid = QUuid::createUuid();
            m_project->addModel(asset, engine, uuid.toString().toStdString());
            m_project->setModelSetting(asset, Project::kSettingNameKey, fileInfo.baseName().toStdString());
            m_project->setModelSetting(asset, Project::kSettingURIKey, filename.toStdString());
            m_project->setModelSetting(asset, "selected", "false");
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

IModel *SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid)
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
        QScopedPointer<IModel> asset(m_factory->createModel(IModel::kAsset));
        IModel *assetPtr = asset.data();
        if (loadAsset(dir.absoluteFilePath(filename), uuid, assetPtr)) {
            if (!name.isEmpty()) {
                internal::String s(name);
                asset->setName(&s);
            }
            if (!filename.isEmpty()) {
                m_name2assets.insert(filename, assetPtr);
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
                internal::String s(name);
                IBone *bone = m_model->findBone(&s);
                asset->setParentBone(bone);
            }
            Q_UNUSED(enableShadow);
        }
        return asset.take();
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(baseName), qPrintable(file.errorString()));
        return 0;
    }
}

IMotion *SceneLoader::loadCameraMotion(const QString &path)
{
    /* カメラモーションをファイルから読み込み、場面オブジェクトに設定する */
    QFile file(path);
    QScopedPointer<IMotion> motion;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        motion.reset(m_factory->createMotion());
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())
                && motion->countKeyframes(IKeyframe::kCamera) > 0) {
            setCameraMotion(motion.data());
            m_project->addMotion(motion.data(), QUuid::createUuid().toString().toStdString());
        }
        else {
            motion.reset(0);
        }
    }
    return motion.take();
}

bool SceneLoader::loadModel(const QString &filename, IModel *&model)
{
    /*
     * モデルをファイルから読み込む。レンダリングエンジンに送るには addModel を呼び出す必要がある
     * (確認ダイアログを出す必要があるので、読み込みとレンダリングエンジンへの追加は別処理)
     */
    UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
    IModel::Type type;
    const QByteArray &bytes = UILoadFile(filename, kModelLoadable, kModelExtensions, type, delegate);
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        bool allocated = false;
        if (!model) {
            model = m_factory->createModel(type);
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

IMotion *SceneLoader::loadModelMotion(const QString &path)
{
    /* モーションをファイルから読み込む。モデルへの追加は setModelMotion を使う必要がある */
    QFile file(path);
    QScopedPointer<IMotion> motion;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        motion.reset(m_factory->createMotion());
        if (!motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size()))
            motion.reset(0);
    }
    return motion.take();
}

IMotion *SceneLoader::loadModelMotion(const QString &path, QList<IModel *> &models)
{
    /* モーションをファイルから読み込み、対象の全てのモデルに対してモーションを適用する */
    IMotion *motion = loadModelMotion(path);
    if (motion) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = m_project->model(modelUUIDs[i]);
            setModelMotion(motion, model);
            models.append(model);
        }
    }
    return motion;
}

IMotion *SceneLoader::loadModelMotion(const QString &path, IModel *model)
{
    /* loadModelMotion に setModelMotion の追加が入ったショートカット的なメソッド */
    IMotion *motion = loadModelMotion(path);
    if (motion)
        setModelMotion(motion, model);
    return motion;
}

VPDFilePtr SceneLoader::loadModelPose(const QString &path, IModel *model)
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
        const Vector3 &color = UIGetVector3(m_project->globalSetting("light.color"), Vector3(0.6, 0.6, 0.6));
        const Vector3 &position = UIGetVector3(m_project->globalSetting("light.direction"), Vector3(-0.5, -1.0, -0.5));
        setLightColor(Color(color.x(), color.y(), color.z(), 1.0));
        setLightDirection(position);
        /* アクセラレーションの有効化(有効にしている場合) */
        if (isAccelerationEnabled()) {
#if QMA2_TBD
            if (m_renderer->initializeAccelerator())
                m_renderer->scene()->setSoftwareSkinningEnable(false);
#endif
        }
        int progress = 0;
        QList<IModel *> lostModels;
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        emit projectDidCount(modelUUIDs.size());
        const Array<IMotion *> &motions = m_project->motions();
        const int nmotions = motions.count();
        /* Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        int nmodels = modelUUIDs.size();
        Quaternion rotation;
        UIDelegate *delegate = static_cast<UIDelegate *>(m_renderDelegate);
        for (int i = 0; i < nmodels; i++) {
            const Project::UUID &modelUUIDString = modelUUIDs[i];
            IModel *model = m_project->model(modelUUIDString);
            const std::string &name = m_project->modelSetting(model, Project::kSettingNameKey);
            const std::string &uri = m_project->modelSetting(model, Project::kSettingURIKey);
            const QString &filename = QString::fromStdString(uri);
            if (loadModel(filename, model)) {
                const QFileInfo fileInfo(filename);
                internal::String d(fileInfo.absolutePath());
                IRenderEngine *engine = m_project->createRenderEngine(m_renderDelegate, model);
                engine->upload(&d);
                scene()->addModel(model, engine);
                IModel::Type type = model->type();
                if (type == IModel::kPMD || type == IModel::kPMX) {
                    delegate->setArchive(0);
                    /* ModelInfoWidget でエッジ幅の値を設定するので modelDidSelect を呼ぶ前に設定する */
                    // const Vector3 &color = UIGetVector3(m_project->modelSetting(model, "edge.color"), kZeroV3);
                    // model->setEdgeColor(Color(color.x(), color.y(), color.z(), 1.0));
                    // model->setEdgeOffset(QString::fromStdString(m_project->modelSetting(model, "edge.offset")).toFloat());
                    model->setPosition(UIGetVector3(m_project->modelSetting(model, "offset.position"), kZeroV3));
                    /* 角度で保存されるので、オイラー角を用いて Quaternion を構築する */
                    const Vector3 &angle = UIGetVector3(m_project->modelSetting(model, "offset.rotation"), kZeroV3);
                    rotation.setEulerZYX(radian(angle.x()), radian(angle.y()), radian(angle.z()));
                    // model->setRotationOffset(rotation);
                    const QUuid modelUUID(modelUUIDString.c_str());
                    m_renderOrderList.add(modelUUID);
                    emit modelDidAdd(model, modelUUID);
                    if (isModelSelected(model))
                        setSelectedModel(model);
                    /* モデルに属するモーションを取得し、追加する */
                    for (int i = 0; i < nmotions; i++) {
                        IMotion *motion = motions[i];
                        if (motion->parentModel() == model) {
                            const Project::UUID &motionUUIDString = m_project->motionUUID(motion);
                            const QUuid motionUUID(motionUUIDString.c_str());
                            motion->setParentModel(model);
                            emit motionDidAdd(motion, model, motionUUID);
                        }
                    }
                    emit projectDidProceed(++progress);
                    continue;
                }
                else if (type == IModel::kAsset) {
                    internal::String s(fileInfo.baseName().toUtf8());
                    model->setName(&s);
                    delegate->setArchive(0);
                    const QUuid assetUUID(modelUUIDString.c_str());
                    m_renderOrderList.add(assetUUID);
                    emit assetDidAdd(model, assetUUID);
                    if (isAssetSelected(model))
                        setSelectedAsset(model);
                    emit projectDidProceed(++progress);
                    continue;
                }
            }
            /* 読み込みに失敗したモデルは後で Project から削除するため失敗したリストに追加する */
            qWarning("Model(uuid=%s, name=%s, path=%s) cannot be loaded",
                     modelUUIDString.c_str(),
                     name.c_str(),
                     qPrintable(filename));
            lostModels.append(model);
            emit projectDidProceed(++progress);
        }
        /* カメラモーションの読み込み(親モデルがないことが前提。複数存在する場合最後に読み込まれたモーションが適用される) */
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i];
            if (!motion->parentModel() && motion->countKeyframes(IKeyframe::kCamera) > 0) {
                const QUuid uuid(m_project->motionUUID(motion).c_str());
                deleteCameraMotion();
                m_camera = motion;
                m_project->camera()->setMotion(motion);
                emit cameraMotionDidSet(motion, uuid);
            }
        }
        /* 読み込みに失敗したモデルとアクセサリを Project から削除する */
        foreach (IModel *model, lostModels) {
            m_project->removeModel(model);
            m_project->deleteModel(model);
        }
        updateDepthBuffer(shadowMapSize());
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

IMotion *SceneLoader::newCameraMotion() const
{
    /* 0番目に空のキーフレームが入ったカメラのモーションを作成する */
    QScopedPointer<IMotion> newCameraMotion(m_factory->createMotion());
    QScopedPointer<ICameraKeyframe> frame(m_factory->createCameraKeyframe());
    Scene::ICamera *camera = m_project->camera();
    frame->setDefaultInterpolationParameter();
    frame->setPosition(camera->position());
    frame->setAngle(camera->angle());
    frame->setFovy(camera->fov());
    frame->setDistance(camera->distance());
    newCameraMotion->addKeyframe(frame.take());
    newCameraMotion->update(IKeyframe::kCamera);
    return newCameraMotion.take();
}

IMotion *SceneLoader::newModelMotion(IModel *model) const
{
    /* 全ての可視ボーンと頂点モーフに対して0番目に空のキーフレームが入ったモデルのモーションを作成する */
    QScopedPointer<IMotion> newModelMotion;
    if (model) {
        newModelMotion.reset(m_factory->createMotion());
        Array<IBone *> bones;
        model->getBones(bones);
        const int nbones = bones.count();
        QScopedPointer<IBoneKeyframe> boneKeyframe;
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable()) {
                boneKeyframe.reset(m_factory->createBoneKeyframe());
                boneKeyframe->setDefaultInterpolationParameter();
                boneKeyframe->setName(bone->name());
                newModelMotion->addKeyframe(boneKeyframe.take());
            }
        }
        newModelMotion->update(IKeyframe::kBone);
        Array<IMorph *> morphs;
        model->getMorphs(morphs);
        const int nmorphs = morphs.count();
        QScopedPointer<IMorphKeyframe> morphKeyframe;
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morph = morphs[i];
            morphKeyframe.reset(m_factory->createMorphKeyframe());
            morphKeyframe->setName(morph->name());
            newModelMotion->addKeyframe(morphKeyframe.take());
        }
        newModelMotion->update(IKeyframe::kMorph);
    }
    return newModelMotion.take();
}

void SceneLoader::release()
{
    /* やっていることは削除ではなくシグナル発行すること以外 Renderer::releaseProject と同じ */
    const Project::UUIDList &motionUUIDs = m_project->motionUUIDs();
    for (Project::UUIDList::const_iterator it = motionUUIDs.begin(); it != motionUUIDs.end(); it++) {
        const Project::UUID &motionUUID = *it;
        IMotion *motion = m_project->motion(motionUUID);
        if (motion)
            emit motionWillDelete(motion, QUuid(motionUUID.c_str()));
    }
    const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
    for (Project::UUIDList::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        const Project::UUID &modelUUID = *it;
        IModel *model = m_project->model(modelUUID);
        if (model)
            emit modelWillDelete(model, QUuid(modelUUID.c_str()));
    }
    m_renderOrderList.clear();
    deleteCameraMotion();
    delete m_project;
    m_project = 0;
    m_asset = 0;
    m_model = 0;
}

void SceneLoader::renderModels()
{
    UIEnableMultisample();
    /* 順番にそってレンダリング開始 */
    const int nobjects = m_renderOrderList.count();
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (IModel *model = m_project->model(uuidString)) {
            IRenderEngine *engine = m_project->renderEngine(model);
#ifdef IS_QMA2
            if (isProjectiveShadowEnabled(model) && !isSelfShadowEnabled(model)) {
#else
            if (true) {
#endif
                engine->renderShadow();
            }
            engine->renderEdge();
            engine->renderModel();
        }
    }
}

void SceneLoader::renderZPlot()
{
    Scene::IMatrices *matrices = m_project->matrices();
    Scene::ILight *light = m_project->light();
    QMatrix4x4 shadowMatrix;
    Vector3 center;
    Scalar radius;
    float shadowMatrix4x4[16];
    /* プロジェクトにバウンディングスフィアの設定があればそちらを適用し、無ければ計算する */
    const Vector4 &boundingSphere = shadowBoundingSphere();
    if (!boundingSphere.isZero() && !btFuzzyZero(boundingSphere.w())) {
        center = boundingSphere;
        radius = boundingSphere.w();
    }
    else {
        getBoundingSphere(center, radius);
    }
    const Scalar &angle = 45;
    const Scalar &distance = radius / btSin(btRadians(angle) * 0.5);
    const Scalar &margin = 50;
    const Vector3 &eye = -light->direction() * radius * 2 + center;
    shadowMatrix.perspective(angle, 1, 1, distance + radius + margin);
    shadowMatrix.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                        QVector3D(center.x(), center.y(), center.z()),
                        QVector3D(0, 1, 0));
    for (int i = 0; i < 16; i++)
        shadowMatrix4x4[i] = shadowMatrix.constData()[i];
    matrices->setLightViewProjection(shadowMatrix4x4);
    /* デプスバッファのテクスチャにレンダリング */
    glDisable(GL_BLEND);
    m_depthBuffer->bind();
    glViewport(0, 0, m_depthBuffer->width(), m_depthBuffer->height());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const int nobjects = m_renderOrderList.count();
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        IModel *model = m_project->model(uuidString);
        if (model && isSelfShadowEnabled(model)) {
            IRenderEngine *engine = m_project->renderEngine(model);
            engine->renderZPlot();
        }
    }
    m_depthBuffer->release();
    glEnable(GL_BLEND);
    /* デプスバッファの読み込みに必要な行列を作成する */
    QMatrix4x4 m;
    m.scale(0.5);
    m.translate(1, 1, 1);
    shadowMatrix = m * shadowMatrix;
    for (int i = 0; i < 16; i++)
        shadowMatrix4x4[i] = shadowMatrix.constData()[i];
    matrices->setLightViewProjection(shadowMatrix4x4);
    light->setDepthTexture(&m_depthBufferID);
}

void SceneLoader::updateDepthBuffer(const QSize &value)
{
    delete m_depthBuffer;
    if (!value.isEmpty())
        m_depthBuffer = new QGLFramebufferObject(value, QGLFramebufferObject::Depth);
    else
        m_depthBuffer = new QGLFramebufferObject(1024, 1024, QGLFramebufferObject::Depth);
    m_depthBufferID = m_depthBuffer->texture();
}

void SceneLoader::updateMatrices(const QSizeF &size)
{
    /* モデル行列とビュー行列の乗算を QMatrix4x4 を用いて行う */
    QMatrix4x4 modelView4x4;
    float modelViewMatrixf[16], projectionMatrixf[16];
    Scene::ICamera *camera = m_project->camera();
    Scene::IMatrices *matrices = m_project->matrices();
    matrices->getModelView(modelViewMatrixf);
    m_projection.setToIdentity();
    qreal aspectRatio = size.width() / size.height();
    m_projection.perspective(camera->fov(), aspectRatio, camera->znear(), camera->zfar());
    for (int i = 0; i < 16; i++) {
        modelView4x4.data()[i] = modelViewMatrixf[i];
        projectionMatrixf[i] = m_projection.constData()[i];
    }
    const QMatrix4x4 &modelViewProjection4x4 = m_projection * modelView4x4;
    for (int i = 0; i < 16; i++)
        modelViewMatrixf[i] = modelViewProjection4x4.constData()[i];
    matrices->setModelViewProjection(modelViewMatrixf);
}

const QList<QUuid> SceneLoader::renderOrderList() const
{
    QList<QUuid> r;
    int n = m_renderOrderList.count();
    for (int i = 0; i < n; i++)
        r.append(m_renderOrderList[i]);
    return r;
}

void SceneLoader::saveMetadataFromAsset(const QString &path, IModel *asset)
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
        const IBone *bone = asset->parentBone();
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

void SceneLoader::setCameraMotion(IMotion *motion)
{
    const QUuid &uuid = QUuid::createUuid();
    deleteCameraMotion();
    m_camera = motion;
    m_project->addMotion(motion, uuid.toString().toStdString());
    m_project->camera()->setMotion(motion);
    emit cameraMotionDidSet(motion, uuid);
}

void SceneLoader::setLightColor(const Vector3 &color)
{
    m_project->light()->setColor(color);
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", color.x(), color.y(), color.z());
        m_project->setGlobalSetting("light.color", str.toStdString());
    }
    emit lightColorDidSet(color);
}

void SceneLoader::setLightDirection(const Vector3 &position)
{
    m_project->light()->setDirection(position);
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", position.x(), position.y(), position.z());
        m_project->setGlobalSetting("light.direction", str.toStdString());
    }
    emit lightDirectionDidSet(position);
}

void SceneLoader::setModelMotion(IMotion *motion, IModel *model)
{
    const QUuid &uuid = QUuid::createUuid();
#ifdef IS_QMA2
    /* 物理削除を伴うので、まず配列のコピーを用意してそこにコピーしてから削除。そうしないと SEGV が起きる */
    Array<IMotion *> motions;
    motions.copy(m_project->motions());
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *m = motions[i];
        if (m->parentModel() == model) {
            m_project->removeMotion(m);
            delete m;
        }
    }
#endif
    motion->setParentModel(model);
    m_project->addMotion(motion, uuid.toString().toStdString());
    emit motionDidAdd(motion, model, uuid);
}

void SceneLoader::setRenderOrderList(const QList<QUuid> &value)
{
    int i = 1;
    foreach (const QUuid &uuid, value) {
        const Project::UUID &u = uuid.toString().toStdString();
        const std::string &n = QVariant(i).toString().toStdString();
        if (IModel *model = m_project->model(u)) {
            m_project->setModelSetting(model, "order", n);
        }
        i++;
    }
    sort(true);
}

void SceneLoader::sort(bool useOrderAttr)
{
    if (m_project)
        m_renderOrderList.sort(UIRenderOrderPredication(m_project, m_project->camera()->modelViewTransform(), useOrderAttr));
}

void SceneLoader::startPhysicsSimulation()
{
    /* 物理暴走を防ぐために少し進めてから開始する */
    if (isPhysicsEnabled()) {
        btDiscreteDynamicsWorld *world = m_world->mutableWorld();
        const Array<IModel *> &models = m_project->models();
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            model->joinWorld(world);
        }
        world->stepSimulation(1, 60);
    }
}

void SceneLoader::stopPhysicsSimulation()
{
    btDiscreteDynamicsWorld *world = m_world->mutableWorld();
    const Array<IModel *> &models = m_project->models();
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        model->leaveWorld(world);
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

bool SceneLoader::isProjectiveShadowEnabled(const IModel *model) const
{
    return m_project ? m_project->modelSetting(model, "shadow.projective") == "true" : false;
}

void SceneLoader::setProjectiveShadowEnable(const IModel *model, bool value)
{
    if (m_project)
        m_project->setModelSetting(model, "shadow.projective", value ? "true" : "false");
}

bool SceneLoader::isSelfShadowEnabled(const IModel *model) const
{
    return m_project ? m_project->modelSetting(model, "shadow.ss") == "true" : false;
}

void SceneLoader::setSelfShadowEnable(const IModel *model, bool value)
{
    if (m_project)
        m_project->setModelSetting(model, "shadow.ss", value ? "true" : "false");
}

IModel *SceneLoader::selectedModel() const
{
    return m_model;
}

bool SceneLoader::isModelSelected(const IModel *value) const
{
    return m_project ? m_project->modelSetting(value, "selected") == "true" : false;
}

void SceneLoader::setSelectedModel(IModel *value)
{
    if (m_project && value != m_model) {
        m_model = value;
        m_project->setModelSetting(value, "selected", "true");
        if (signal)
            emit modelDidSelect(value, this);
    }
}

void SceneLoader::setModelEdgeOffset(IModel *model, float value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f", value);
        // model->setEdgeOffset(value);
        m_project->setModelSetting(model, "edge.offset", str.toStdString());
    }
}

void SceneLoader::setModelPosition(IModel *model, const Vector3 &value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        model->setPosition(value);
        m_project->setModelSetting(model, "offset.position", str.toStdString());
    }
}

const Vector3 SceneLoader::modelRotation(IModel *value) const
{
    return m_project ? UIGetVector3(m_project->modelSetting(value, "offset.rotation"), kZeroV3) : kZeroV3;
}

void SceneLoader::setModelRotation(IModel *model, const Vector3 &value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_project->setModelSetting(model, "offset.rotation", str.toStdString());
        Quaternion rotation;
        rotation.setEulerZYX(radian(value.x()), radian(value.y()), radian(value.z()));
        model->setRotation(rotation);
    }
}

void SceneLoader::setModelEdgeColor(IModel *model, const QColor &value)
{
    if (m_project) {
        QString str;
        float red = value.redF(), green = value.greenF(), blue = value.blueF();
        str.sprintf("%.5f,%.5f,%.5f", red, green, blue);
        // model->setEdgeColor(Color(red, green, blue, 1.0));
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
#if QMA2_TBD
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
#else
    Q_UNUSED(value)
#endif
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
    return globalSetting("play.frame_index.to", int(m_project->maxFrameIndex()));
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
    return globalSetting("video.frame_index.to", int(m_project->maxFrameIndex()));
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

const Vector3 SceneLoader::assetPosition(const IModel *asset)
{
    return m_project ? UIGetVector3(m_project->modelSetting(asset, "position"), kZeroV3) : kZeroV3;
}

void SceneLoader::setAssetPosition(const IModel *asset, const Vector3 &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_project->setModelSetting(asset, "position", str.toStdString());
    }
}

const Quaternion SceneLoader::assetRotation(const IModel *asset)
{
    return m_project ? UIGetQuaternion(m_project->modelSetting(asset, "rotation"), Quaternion::getIdentity()) : Quaternion::getIdentity();
}

void SceneLoader::setAssetRotation(const IModel *asset, const Quaternion &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
        m_project->setModelSetting(asset, "rotation", str.toStdString());
    }
}

float SceneLoader::assetOpacity(const IModel *asset)
{
    if (m_project) {
        float value = QString::fromStdString(m_project->modelSetting(asset, "opacity")).toFloat();
        return qBound(0.0f, value, 1.0f);
    }
    return 1.0f;
}

void SceneLoader::setAssetOpacity(const IModel *asset, float value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f", value);
        m_project->setModelSetting(asset, "opacity", str.toStdString());
    }
}

float SceneLoader::assetScaleFactor(const IModel *asset)
{
    if (m_project) {
        float value = QString::fromStdString(m_project->modelSetting(asset, "scale")).toFloat();
        return qBound(0.0001f, value, 10000.0f);
    }
    return 10.0f;
}

void SceneLoader::setAssetScaleFactor(const IModel *asset, float value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f", value);
        m_project->setModelSetting(asset, "scale", str.toStdString());
    }
}

IModel *SceneLoader::assetParentModel(IModel *asset) const
{
    return m_project ? m_project->model(m_project->modelSetting(asset, "parent.model")) : 0;
}

void SceneLoader::setAssetParentModel(const IModel *asset, IModel *model)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.model", m_project->modelUUID(model));
}

IBone *SceneLoader::assetParentBone(IModel *asset) const
{
    IModel *model = 0;
    if (m_project && (model = assetParentModel(asset))) {
        const QString &name = QString::fromStdString(m_project->modelSetting(asset, "parent.bone"));
        internal::String s(name);
        return model->findBone(&s);
    }
    return 0;
}

void SceneLoader::setAssetParentBone(const IModel *asset, IBone *bone)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.bone", internal::toQString(bone).toStdString());
}

IModel *SceneLoader::selectedAsset() const
{
    return m_asset;
}

bool SceneLoader::isAssetSelected(const IModel *value) const
{
    return m_project ? m_project->modelSetting(value, "selected") == "true" : false;
}

void SceneLoader::setSelectedAsset(IModel *value)
{
    if (m_project) {
        commitAssetProperties();
        m_asset = value;
        m_project->setModelSetting(value, "selected", "true");
        emit assetDidSelect(value, this);
    }
}

void SceneLoader::setPreferredFPS(int value)
{
    m_world->setPreferredFPS(value);
}

void SceneLoader::setScreenColor(const QColor &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.redF(), value.greenF(), value.blueF());
        m_project->setGlobalSetting("screen.color", str.toStdString());
    }
}

const QSize SceneLoader::shadowMapSize() const
{
    static const Vector3 defaultValue(1024, 1024, 0);
    if (m_project) {
        const std::string &value = m_project->globalSetting("shadow.texture.size");
        Vector3 s = UIGetVector3(value, defaultValue);
        /* テクスチャのサイズが 128px 未満または2乗の数ではない場合はデフォルトのサイズを適用するように強制する */
        if (s.x() < 128 || s.y() < 128 || !UIIsPowerOfTwo(s.x()) || !UIIsPowerOfTwo(s.y()))
            s = defaultValue;
        return QSize(s.x(), s.y());
    }
    return QSize(defaultValue.x(), defaultValue.y());
}

void SceneLoader::setShadowMapSize(const QSize &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%d,%d,0", value.width(), value.height());
        m_project->setGlobalSetting("shadow.texture.size", str.toStdString());
        updateDepthBuffer(value);
    }
}

const Vector4 SceneLoader::shadowBoundingSphere() const
{
    return m_project ? UIGetVector4(m_project->globalSetting("shadow.sphere"), kZeroV4) : kZeroV4;
}

void SceneLoader::setShadowBoundingSphere(const Vector4 &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.f,%.f,%.f,%.f", value.x(), value.y(), value.z(), value.w());
        m_project->setGlobalSetting("shadow.sphere", str.toStdString());
    }
}
bool SceneLoader::isSoftShadowEnabled() const
{
    return m_project ? QString::fromStdString(m_project->globalSetting("shadow.texture.soft")) == "true" : false;
}

void SceneLoader::setSoftShadowEnable(bool value)
{
    if (m_project) {
        m_project->setGlobalSetting("shadow.texture.soft", value ? "true" : "false");
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

Scene *SceneLoader::scene() const
{
    return m_project;
}

internal::World *SceneLoader::world() const
{
    return m_world;
}

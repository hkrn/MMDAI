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
#include <vpvl2/qt/Archive.h>
#include <vpvl2/qt/CString.h>
#include <vpvl2/qt/World.h>
#include <vpvl2/qt/RenderContext.h>

#include "SceneLoader.h"
#include "util.h"

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>
#include <vpvl2/vpvl2.h>

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

using namespace vpvl2;
using namespace vpvl2::qt;

namespace
{

typedef QScopedArrayPointer<uint8_t> ByteArrayPtr;

static const QRegExp &kAssetLoadable = QRegExp(".(bmp|dds|jpe?g|png|sp[ah]|tga|x)$");
static const QRegExp &kAssetExtensions = QRegExp(".x$");
static const QRegExp &kModelLoadable = QRegExp(".(bmp|dds|jpe?g|pm[dx]|png|sp[ah]|tga)$");
static const QRegExp &kModelExtensions = QRegExp(".pm[dx]$");

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
        IModel *lmodel = m_project->findModel(luuid), *rmodel = m_project->findModel(ruuid);
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

class ProjectDelegate : public Project::IDelegate {
public:
    ProjectDelegate() {}
    ~ProjectDelegate() {}

    const std::string toStdFromString(const IString *value) const {
        return static_cast<const CString *>(value)->value().toStdString();
    }
    const IString *toStringFromStd(const std::string &value) const {
        return new(std::nothrow) CString(QString::fromStdString(value));
    }
    void error(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }
    void warning(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }
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
                            RenderContext *renderContext)
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
                    renderContext->setArchive(archive);
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

namespace vpvm
{

SceneLoader::SceneLoader(IEncoding *encodingRef, Factory *factoryRef, RenderContext *renderContextRef)
    : QObject(),
      m_world(new World()),
      m_projectDelegate(new ProjectDelegate()),
      m_renderContextRef(renderContextRef),
      m_encodingRef(encodingRef),
      m_factoryRef(factoryRef),
      m_selectedModelRef(0),
      m_selectedAssetRef(0),
      m_depthBufferID(0)
{
    createProject();
}

SceneLoader::~SceneLoader()
{
    releaseProject();
}

void SceneLoader::addModel(IModel *model, const QString &baseName, const QDir &dir, QUuid &uuid)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    const QString &key = toQStringFromModel(model).trimmed();
    if (key.isEmpty()) {
        CString s(key);
        model->setName(&s);
    }
    const QString &path = dir.absoluteFilePath(baseName);
    m_renderContextRef->addModelPath(model, path);
    IRenderEnginePtr enginePtr;
    if (createModelEngine(model, dir, enginePtr)) {
        /* モデルを SceneLoader にヒモ付けする */
        uuid = QUuid::createUuid();
        m_project->addModel(model, enginePtr.take(), uuid.toString().toStdString());
        m_project->setModelSetting(model, Project::kSettingNameKey, key.toStdString());
        m_project->setModelSetting(model, Project::kSettingURIKey, path.toStdString());
        m_project->setModelSetting(model, "selected", "false");
        m_renderOrderList.add(uuid);
#ifndef IS_VPVM
        if (isPhysicsEnabled())
            m_world->addModel(model);
#endif
        emit modelDidAdd(model, uuid);
    }
}

bool SceneLoader::createModelEngine(IModel *model, const QDir &dir, IRenderEnginePtr &enginePtr)
{
    const CString d(dir.absolutePath());
    IEffect *effect = 0;
    int flags = 0;
    if (isEffectEnabled()) {
        const QFuture<IEffect *> &future = QtConcurrent::run(m_renderContextRef, &RenderContext::createEffectAsync, model, &d);
        /* progress dialog */
        /*
         * IEffect のインスタンスは Delegate#m_effectCaches が所有し、
         * プロジェクトの新規作成毎またはデストラクタで解放するため、解放する必要はない(むしろ解放してはいけない)
         */
        effect = future.result();
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (!effect) {
            qWarning("Loaded effect pointer seems null");
        }
        else if (!effect->internalPointer()) {
            CGcontext c = static_cast<CGcontext>(effect->internalContext());
            qWarning("Loading an effect failed: %s", cgGetLastListing(c));
        }
        else {
            flags = Scene::kEffectCapable;
        }
#endif
    }
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    enginePtr.reset(m_project->createRenderEngine(m_renderContextRef, model, flags));
    if (enginePtr->upload(&d)) {
        /* 先にエンジンにエフェクトを登録する。それからじゃないとオフスクリーンレンダーターゲットの取得が出来ないため */
        enginePtr->setEffect(IEffect::kAutoDetection, effect, &d);
        m_renderContextRef->parseOffscreenSemantic(effect, dir);
        m_renderContextRef->setArchive(0);
        return true;
    }
    return false;
}

QList<IModel *> SceneLoader::allModels() const
{
    const Project::UUIDList &uuids = m_project->modelUUIDs();
    QList<IModel *> models;
    Project::UUIDList::const_iterator it = uuids.begin(), end = uuids.end();
    while (it != end) {
        models.append(m_project->findModel(*it));
        it++;
    }
    return models;
}

void SceneLoader::commitAssetProperties()
{
    if (m_selectedAssetRef) {
        setAssetPosition(m_selectedAssetRef, m_selectedAssetRef->position());
        setAssetRotation(m_selectedAssetRef, m_selectedAssetRef->rotation());
        setAssetOpacity(m_selectedAssetRef, m_selectedAssetRef->opacity());
        setAssetScaleFactor(m_selectedAssetRef, m_selectedAssetRef->scaleFactor());
        setAssetParentModel(m_selectedAssetRef, m_selectedAssetRef->parentModel());
        setAssetParentBone(m_selectedAssetRef, m_selectedAssetRef->parentBone());
    }
}

void SceneLoader::createProject()
{
    if (!m_project) {
        m_project.reset(new Project(m_projectDelegate.data(), m_factoryRef));
        /*
         * デフォルトではグリッド表示と物理演算とソフトシャドウを有効にするため、設定後強制的に dirty フラグを無効にする
         * これによってアプリケーションを起動して何もしないまま終了する際の保存ダイアログを抑制する
         */
        m_project->setGlobalSetting("grid.visible", "true");
        m_project->setGlobalSetting("physics.enabled", "true");
        m_project->setGlobalSetting("shadow.texture.soft", "true");
        m_project->setDirty(false);
        m_renderContextRef->setSceneRef(m_project.data());
        emit projectDidInitialized();
    }
}

void SceneLoader::deleteCameraMotion()
{
    /* カメラモーションをシーンから解除及び削除し、最初の視点に戻しておく */
    ICamera *camera = m_project->camera();
    camera->setMotion(0);
    camera->resetDefault();
    m_project->removeMotion(m_camera.data());
    m_camera.reset();
}

void SceneLoader::deleteModel(IModel *&model)
{
    if (m_project->containsModel(model)) {
        const QUuid uuid(m_project->modelUUID(model).c_str());
        /*
         * 削除対象が選択中の場合選択対象から外して 0 にリセットする
         * ただし実際は先に setSelectedModel(0) で解除してしまうので予防策
         */
        if (model == m_selectedModelRef)
            setSelectedModel(0);
        if (model == m_selectedAssetRef)
            setSelectedAsset(0);
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
        m_renderContextRef->removeModel(model);
        m_project->removeModel(model);
        m_project->deleteModel(model);
        m_renderOrderList.remove(uuid);
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

IModel *SceneLoader::findModel(const QUuid &uuid) const
{
    return m_project->findModel(uuid.toString().toStdString());
}

IMotion *SceneLoader::findMotion(const QUuid &uuid) const
{
    return m_project->findMotion(uuid.toString().toStdString());
}

const QUuid SceneLoader::findUUID(const IModel *model) const
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
            // model->getBoundingSphere(c, r);
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

void SceneLoader::getCameraMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection) const
{
    m_renderContextRef->getCameraMatrices(world, view, projection);
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

bool SceneLoader::loadAsset(const QString &filename, QUuid &uuid, IModelPtr &assetPtr)
{
    IModel::Type type; /* unused */
    const QByteArray &bytes = UILoadFile(filename, kAssetLoadable, kAssetExtensions, type, m_renderContextRef);
    /*
     * アクセサリをファイルから読み込み、レンダリングエンジンに渡してレンダリング可能な状態にする
     */
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        assetPtr.reset(m_factoryRef->createModel(IModel::kAsset));
        if (assetPtr->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
            QFileInfo fileInfo(filename);
            CString name(fileInfo.completeBaseName());
            assetPtr->setName(&name);
            m_renderContextRef->addModelPath(assetPtr.data(), filename);
            IRenderEnginePtr enginePtr;
            if (createModelEngine(assetPtr.data(), fileInfo.dir(), enginePtr)) {
                uuid = QUuid::createUuid();
                m_project->addModel(assetPtr.data(), enginePtr.take(), uuid.toString().toStdString());
                m_project->setModelSetting(assetPtr.data(), Project::kSettingNameKey, fileInfo.completeBaseName().toStdString());
                m_project->setModelSetting(assetPtr.data(), Project::kSettingURIKey, filename.toStdString());
                m_project->setModelSetting(assetPtr.data(), "selected", "false");
                m_renderOrderList.add(uuid);
                setAssetPosition(assetPtr.data(), assetPtr->position());
                setAssetRotation(assetPtr.data(), assetPtr->rotation());
                setAssetOpacity(assetPtr.data(), assetPtr->opacity());
                setAssetScaleFactor(assetPtr.data(), assetPtr->scaleFactor());
                emit modelDidAdd(assetPtr.data(), uuid);
            }
        }
    }
    return !isNullData && assetPtr;
}

bool SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid, IModelPtr &modelPtr)
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
        modelPtr.reset(m_factoryRef->createModel(IModel::kAsset));
        if (loadAsset(dir.absoluteFilePath(filename), uuid, modelPtr)) {
            if (!name.isEmpty()) {
                CString s(name);
                modelPtr->setName(&s);
            }
            if (!filename.isEmpty()) {
                m_name2assets.insert(filename, modelPtr.data());
            }
            if (scaleFactor > 0)
                modelPtr->setScaleFactor(scaleFactor);
            if (position.count() == 3) {
                float x = position.at(0).toFloat();
                float y = position.at(1).toFloat();
                float z = position.at(2).toFloat();
                modelPtr->setPosition(Vector3(x, y, z));
            }
            if (rotation.count() == 3) {
                float x = rotation.at(0).toFloat();
                float y = rotation.at(1).toFloat();
                float z = rotation.at(2).toFloat();
                modelPtr->setRotation(Quaternion(x, y, z));
            }
            if (!bone.isEmpty() && m_selectedModelRef) {
                CString s(name);
                IBone *bone = m_selectedModelRef->findBone(&s);
                modelPtr->setParentBone(bone);
            }
            Q_UNUSED(enableShadow);
        }
        return true;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(baseName), qPrintable(file.errorString()));
        return false;
    }
}

bool SceneLoader::loadCameraMotion(const QString &path, IMotionPtr &motionPtr)
{
    /* カメラモーションをファイルから読み込み、場面オブジェクトに設定する */
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data.constData());
        motionPtr.reset(m_factoryRef->createMotion(Factory::findMotionType(ptr, data.size()), 0));
        if (motionPtr->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())
                && motionPtr->countKeyframes(IKeyframe::kCamera) > 0) {
            setCameraMotion(motionPtr.data());
            m_project->addMotion(motionPtr.data(), QUuid::createUuid().toString().toStdString());
        }
        else {
            motionPtr.reset(0);
        }
        return true;
    }
    return false;
}

bool SceneLoader::loadModel(const QString &filename, IModelPtr &modelPtr)
{
    /*
     * モデルをファイルから読み込む。レンダリングエンジンに送るには addModel を呼び出す必要がある
     * (確認ダイアログを出す必要があるので、読み込みとレンダリングエンジンへの追加は別処理)
     */
    IModel::Type type;
    const QByteArray &bytes = UILoadFile(filename, kModelLoadable, kModelExtensions, type, m_renderContextRef);
    bool isNullData = bytes.isNull();
    if (!isNullData) {
        if (modelPtr.isNull())
            modelPtr.reset(m_factoryRef->createModel(type));
        const uint8_t *dataPtr = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        if (!modelPtr->load(dataPtr, size)) {
            m_renderContextRef->setArchive(0);
        }
    }
    return !isNullData && modelPtr;
}

bool SceneLoader::loadModelMotion(const QString &path, IMotionPtr &motionPtr)
{
    /* モーションをファイルから読み込む。モデルへの追加は setModelMotion を使う必要がある */
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data.constData());
        motionPtr.reset(m_factoryRef->createMotion(Factory::findMotionType(ptr, data.size()), 0));
        if (!motionPtr->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            motionPtr.reset(0);
            return false;
        }
        return true;
    }
    return false;
}

bool SceneLoader::loadModelMotion(const QString &path, QList<IModel *> &models, IMotionPtr &motionPtr)
{
    /* モーションをファイルから読み込み、対象の全てのモデルに対してモーションを適用する */
    if (loadModelMotion(path, 0, motionPtr)) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = m_project->findModel(modelUUIDs[i]);
            setModelMotion(motionPtr.data(), model);
            models.append(model);
        }
    }
    return motionPtr;
}

bool SceneLoader::loadModelMotion(const QString &path, IModel *model, IMotionPtr &motionPtr)
{
    /* loadModelMotion に setModelMotion の追加が入ったショートカット的なメソッド */
    if (loadModelMotion(path, motionPtr)) {
        setModelMotion(motionPtr.data(), model);
        return true;
    }
    return false;
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
    releaseProject();
    createProject();
    bool ret = m_project->load(path.toLocal8Bit().constData());
    if (ret) {
        /* 光源設定 */
        const Vector3 &color = UIGetVector3(m_project->globalSetting("light.color"), Vector3(0.6, 0.6, 0.6));
        const Vector3 &position = UIGetVector3(m_project->globalSetting("light.direction"), Vector3(-0.5, -1.0, -0.5));
        setLightColor(Color(color.x(), color.y(), color.z(), 1.0));
        setLightDirection(position);
        int progress = 0;
        QSet<IModel *> lostModels;
        QList<IModel *> assets;
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        /* プロジェクト内のモデル数を発行する */
        emit projectDidCount(modelUUIDs.size());
        const Array<IMotion *> &motions = m_project->motions();
        const int nmotions = motions.count();
        /* Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        int nmodels = modelUUIDs.size();
        Quaternion rotation;
        Scene *sceneObject = sceneRef();
        Scene::AccelerationType accelerationType = globalAccelerationType();
        sceneObject->setAccelerationType(accelerationType);
        for (int i = 0; i < nmodels; i++) {
            const Project::UUID &modelUUIDString = modelUUIDs[i];
            IModelPtr modelPtr(m_project->findModel(modelUUIDString));
            const std::string &name = m_project->modelSetting(modelPtr.data(), Project::kSettingNameKey);
            const std::string &uri = m_project->modelSetting(modelPtr.data(), Project::kSettingURIKey);
            const QString &filename = QString::fromStdString(uri);
            if (loadModel(filename, modelPtr)) {
                const QFileInfo fileInfo(filename);
                IModel *model = modelPtr.take();
                m_renderContextRef->addModelPath(model, filename);
                IRenderEnginePtr enginePtr;
                if (createModelEngine(model, fileInfo.absoluteDir(), enginePtr)) {
                    /* モデル名がないときはファイル名で補完する */
                    if (!model->name()) {
                        const CString s(fileInfo.fileName());
                        model->setName(&s);
                    }
                    sceneObject->addModel(model, enginePtr.take());
                    sceneObject->setAccelerationType(modelAccelerationType(model));
                    IModel::Type type = model->type();
                    if (type == IModel::kPMD || type == IModel::kPMX) {
                        m_renderContextRef->setArchive(0);
                        /* ModelInfoWidget でエッジ幅の値を設定するので modelDidSelect を呼ぶ前に設定する */
                        const Vector3 &color = UIGetVector3(m_project->modelSetting(model, "edge.color"), kZeroV3);
                        model->setEdgeColor(color);
                        model->setEdgeWidth(QString::fromStdString(m_project->modelSetting(model, "edge.offset")).toFloat());
                        model->setPosition(UIGetVector3(m_project->modelSetting(model, "offset.position"), kZeroV3));
                        const std::string &os = m_project->modelSetting(model, "opacity");
                        model->setOpacity(os.empty() ? 1 : QString::fromStdString(os).toFloat());
                        /* 角度で保存されるので、オイラー角を用いて Quaternion を構築する */
                        const Vector3 &angle = UIGetVector3(m_project->modelSetting(model, "offset.rotation"), kZeroV3);
                        rotation.setEulerZYX(radian(angle.x()), radian(angle.y()), radian(angle.z()));
                        model->setRotation(rotation);
                        const QUuid modelUUID(modelUUIDString.c_str());
                        m_renderOrderList.add(modelUUID);
                        emit modelDidAdd(model, modelUUID);
                        /* (Bone|Morph)MotionModel#loadMotion で弾かれることを防ぐために先に選択状態にする */
                        if (isModelSelected(model))
                            setSelectedModel(model);
                        emitMotionDidAdd(motions, model);
                        emit projectDidProceed(++progress);
                        continue;
                    }
                    else if (type == IModel::kAsset) {
                        CString s(fileInfo.completeBaseName().toUtf8());
                        model->setName(&s);
                        m_renderContextRef->setArchive(0);
                        m_renderOrderList.add(QUuid(modelUUIDString.c_str()));
                        assets.append(model);
                        /* (Bone|Morph)MotionModel#loadMotion で弾かれることを防ぐために先に選択状態にする */
                        if (isAssetSelected(model))
                            setSelectedModel(model);
                        emitMotionDidAdd(motions, model);
                        emit projectDidProceed(++progress);
                        continue;
                    }
                }
            }
            /* 読み込みに失敗したモデルは後で Project から削除するため失敗したリストに追加する */
            qWarning("Model(uuid=%s, name=%s, path=%s) cannot be loaded",
                     modelUUIDString.c_str(),
                     name.c_str(),
                     qPrintable(filename));
            lostModels.insert(modelPtr.take());
            emit projectDidProceed(++progress);
        }
        sceneObject->setAccelerationType(accelerationType);
        /* カメラモーションの読み込み(親モデルがないことが前提。複数存在する場合最後に読み込まれたモーションが適用される) */
        Array<IMotion *> motionsToDelete;
        motionsToDelete.copy(motions);
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motionsToDelete[i];
            if (!motion->parentModel() && motion->countKeyframes(IKeyframe::kCamera) > 0) {
                const QUuid uuid(m_project->motionUUID(motion).c_str());
                deleteCameraMotion();
                m_camera.reset(motion);
                m_project->camera()->setMotion(motion);
                emit cameraMotionDidSet(motion, uuid);
            }
        }
        /* 読み込みに失敗したモデルに従属するモーションを Project から削除する */
        motionsToDelete.clear();
        motionsToDelete.copy(m_project->motions());
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motionsToDelete[i];
            if (lostModels.contains(motion->parentModel())) {
                m_project->removeMotion(motion);
            }
        }
        /* 読み込みに失敗したモデルとアクセサリを Project から削除する */
        foreach (IModel *model, lostModels) {
            m_renderContextRef->removeModel(model);
            m_project->removeModel(model);
            m_project->deleteModel(model);
        }
        /* ボーン追従の関係で assetDidAdd/assetDidSelect は全てのモデルとアクセサリ読み込みに行う */
        foreach (IModel *model, assets) {
            const QUuid assetUUID(m_project->modelUUID(model).c_str());
            model->setPosition(assetPosition(model));
            model->setRotation(assetRotation(model));
            model->setScaleFactor(assetScaleFactor(model));
            model->setOpacity(assetOpacity(model));
            model->setParentModel(assetParentModel(model));
            model->setParentBone(assetParentBone(model));
            emit modelDidAdd(model, assetUUID);
        }
        updateDepthBuffer(shadowMapSize());
        sort(true);
        m_project->setDirty(false);
        /* エフェクトが有効であればエフェクトボタンを有効にする */
        emit effectDidEnable(isEffectEnabled());
        /* 読み込み完了 */
        emit projectDidLoad(true);
    }
    else {
        qDebug("Failed loading project %s", qPrintable(path));
        createProject();
        emit projectDidLoad(false);
    }
}

void SceneLoader::newCameraMotion(IMotionPtr &motionPtr) const
{
    /* 0番目に空のキーフレームが入ったカメラのモーションを作成する */
    motionPtr.reset(m_factoryRef->createMotion(IMotion::kVMD, 0));
    QScopedPointer<ICameraKeyframe> cameraKeyframe(m_factoryRef->createCameraKeyframe(motionPtr.data()));
    QScopedPointer<ILightKeyframe> lightKeyframe(m_factoryRef->createLightKeyframe(motionPtr.data()));
    ICamera *camera = m_project->camera();
    ILight *light = m_project->light();
    cameraKeyframe->setDefaultInterpolationParameter();
    cameraKeyframe->setPosition(camera->lookAt());
    cameraKeyframe->setAngle(camera->angle());
    cameraKeyframe->setFov(camera->fov());
    cameraKeyframe->setDistance(camera->distance());
    lightKeyframe->setColor(light->color());
    lightKeyframe->setDirection(light->direction());
    motionPtr->addKeyframe(cameraKeyframe.take());
    motionPtr->addKeyframe(lightKeyframe.take());
    motionPtr->update(IKeyframe::kCamera);
    motionPtr->update(IKeyframe::kLight);
}

void SceneLoader::newModelMotion(const IModel *model, IMotionPtr &motionPtr) const
{
    /* 全ての可視ボーンと頂点モーフに対して0番目に空のキーフレームが入ったモデルのモーションを作成する */
    if (model) {
        motionPtr.reset(m_factoryRef->createMotion(IMotion::kVMD, 0));
        Array<IBone *> bones;
        model->getBoneRefs(bones);
        const int nbones = bones.count();
        QScopedPointer<IBoneKeyframe> boneKeyframe;
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable()) {
                boneKeyframe.reset(m_factoryRef->createBoneKeyframe(motionPtr.data()));
                boneKeyframe->setDefaultInterpolationParameter();
                boneKeyframe->setName(bone->name());
                boneKeyframe->setPosition(bone->localPosition());
                boneKeyframe->setRotation(bone->rotation());
                motionPtr->addKeyframe(boneKeyframe.take());
            }
        }
        motionPtr->update(IKeyframe::kBone);
        Array<IMorph *> morphs;
        model->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        QScopedPointer<IMorphKeyframe> morphKeyframe;
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morph = morphs[i];
            morphKeyframe.reset(m_factoryRef->createMorphKeyframe(motionPtr.data()));
            morphKeyframe->setName(morph->name());
            morphKeyframe->setWeight(morph->weight());
            motionPtr->addKeyframe(morphKeyframe.take());
        }
        motionPtr->update(IKeyframe::kMorph);
    }
}

void SceneLoader::releaseProject()
{
    const Project::UUIDList &motionUUIDs = m_project->motionUUIDs();
    for (Project::UUIDList::const_iterator it = motionUUIDs.begin(); it != motionUUIDs.end(); it++) {
        const Project::UUID &motionUUID = *it;
        IMotion *motion = m_project->findMotion(motionUUID);
        if (motion)
            emit motionWillDelete(motion, QUuid(motionUUID.c_str()));
    }
    const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
    for (Project::UUIDList::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        const Project::UUID &modelUUID = *it;
        IModel *model = m_project->findModel(modelUUID);
        emit modelWillDelete(model, QUuid(modelUUID.c_str()));
    }
    m_renderOrderList.clear();
    deleteCameraMotion();
    m_project.reset();
    m_selectedAssetRef = 0;
    m_selectedModelRef = 0;
}

void SceneLoader::renderWindow()
{
    const int nobjects = m_renderOrderList.count();
    /* ポストプロセスの前処理 */
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (IModel *model = m_project->findModel(uuidString)) {
            IRenderEngine *engine = m_project->findRenderEngine(model);
            IEffect *effect = engine->effect(IEffect::kPostProcess);
            /*
             * レンダリングエンジンの状態が自動更新されないことが原因でポストエフェクトで正しく処理されない問題があるため、
             * アクセサリの場合のみポストエフェクト処理前に事前にレンダリングエンジンの状態の更新を行う
             * (具体例は VIEWPORTPIXELSIZE が (0,0) になってしまい、それに依存するポストエフェクトが正しく描画されない問題)
             */
            if (model->type() == IModel::kAsset)
                engine->update();
            engine->setEffect(IEffect::kPostProcess, effect, 0);
            engine->preparePostProcess();
        }
    }
    /* プリプロセス */
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (IModel *model = m_project->findModel(uuidString)) {
            IRenderEngine *engine = m_project->findRenderEngine(model);
            IEffect *effect = engine->effect(IEffect::kPreProcess);
            engine->setEffect(IEffect::kPreProcess, effect, 0);
            engine->performPreProcess();
        }
    }
    emit preprocessDidPerform();
    /* 通常の描写 */
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (IModel *model = m_project->findModel(uuidString)) {
            IRenderEngine *engine = m_project->findRenderEngine(model);
            IEffect *effect = engine->effect(IEffect::kStandard);
            engine->setEffect(IEffect::kStandard, effect, 0);
            if (isProjectiveShadowEnabled(model) && !isSelfShadowEnabled(model)) {
                engine->renderShadow();
            }
            engine->renderModel();
            engine->renderEdge();
        }
    }
    /* ポストプロセス */
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        if (IModel *model = m_project->findModel(uuidString)) {
            IRenderEngine *engine = m_project->findRenderEngine(model);
            IEffect *effect = engine->effect(IEffect::kPostProcess);
            engine->setEffect(IEffect::kPostProcess, effect, 0);
            engine->performPostProcess();
        }
    }
    /* Cg でリセットされてしまうため、アルファブレンドを再度有効にする */
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SceneLoader::setLightViewProjectionMatrix()
{
    Vector3 center;
    Scalar radius;
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
    const Vector3 &eye = -m_project->light()->direction() * radius * 2 + center;
    QMatrix4x4 world, view, projection;
    projection.perspective(angle, 1, 1, distance + radius + margin);
    view.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                QVector3D(center.x(), center.y(), center.z()),
                QVector3D(0, 1, 0));
    m_renderContextRef->setLightMatrices(world, view, projection);
}

void SceneLoader::setMousePosition(const QMouseEvent *event, const QRect &geometry)
{
    const QPointF &pos = event->posF();
    const QSizeF &size = geometry.size() / 2;
    const qreal w = size.width(), h = size.height();
    const Vector3 &value = Vector3((pos.x() - w) / w, (pos.y() - h) / -h, 0);
    Qt::MouseButtons buttons = event->buttons();
    m_renderContextRef->setMousePosition(value, buttons & Qt::LeftButton, IRenderContext::kMouseLeftPressPosition);
    m_renderContextRef->setMousePosition(value, buttons & Qt::MiddleButton, IRenderContext::kMouseMiddlePressPosition);
    m_renderContextRef->setMousePosition(value, buttons & Qt::RightButton, IRenderContext::kMouseRightPressPosition);
    m_renderContextRef->setMousePosition(value, false, IRenderContext::kMouseCursorPosition);
}

void SceneLoader::bindDepthTexture()
{
    glDisable(GL_BLEND);
    m_depthBuffer->bind();
    glViewport(0, 0, m_depthBuffer->width(), m_depthBuffer->height());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneLoader::renderOffscreen(const QSize &size)
{
    m_renderContextRef->renderOffscreen(size);
}

void SceneLoader::renderZPlot()
{
    /* デプスバッファのテクスチャにレンダリング */
    const int nobjects = m_renderOrderList.count();
    for (int i = 0; i < nobjects; i++) {
        const QUuid &uuid = m_renderOrderList[i];
        const Project::UUID &uuidString = uuid.toString().toStdString();
        IModel *model = m_project->findModel(uuidString);
        if (model && isSelfShadowEnabled(model)) {
            IRenderEngine *engine = m_project->findRenderEngine(model);
            engine->renderZPlot();
        }
    }
}

void SceneLoader::renderZPlotToTexture()
{
    setLightViewProjectionMatrix();
    bindDepthTexture();
    renderZPlot();
    releaseDepthTexture();
    QMatrix4x4 world, view, projection;
    m_renderContextRef->getLightMatrices(world, view, projection);
    world.scale(0.5);
    world.translate(1, 1, 1);
    m_renderContextRef->setLightMatrices(world, view, projection);
}

void SceneLoader::releaseDepthTexture()
{
    m_depthBuffer->release();
    ILight *light = m_project->light();
    light->setDepthTexture(&m_depthBufferID);
    light->setDepthTextureSize(Vector3(m_depthBuffer->width(), m_depthBuffer->height(), 0));
    light->setSoftShadowEnable(isSoftShadowEnabled());
    glEnable(GL_BLEND);
}

void SceneLoader::updateDepthBuffer(const QSize &value)
{
    if (!value.isEmpty())
        m_depthBuffer.reset(new QGLFramebufferObject(value, QGLFramebufferObject::Depth));
    else
        m_depthBuffer.reset(new QGLFramebufferObject(1024, 1024, QGLFramebufferObject::Depth));
    m_depthBufferID = m_depthBuffer->texture();
}

void SceneLoader::updateMatrices(const QSizeF &size)
{
    m_renderContextRef->updateMatrices(size);
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
        stream << toQStringFromModel(asset) << lineSeparator;
        stream << m_name2assets.key(asset) << lineSeparator;
        stream << asset->scaleFactor() << lineSeparator;
        const Vector3 &position = asset->position();
        stream << QString("%1,%2,%3").arg(position.x(), 0, 'f', 1)
                  .arg(position.y(), 0, 'f', 1).arg(position.z(), 0, 'f', 1) << lineSeparator;
        const Quaternion &rotation = asset->rotation();
        stream << QString("%1,%2,%3").arg(rotation.x(), 0, 'f', 1)
                  .arg(rotation.y(), 0, 'f', 1).arg(rotation.z(), 0, 'f', 1) << lineSeparator;
        const IBone *bone = asset->parentBone();
        stream << (bone ? toQStringFromBone(bone) : "地面") << lineSeparator;
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
    m_camera.reset(motion);
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
    if (model) {
        const QUuid &uuid = QUuid::createUuid();
#ifdef IS_VPVM
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
}

void SceneLoader::setRenderOrderList(const QList<QUuid> &value)
{
    int i = 1;
    foreach (const QUuid &uuid, value) {
        const Project::UUID &u = uuid.toString().toStdString();
        const std::string &n = QVariant(i).toString().toStdString();
        if (IModel *model = m_project->findModel(u)) {
            m_project->setModelSetting(model, "order", n);
        }
        i++;
    }
    sort(true);
}

void SceneLoader::sort(bool useOrderAttr)
{
    if (m_project)
        m_renderOrderList.sort(UIRenderOrderPredication(m_project.data(), m_project->camera()->modelViewTransform(), useOrderAttr));
}

void SceneLoader::startPhysicsSimulation()
{
    /* 物理暴走を防ぐために少し進めてから開始する */
    if (isPhysicsEnabled()) {
        const Array<IModel *> &models = m_project->models();
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            m_world->addModel(model);
        }
        m_world->stepSimulation(1);
    }
}

void SceneLoader::stopPhysicsSimulation()
{
    const Array<IModel *> &models = m_project->models();
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        m_world->removeModel(model);
    }
}

const Vector3 SceneLoader::worldGravity() const
{
    static const Vector3 defaultGravity(0.0, -9.8, 0.0);
    const Vector3 &gravity = m_project ? UIGetVector3(m_project->globalSetting("physics.gravity"), defaultGravity) : defaultGravity;
    return gravity;
}

unsigned long SceneLoader::worldRandSeed() const
{
    if (m_project)
        return QVariant(QString::fromStdString(m_project->globalSetting("physics.randseed"))).toULongLong();    else
        return 0;
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

void SceneLoader::setWorldRandSeed(unsigned long value)
{
    if (m_project) {
        m_world->setRandSeed(value);
        m_project->setGlobalSetting("physics.randseed", QVariant(qulonglong(value)).toString().toStdString());
    }
}

bool SceneLoader::isProjectiveShadowEnabled(const IModel *model) const
{
    bool enabled = m_project ? m_project->modelSetting(model, "shadow.projective") == "true" : false;
    return enabled;
}

void SceneLoader::setProjectiveShadowEnable(const IModel *model, bool value)
{
    if (m_project && isProjectiveShadowEnabled(model) != value)
        m_project->setModelSetting(model, "shadow.projective", value ? "true" : "false");
}

bool SceneLoader::isSelfShadowEnabled(const IModel *model) const
{
    bool enabled = m_project ? m_project->modelSetting(model, "shadow.ss") == "true" : false;
    return enabled;
}

void SceneLoader::setSelfShadowEnable(const IModel *model, bool value)
{
    if (m_project && isSelfShadowEnabled(model) != value)
        m_project->setModelSetting(model, "shadow.ss", value ? "true" : "false");
}

bool SceneLoader::isOpenCLSkinningType1Enabled(const IModel *model) const
{
    bool enabled = m_project ? m_project->modelSetting(model, "skinning.opencl") == "true" : false;
    return enabled;
}

void SceneLoader::setOpenCLSkinningEnableType1(const IModel *model, bool value)
{
    if (m_project && isOpenCLSkinningType1Enabled(model) != value)
        m_project->setModelSetting(model, "skinning.opencl", value ? "true" : "false");
}

bool SceneLoader::isVertexShaderSkinningType1Enabled(const IModel *model) const
{
    bool enabled = m_project ? m_project->modelSetting(model, "skinning.vs.type1") == "true" : false;
    return enabled;
}

void SceneLoader::setVertexShaderSkinningType1Enable(const IModel *model, bool value)
{
    if (m_project && isVertexShaderSkinningType1Enabled(model) != value)
        m_project->setModelSetting(model, "skinning.vs.type1", value ? "true" : "false");
}

IModel *SceneLoader::selectedModelRef() const
{
    return m_selectedModelRef;
}

bool SceneLoader::isModelSelected(const IModel *value) const
{
    bool selected = m_project ? m_project->modelSetting(value, "selected") == "true" : false;
    return selected;
}

void SceneLoader::setSelectedModel(IModel *value)
{
    if (m_project && value != m_selectedModelRef) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        Project::UUIDList::const_iterator it = modelUUIDs.begin(), end = modelUUIDs.end();
        while (it != end) {
            IModel *model = m_project->findModel(*it);
            IModel::Type type = model->type();
            if (type == IModel::kPMD || type == IModel::kPMX)
                m_project->setModelSetting(model, "selected", "false");
            ++it;
        }
        m_selectedModelRef = value;
        m_project->setModelSetting(value, "selected", "true");
        emit modelDidSelect(value, this);
    }
}

void SceneLoader::setModelEdgeOffset(IModel *model, float value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f", value);
        model->setEdgeWidth(value);
        m_project->setModelSetting(model, "edge.offset", str.toStdString());
    }
}

void SceneLoader::setModelOpacity(IModel *model, const Scalar &value)
{
    if (m_project && model) {
        QString str;
        str.sprintf("%.5f", value);
        model->setOpacity(value);
        m_project->setModelSetting(model, "opacity", str.toStdString());
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
    const Vector3 &rotation = m_project ? UIGetVector3(m_project->modelSetting(value, "offset.rotation"), kZeroV3) : kZeroV3;
    return rotation;
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
        model->setEdgeColor(Color(red, green, blue, 1.0));
        m_project->setModelSetting(model, "edge.color", str.toStdString());
    }
}

bool SceneLoader::isGridVisible() const
{
    bool visible = globalSetting("grid.visible", true);
    return visible;
}

void SceneLoader::setGridVisible(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isGridVisible() != value)
        m_project->setGlobalSetting("grid.visible", value ? "true" : "false");
}

bool SceneLoader::isPhysicsEnabled() const
{
    bool enabled = globalSetting("physics.enabled", false);
    return enabled;
}

void SceneLoader::setPhysicsEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isPhysicsEnabled() != value)
        m_project->setGlobalSetting("physics.enabled", value ? "true" : "false");
}

/* 再生設定及びエンコード設定の場合は同値チェックを行わない。こちらは値を確実に保存させる必要があるため */
int SceneLoader::frameIndexPlayFrom() const
{
    int value = globalSetting("play.frame_index.from", 0);
    return value;
}

void SceneLoader::setFrameIndexPlayFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexPlayTo() const
{
    int value = globalSetting("play.frame_index.to", int(m_project->maxFrameIndex()));
    return value;
}

void SceneLoader::setFrameIndexPlayTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForPlay() const
{
    int value = globalSetting("play.fps", 60);
    return value;
}

void SceneLoader::setSceneFPSForPlay(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoFrom() const
{
    int value = globalSetting("video.frame_index.from", 0);
    return value;
}

void SceneLoader::setFrameIndexEncodeVideoFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoTo() const
{
    int value = globalSetting("video.frame_index.to", int(m_project->maxFrameIndex()));
    return value;
}

void SceneLoader::setFrameIndexEncodeVideoTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForEncodeVideo() const
{
    int value = globalSetting("video.fps", 60);
    return value;
}

void SceneLoader::setSceneFPSForEncodeVideo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneWidth() const
{
    int value = globalSetting("video.width", 0);
    return value;
}

void SceneLoader::setSceneWidth(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.width", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneHeight() const
{
    int value = globalSetting("video.height", 0);
    return value;
}

void SceneLoader::setSceneHeight(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.height", QVariant(value).toString().toStdString());
}

bool SceneLoader::isLoop() const
{
    bool value = globalSetting("play.loop", false);
    return value;
}

void SceneLoader::setLoop(bool value)
{
    if (m_project && value != isLoop())
        m_project->setGlobalSetting("play.loop", value ? "true" : "false");
}

bool SceneLoader::isGridIncluded() const
{
    bool included = globalSetting("grid.video", false);
    return included;
}

void SceneLoader::setGridIncluded(bool value)
{
    if (m_project)
        m_project->setGlobalSetting("grid.video", value ? "true" : "false");
}

const QString SceneLoader::backgroundAudio() const
{
    const QString &path = m_project ? QString::fromStdString(m_project->globalSetting("audio.path")) : "";
    return path;
}

void SceneLoader::setBackgroundAudioPath(const QString &value)
{
    if (m_project)
        m_project->setGlobalSetting("audio.path", value.toStdString());
}

const Vector3 SceneLoader::assetPosition(const IModel *asset)
{
    const Vector3 &position = m_project ? UIGetVector3(m_project->modelSetting(asset, "position"), kZeroV3) : kZeroV3;
    return position;
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
    const Quaternion &rotation = m_project ? UIGetQuaternion(m_project->modelSetting(asset, "rotation"), Quaternion::getIdentity()) : Quaternion::getIdentity();
    return rotation;
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

IModel *SceneLoader::assetParentModel(const IModel *asset) const
{
    IModel *parentModel = m_project ? m_project->findModel(m_project->modelSetting(asset, "parent.model")) : 0;
    return parentModel;
}

void SceneLoader::setAssetParentModel(const IModel *asset, IModel *model)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.model", m_project->modelUUID(model));
}

IBone *SceneLoader::assetParentBone(const IModel *asset) const
{
    IModel *model = 0;
    if (m_project && (model = assetParentModel(asset))) {
        const QString &name = QString::fromStdString(m_project->modelSetting(asset, "parent.bone"));
        CString s(name);
        return model->findBone(&s);
    }
    return 0;
}

void SceneLoader::setAssetParentBone(const IModel *asset, IBone *bone)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.bone", toQStringFromBone(bone).toStdString());
}

IModel *SceneLoader::selectedAsset() const
{
    return m_selectedAssetRef;
}

bool SceneLoader::isAssetSelected(const IModel *value) const
{
    bool selected = m_project ? m_project->modelSetting(value, "selected") == "true" : false;
    return selected;
}

void SceneLoader::setSelectedAsset(IModel *value)
{
    if (m_project) {
        commitAssetProperties();
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        Project::UUIDList::const_iterator it = modelUUIDs.begin(), end = modelUUIDs.end();
        while (it != end) {
            IModel *model = m_project->findModel(*it);
            if (model->type() == IModel::kAsset)
                m_project->setModelSetting(model, "selected", "false");
            ++it;
        }
        m_selectedAssetRef = value;
        m_project->setModelSetting(value, "selected", "true");
        emit modelDidSelect(value, this);
    }
}

void SceneLoader::setPreferredFPS(int value)
{
    m_project->setPreferredFPS(value);
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
    const Vector4 &value = m_project ? UIGetVector4(m_project->globalSetting("shadow.sphere"), kZeroV4) : kZeroV4;
    return value;
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
    bool enabled = m_project ? QString::fromStdString(m_project->globalSetting("shadow.texture.soft")) == "true" : true;
    return enabled;
}

void SceneLoader::setSoftShadowEnable(bool value)
{
    if (m_project && isSoftShadowEnabled() != value) {
        m_project->setGlobalSetting("shadow.texture.soft", value ? "true" : "false");
    }
}

const QString SceneLoader::backgroundImage() const
{
    const QString &path = m_project ? QString::fromStdString(m_project->globalSetting("background.image.path")) : "";
    return path;
}

void SceneLoader::setBackgroundImagePath(const QString &value)
{
    if (m_project) {
        m_project->setGlobalSetting("background.image.path", value.toStdString());
    }
}

const QPoint SceneLoader::backgroundImagePosition() const
{
    if (m_project) {
        const Vector3 &value = UIGetVector3(m_project->globalSetting("background.image.position"), kZeroV3);
        return QPoint(value.x(), value.y());
    }
    return QPoint();
}

void SceneLoader::setBackgroundImagePosition(const QPoint &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%d,%d,0", value.x(), value.y());
        m_project->setGlobalSetting("background.image.position", str.toStdString());
    }
}

bool SceneLoader::isBackgroundImageUniformEnabled() const
{
    bool enabled = m_project ? m_project->globalSetting("background.image.uniform") == "true" : false;
    return enabled;
}

void SceneLoader::setBackgroundImageUniformEnable(bool value)
{
    if (m_project && isBackgroundImageUniformEnabled() != value) {
        m_project->setGlobalSetting("background.image.uniform", value ? "true" : "false");
    }
}

bool SceneLoader::isOpenCLSkinningType1Enabled() const
{
    bool enabled = m_project ? m_project->globalSetting("skinning.opencl") == "true" : false;
    return enabled;
}

bool SceneLoader::isOpenCLSkinningType2Enabled() const
{
    bool enabled = m_project ? m_project->globalSetting("skinning.opencl.cpu") == "true" : false;
    return enabled;
}

void SceneLoader::setOpenCLSkinningEnableType1(bool value)
{
    if (m_project && isOpenCLSkinningType1Enabled() != value) {
        m_project->setGlobalSetting("skinning.opencl", value ? "true" : "false");
        if (value)
            m_project->setAccelerationType(Scene::kOpenCLAccelerationType1);
    }
}

void SceneLoader::setOpenCLSkinningEnableType2(bool value)
{
    if (m_project && isOpenCLSkinningType2Enabled() != value) {
        m_project->setGlobalSetting("skinning.opencl.cpu", value ? "true" : "false");
        if (value)
            m_project->setAccelerationType(Scene::kOpenCLAccelerationType2);
    }
}

bool SceneLoader::isVertexShaderSkinningType1Enabled() const
{
    bool enabled = m_project ? m_project->globalSetting("skinning.vs.type1") == "true" : false;
    return enabled;
}

bool SceneLoader::isEffectEnabled() const
{
    bool enabled = m_project ? m_project->globalSetting("effect.enabled") == "true" : false;
    return enabled;
}

void SceneLoader::setVertexShaderSkinningType1Enable(bool value)
{
    if (m_project && isVertexShaderSkinningType1Enabled() != value) {
        m_project->setGlobalSetting("skinning.vs.type1", value ? "true" : "false");
        if (value)
            m_project->setAccelerationType(Scene::kVertexShaderAccelerationType1);
    }
}

void SceneLoader::setSoftwareSkinningEnable(bool value)
{
    if (m_project && value)
        m_project->setAccelerationType(Scene::kSoftwareFallback);
}

void SceneLoader::setEffectEnable(bool value)
{
    if (m_project && isEffectEnabled() != value) {
        m_project->setGlobalSetting("effect.enabled", value ? "true" : "false");
        emit effectDidEnable(value);
    }
}

void SceneLoader::setProjectDirtyFalse()
{
    m_project->setDirty(false);
}

void SceneLoader::deleteModelSlot(IModel *model)
{
    IModel *modelPtr = model;
    deleteModel(modelPtr);
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

Scene::AccelerationType SceneLoader::globalAccelerationType() const
{
    if (isOpenCLSkinningType1Enabled())
        return Scene::kOpenCLAccelerationType1;
    else if (isVertexShaderSkinningType1Enabled())
        return Scene::kVertexShaderAccelerationType1;
    else
        return Scene::kSoftwareFallback;
}

Scene::AccelerationType SceneLoader::modelAccelerationType(const IModel *model) const
{
    if (isOpenCLSkinningType1Enabled(model))
        return Scene::kOpenCLAccelerationType1;
    else if (isVertexShaderSkinningType1Enabled(model))
        return Scene::kVertexShaderAccelerationType1;
    else
        return globalAccelerationType();
}

Scene *SceneLoader::sceneRef() const
{
    return m_project.data();
}

qt::World *SceneLoader::worldRef() const
{
    return m_world.data();
}

void SceneLoader::emitMotionDidAdd(const Array<IMotion *> &motions, IModel *model)
{
    const int nmotions = motions.count();
    /* モデルに属するモーションを取得し、追加する */
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        if (motion->parentModel() == model) {
            const Project::UUID &motionUUIDString = m_project->motionUUID(motion);
            const QUuid motionUUID(motionUUIDString.c_str());
            emit motionDidAdd(motion, model, motionUUID);
        }
    }
}

} /*  namespace vpvm */

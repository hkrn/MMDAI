/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "SceneLoader.h"

/* */
#define VPVL2_LINK_GLEW

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/World.h>
#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include <vpvl2/qt/RenderContext.h>
#include <vpvl2/qt/Util.h>

#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
#include <vpvl2/extensions/Archive.h>
#endif

#include <vpvl2/vpvl2.h>
#include <glm/gtc/matrix_transform.hpp>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

#include <QtCore>
#include <QMouseEvent>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent>
#endif

using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::qt;

namespace
{

typedef QScopedArrayPointer<uint8_t> ByteArrayPtr;

static inline const std::string UIFloatString(float value)
{
    QString str;
    str.sprintf("%.5f", value);
    return str.toStdString();
}

static inline const std::string UIVector3String(const Vector3 &value)
{
    QString str;
    str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
    return str.toStdString();
}

static inline const std::string UIQuaterionString(const Quaternion &value)
{
    QString str;
    str.sprintf("%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
    return str.toStdString();
}

static inline const std::string UIColorString(const QColor &value)
{
    return UIVector3String(Vector3(value.redF(), value.greenF(), value.blueF()));
}

static inline float UIGetFloat(const std::string &value, float def)
{
    if (!value.empty()) {
        return QString::fromStdString(value).toFloat();
    }
    return def;
}

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

static inline bool UIIsPowerOfTwo(int value)
{
    return (value & (value - 1)) == 0;
}

static inline IEffect *UICreateModelEffectRef(RenderContext *renderContext, IModel *m, const IString *d)
{
    return renderContext->createEffectRef(m, d);
}

static inline IEffect *UICreateGenericEffectRef(RenderContext *renderContext, const IString *p)
{
    return renderContext->createEffectRef(p);
}

class ProjectDelegate : public Project::IDelegate {
public:
    ProjectDelegate() {}
    ~ProjectDelegate() {}

    const std::string toStdFromString(const IString *value) const {
        return String::toStdString(static_cast<const String *>(value)->value());
    }
    const IString *toStringFromStd(const std::string &value) const {
        return new(std::nothrow) String(UnicodeString::fromUTF8(value));
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
void UISetModelType(const QFileInfo &finfo, IModel::Type &type)
{
    if (finfo.suffix() == "pmx")
        type = IModel::kPMXModel;
    else if (finfo.suffix() == "pmd")
        type = IModel::kPMDModel;
    else
        type = IModel::kAssetModel;
}

}

namespace vpvm
{

const QRegExp SceneLoader::kAllLoadable = QRegExp(".(bmp|dds|jpe?g|pm[dx]|png|sp[ah]|tga|x)$");
const QRegExp SceneLoader::kAllExtensions = QRegExp(".(pm[dx]|x)$");
const QRegExp SceneLoader::kAssetLoadable = QRegExp(".(bmp|dds|jpe?g|png|sp[ah]|tga|x)$");
const QRegExp SceneLoader::kAssetExtensions = QRegExp(".x$");
const QRegExp SceneLoader::kModelLoadable = QRegExp(".(bmp|dds|jpe?g|pm[dx]|png|sp[ah]|tga)$");
const QRegExp SceneLoader::kModelExtensions = QRegExp(".pm[dx]$");


#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
QStringList SceneLoader::toStringList(const Archive::EntryNames &value)
{
    QStringList ret;
    Archive::EntryNames::const_iterator it = value.begin();
    while (it != value.end()) {
        ret << Util::toQString(*it);
        ++it;
    }
    return ret;
}

void SceneLoader::getEntrySet(const QStringList &value, Archive::EntrySet &setRef)
{
    setRef.erase(setRef.begin(), setRef.end());
    setRef.clear();
    foreach (const QString &item, value) {
        setRef.insert(item.toStdString());
    }
}
#endif

SceneLoader::SceneLoader(IEncoding *encodingRef, Factory *factoryRef, RenderContext *renderContextRef)
    : QObject(),
      m_world(new World()),
      m_projectDelegate(new ProjectDelegate()),
      m_renderContextRef(renderContextRef),
      m_encodingRef(encodingRef),
      m_factoryRef(factoryRef),
      m_selectedModelRef(0),
      m_selectedAssetRef(0)
{
    newProject();
    QScopedPointer<btStaticPlaneShape> ground(new btStaticPlaneShape(Vector3(0, 1, 0), 0));
    btRigidBody::btRigidBodyConstructionInfo info(0, 0, ground.take(), kZeroV3);
    m_floor.reset(new btRigidBody(info));
    m_world->addRigidBody(m_floor.data());
}

SceneLoader::~SceneLoader()
{
    releaseProject();
    m_world->removeRigidBody(m_floor.data());
}

void SceneLoader::addAsset(IModelSharedPtr assetPtr, const QFileInfo &finfo, IRenderEnginePtr &enginePtr, QUuid &uuid)
{
    uuid = QUuid::createUuid();
    /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
    String name(Util::fromQString(finfo.baseName()));
    assetPtr->setName(&name);
    Array<IModel *> models;
    m_project->getModelRefs(models);
    m_project->addModel(assetPtr.data(), enginePtr.data(), uuid.toString().toStdString(), models.count());
    m_project->setModelSetting(assetPtr.data(), "selected", "false");
    const IModel *assetRef = assetPtr.data();
    setAssetPosition(assetRef, assetPtr->worldPosition());
    setAssetRotation(assetRef, assetPtr->worldRotation());
    setAssetOpacity(assetRef, assetPtr->opacity());
    setAssetScaleFactor(assetRef, assetPtr->scaleFactor());
}

void SceneLoader::addModel(IModelSharedPtr model, const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    const QString &key = Util::toQStringFromModel(model.data()).trimmed();
    if (key.isEmpty()) {
        String s(Util::fromQString(key));
        model->setName(&s);
    }
    bool isArchived = entry.filePath().isEmpty() ? false : true;
    m_renderContextRef->addModelPath(model.data(), Util::fromQString(isArchived ? entry.filePath() : finfo.filePath()));
    if (IRenderEnginePtr enginePtr = createModelEngine(model, finfo.dir())) {
        /* モデルを SceneLoader にヒモ付けする */
        uuid = QUuid::createUuid();
        Array<IModel *> models;
        m_project->getModelRefs(models);
        m_project->addModel(model.data(), enginePtr.data(), uuid.toString().toStdString(), models.count());
        m_project->setModelSetting(model.data(), Project::kSettingNameKey, key.toStdString());
        m_project->setModelSetting(model.data(), Project::kSettingURIKey, finfo.filePath().toStdString());
        m_project->setModelSetting(model.data(), "selected", "false");
        int options = isParallelSkinningEnabled() ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone;
        enginePtr->setUpdateOptions(options);
        /* zip ファイルならアーカイブ内のパスを保存する */
        if (isArchived) {
            m_project->setModelSetting(model.data(), Project::kSettingArchiveURIKey, entry.filePath().toStdString());
        }
        emit modelDidAdd(model, uuid);
    }
}

IRenderEnginePtr SceneLoader::createModelEngine(IModelSharedPtr model, const QDir &dir)
{
    int flags = 0;
    IEffect *effectRef = createEffectRef(model, dir.absolutePath(), flags);
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    const String d(Util::fromQString(dir.absolutePath()));
    IRenderEnginePtr engine(m_project->createRenderEngine(m_renderContextRef, model.data(), flags),
                            &Scene::deleteRenderEngineUnlessReferred);
    if (engine) {
        /* ミップマップの状態取得及びテクスチャ設定の処理関係で先にエフェクトを登録してからアップロードする */
        engine->setEffect(IEffect::kAutoDetection, effectRef, &d);
        engine->upload(&d);
        /* オフスクリーンレンダーターゲットの取得順序の関係でエフェクトを登録し、アップロードしてから呼び出す */
        m_renderContextRef->parseOffscreenSemantic(effectRef, &d);
    }
    return engine;
}

IEffect * SceneLoader::createEffectRef(IModelSharedPtr model, const QString &dirOrPath, int &flags)
{
    flags = 0;
    IEffect *effectRef = 0;
    if (isEffectEnabled()) {
        const String s(Util::fromQString(dirOrPath));
        QFuture<IEffect *> future;
        if (IModel *m = model.data()) {
            future = QtConcurrent::run(&UICreateModelEffectRef, m_renderContextRef, m, &s);
        }
        else {
            future = QtConcurrent::run(&UICreateGenericEffectRef, m_renderContextRef, &s);
        }
        /*
         * IEffect のインスタンスは Delegate#m_effectCaches が所有し、
         * プロジェクトの新規作成毎またはデストラクタで解放するため、解放する必要はない(むしろ解放してはいけない)
         */
        effectRef = future.result();
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (!effectRef) {
            qWarning("Loaded effect pointer seems null, using default fallback effect.");
        }
        else if (!effectRef->internalPointer()) {
            CGcontext c = static_cast<CGcontext>(effectRef->internalContext());
            qWarning("Loading an effect failed: %s", cgGetLastListing(c));
        }
        else {
            effectRef->createFrameBufferObject();
        }
#endif
        flags = Scene::kEffectCapable;
    }
    return effectRef;
}

void SceneLoader::handleFuture(QFuture<IModelSharedPtr> future, IModelSharedPtr &modelPtr) const
{
    while (!future.isResultReadyAt(0))
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    future.result().swap(modelPtr);
}

QByteArray SceneLoader::loadFile(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModel::Type &type)
{
    QByteArray bytes;
    QFileInfo finfo(path.first);
#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
    if (finfo.suffix() == "zip") {
        Archive::EntryNames files;
        ArchiveSmartPtr archive(new Archive(m_encodingRef));
        const String archivePath(Util::fromQString(finfo.filePath()));
        if (archive->open(&archivePath, files)) {
            Archive::EntrySet filtered;
            getEntrySet(toStringList(files).filter(loadable), filtered);
            if (!filtered.empty() && archive->uncompress(filtered)) {
                const QStringList &targets = toStringList(files).filter(extensions);
                if (!targets.isEmpty()) {
                    const QString &inArchivePath = path.second;
                    const std::string *bytesRef = 0;
                    QFileInfo fileInfoToLoad;
                    if (!inArchivePath.isEmpty() && targets.contains(inArchivePath)) {
                        bytesRef = archive->data(Util::fromQString(inArchivePath));
                        fileInfoToLoad.setFile(inArchivePath);
                    }
                    else {
                        const QString &filenameToLoad = targets.first();
                        bytesRef = archive->data(Util::fromQString(filenameToLoad));
                        fileInfoToLoad.setFile(filenameToLoad);
                    }
                    if (bytesRef) {
                        bytes.setRawData(bytesRef->data(), bytesRef->size());
                    }
                    /* ここではパスを置換して uploadTexture で読み込めるようにする */
                    archive->replaceFilePath(Util::fromQString(fileInfoToLoad.path()), Util::fromQString(finfo.path()) + "/");
                    m_renderContextRef->setArchive(archive.release());
                    UISetModelType(fileInfoToLoad, type);
                }
            }
        }
    }
#else
    if (false) {}
#endif
    else {
        QFile file(finfo.filePath());
        if (file.open(QFile::ReadOnly))
            bytes = file.readAll();
        UISetModelType(finfo, type);
    }
    return bytes;
}

IModelSharedPtr SceneLoader::loadModelFromBytesAsync(const QByteArray &bytes, IModel::Type type)
{
    if (!bytes.isEmpty()) {
        IModelSharedPtr modelPtr(m_factoryRef->newModel(type), &Scene::deleteModelUnlessReferred);
        if (!modelPtr->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            modelPtr.clear();
        return modelPtr;
    }
    IModelSharedPtr empty;
    return empty;
}

IModelSharedPtr SceneLoader::loadModelFromFileAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions)
{
    IModel::Type type;
    const QByteArray &bytes = loadFile(path, loadable, extensions, type);
    IModelSharedPtr model = loadModelFromBytesAsync(bytes, type);
    return model;
}

bool SceneLoader::loadModelFromFileDirectAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModelSharedPtr model)
{
    IModel::Type type; /* unused */
    const QByteArray &bytes = loadFile(path, loadable, extensions, type);
    return bytes.isEmpty() ? false : model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size());
}

void SceneLoader::restoreSceneStatesFromProject(Project *project)
{
    ICamera *camera = project->camera();
    camera->setAngle(UIGetVector3(project->globalSetting("state.camera.angle"), camera->angle()));
    camera->setDistance(UIGetFloat(project->globalSetting("state.camera.distance"), camera->distance()));
    camera->setFov(UIGetFloat(project->globalSetting("state.camera.fov"), camera->fov()));
    camera->setLookAt(UIGetVector3(project->globalSetting("state.camera.lookat"), camera->lookAt()));
    /* シグナル発行が必要なため内部のメソッドを使う */
    const ILight *light = project->light();
    setLightColor(UIGetVector3(project->globalSetting("state.light.color"), light->color()));
    setLightDirection(UIGetVector3(project->globalSetting("state.light.direction"), light->direction()));
}

void SceneLoader::applyParallelSkinning(bool value)
{
    Array<IModel *> models;
    m_project->getModelRefs(models);
    const int nmodels = models.count();
    int flags = value ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone;
    for (int i = 0; i < nmodels; i++) {
        IModel *m = models[i];
        if (IRenderEngine *engine = m_project->findRenderEngine(m)) {
            engine->setUpdateOptions(flags);
        }
    }
}

QList<IModelSharedPtr> SceneLoader::allModels() const
{
    const Project::UUIDList &uuids = m_project->modelUUIDs();
    QList<IModelSharedPtr> models;
    Project::UUIDList::const_iterator it = uuids.begin(), end = uuids.end();
    while (it != end) {
        models.append(IModelSharedPtr(m_project->findModel(*it), &Scene::deleteModelUnlessReferred));
        it++;
    }
    return models;
}

void SceneLoader::commitAssetProperties()
{
    if (m_selectedAssetRef) {
        IModelSharedPtr parentModelRef(m_selectedAssetRef->parentModelRef(), &Scene::deleteModelUnlessReferred);
        IModel *assetRef = m_selectedAssetRef.data();
        setAssetPosition(assetRef, m_selectedAssetRef->worldPosition());
        setAssetRotation(assetRef, m_selectedAssetRef->worldRotation());
        setAssetOpacity(assetRef, m_selectedAssetRef->opacity());
        setAssetScaleFactor(assetRef, m_selectedAssetRef->scaleFactor());
        setAssetParentModel(assetRef, parentModelRef);
        setAssetParentBone(assetRef, m_selectedAssetRef->parentBoneRef());
    }
}

void SceneLoader::newProject()
{
    ProjectPtr projectPtr;
    /* プロジェクトが別の参照になる前にカメラモーションの参照を外す (SceneMotionModel で問題になる) */
    deleteCameraMotion();
    newProject(projectPtr);
    m_renderContextRef->setSceneRef(projectPtr.data());
    /* m_project に上で作成したインスタンスを設定する。これは (new|set)CameraMotion が参照するため */
    m_project.reset(projectPtr.take());
    /* 空のカメラモーションを登録を行った後は setDirty(false) で何もしていないのにダイアログが出るのを防ぐ */
    createCameraMotion();
    m_project->setDirty(false);
    emit projectDidInitialized();
}

void SceneLoader::newProject(ProjectPtr &projectPtr)
{
    /*
     * デフォルトではグリッド表示と物理演算とソフトシャドウを有効にするため、設定後強制的に dirty フラグを無効にする
     * これによってアプリケーションを起動して何もしないまま終了する際の保存ダイアログを抑制する
     */
    projectPtr.reset(new Project(m_projectDelegate.data(), m_factoryRef, false));
    projectPtr->setGlobalSetting("grid.visible", "true");
    projectPtr->setGlobalSetting("physics.enabled", "true");
    projectPtr->setGlobalSetting("physics.floor", "true");
    projectPtr->setGlobalSetting("physics.substeps", QVariant(m_world->maxSubSteps()).toString().toStdString());
    projectPtr->setGlobalSetting("shadow.texture.soft", "true");
    projectPtr->setDirty(false);
}

void SceneLoader::deleteCameraMotion()
{
    if (m_project) {
        /* カメラモーションをシーンから解除及び削除し、最初の視点に戻しておく */
        ICamera *camera = m_project->camera();
        camera->setMotion(0);
        camera->resetDefault();
        m_project->removeMotion(m_camera.data());
        m_camera.clear();
    }
}

void SceneLoader::deleteModel(IModelSharedPtr model)
{
    if (m_project->containsModel(model.data())) {
        const QUuid uuid(m_project->modelUUID(model.data()).c_str());
        /*
         * 削除対象が選択中の場合選択対象から外して 0 にリセットする
         * ただし実際は先に setSelectedModel(0) で解除してしまうので予防策
         */
        if (model == m_selectedModelRef)
            setSelectedModel(IModelSharedPtr());
        if (model == m_selectedAssetRef)
            setSelectedAsset(IModelSharedPtr());
        emit modelWillDelete(model, uuid);
        /*
         * motionWillDelete を発行するため、削除対象のモデルに所属するモーションを削除する。
         * なお、データの物理削除を伴うため、配列に削除前のモーション配列をコピーしてから処理する必要がある。
         * そうしないと削除したモーションデータを参照してクラッシュしてしまう。
         */
        Array<IMotion *> motions;
        m_project->getMotionRefs(motions);
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotionSharedPtr motion(motions[i], &Scene::deleteMotionUnlessReferred);
            if (motion->parentModelRef() == model) {
                deleteMotion(motion);
            }
        }
        IModel *m = model.data();
        m_renderContextRef->removeModel(m);
        m_project->removeModel(m);
        model.clear();
    }
}

void SceneLoader::deleteMotion(IMotionSharedPtr motion)
{
    const QUuid uuid(m_project->motionUUID(motion.data()).c_str());
    emit motionWillDelete(motion, uuid);
    m_project->removeMotion(motion.data());
    motion.clear();
}

IModelSharedPtr SceneLoader::findModel(const QUuid &uuid) const
{
    return IModelSharedPtr(m_project->findModel(uuid.toString().toStdString()), &Scene::deleteModelUnlessReferred);
}

IMotionSharedPtr SceneLoader::findMotion(const QUuid &uuid) const
{
    return IMotionSharedPtr(m_project->findMotion(uuid.toString().toStdString()), &Scene::deleteMotionUnlessReferred);
}

const QUuid SceneLoader::findUUID(const IModel *model) const
{
    return QUuid(m_project->modelUUID(model).c_str());
}

void SceneLoader::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    Array<IModel *> models;
    m_project->getModelRefs(models);
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
            radiusArray.append(r);
            centerArray.append(c);
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

void SceneLoader::getCameraMatrices(glm::mat4 &world, glm::mat4 &view, glm::mat4 &projection) const
{
    m_renderContextRef->getCameraMatrices(world, view, projection);
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

bool SceneLoader::loadAsset(const QString &filename, QUuid &uuid, IModelSharedPtr &assetPtr)
{
    const QFuture<IModelSharedPtr> future = QtConcurrent::run(this,
                                                              &SceneLoader::loadModelFromFileAsync,
                                                              FilePathPair(filename, QString()),
                                                              kAssetLoadable,
                                                              kAssetExtensions);
    handleFuture(future, assetPtr);
    if (assetPtr) {
        const QFileInfo finfo(filename);
        m_renderContextRef->addModelPath(assetPtr.data(), Util::fromQString(filename));
        if (IRenderEnginePtr enginePtr = createModelEngine(assetPtr, finfo.dir())) {
            addAsset(assetPtr, finfo, enginePtr, uuid);
            m_project->setModelSetting(assetPtr.data(), Project::kSettingNameKey, finfo.completeBaseName().toStdString());
            m_project->setModelSetting(assetPtr.data(), Project::kSettingURIKey, filename.toStdString());
            emit modelDidAdd(assetPtr, uuid);
            return true;
        }
    }
    return false;
}

bool SceneLoader::loadAsset(const QByteArray &bytes, const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid, IModelSharedPtr &assetPtr)
{
    /*
     * アクセサリをファイルから読み込み、レンダリングエンジンに渡してレンダリング可能な状態にする
     */
    const QFuture<IModelSharedPtr> future = QtConcurrent::run(this, &SceneLoader::loadModelFromBytesAsync, bytes, IModel::kAssetModel);
    handleFuture(future, assetPtr);
    if (assetPtr) {
        m_renderContextRef->addModelPath(assetPtr.data(), Util::fromQString(finfo.path()));
        if (IRenderEnginePtr enginePtr = createModelEngine(assetPtr, finfo.dir())) {
            addAsset(assetPtr, finfo, enginePtr, uuid);
            m_project->setModelSetting(assetPtr.data(), Project::kSettingNameKey, entry.completeBaseName().toStdString());
            m_project->setModelSetting(assetPtr.data(), Project::kSettingURIKey, finfo.filePath().toStdString());
            m_project->setModelSetting(assetPtr.data(), Project::kSettingArchiveURIKey, entry.filePath().toStdString());
            emit modelDidAdd(assetPtr, uuid);
            return true;
        }
    }
    return false;
}

bool SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid, IModelSharedPtr &assetPtr)
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
        if (loadAsset(dir.absoluteFilePath(filename), uuid, assetPtr)) {
            if (!name.isEmpty()) {
                String s(Util::fromQString(name));
                assetPtr->setName(&s);
            }
            if (!filename.isEmpty()) {
                m_name2assets.insert(filename, assetPtr.data());
            }
            if (scaleFactor > 0)
                assetPtr->setScaleFactor(scaleFactor);
            if (position.count() == 3) {
                float x = position.at(0).toFloat();
                float y = position.at(1).toFloat();
                float z = position.at(2).toFloat();
                assetPtr->setWorldPosition(Vector3(x, y, z));
            }
            if (rotation.count() == 3) {
                float x = rotation.at(0).toFloat();
                float y = rotation.at(1).toFloat();
                float z = rotation.at(2).toFloat();
                assetPtr->setWorldRotation(Quaternion(x, y, z));
            }
            if (!bone.isEmpty() && m_selectedModelRef) {
                String s(Util::fromQString(name));
                IBone *bone = m_selectedModelRef->findBone(&s);
                assetPtr->setParentBoneRef(bone);
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

bool SceneLoader::loadCameraMotion(const QString &path, IMotionSharedPtr &motionPtr)
{
    /* カメラモーションをファイルから読み込み、場面オブジェクトに設定する */
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data.constData());
        IMotionSharedPtr newMotionPtr(m_factoryRef->newMotion(Factory::findMotionType(ptr, data.size()), 0),
                                      &Scene::deleteMotionUnlessReferred);
        motionPtr.swap(newMotionPtr);
        if (motionPtr->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())
                && motionPtr->countKeyframes(IKeyframe::kCameraKeyframe) > 0) {
            setCameraMotion(motionPtr, QUuid::createUuid(), true);
        }
        else {
            motionPtr.clear();
        }
        return true;
    }
    return false;
}

bool SceneLoader::loadEffectRef(const QString &filename, IEffect *&effectRef)
{
    if (IRenderEngine *engine = m_project->findRenderEngine(m_selectedModelRef.data())) {
        const QDir dir(filename);
        const String d(Util::fromQString(dir.absolutePath()));
        int flags;
        effectRef = createEffectRef(IModelSharedPtr(), filename, flags);
        if (effectRef) {
            IEffect::ScriptOrderType scriptOrder = effectRef->scriptOrderType();
            if (scriptOrder == IEffect::kStandard) {
                engine->setEffect(scriptOrder, effectRef, &d);
                return true;
            }
        }
    }
    return false;
}

bool SceneLoader::loadModel(const QString &filename, IModelSharedPtr &modelPtr)
{
    const QFuture<IModelSharedPtr> future = QtConcurrent::run(this,
                                                              &SceneLoader::loadModelFromFileAsync,
                                                              FilePathPair(filename, QString()),
                                                              kModelLoadable,
                                                              kModelExtensions);
    handleFuture(future, modelPtr);
    return modelPtr;
}

bool SceneLoader::loadModel(const QByteArray &bytes, IModel::Type type, IModelSharedPtr &modelPtr)
{
    /*
     * モデルをファイルから読み込む。レンダリングエンジンに送るには SceneLoader::addModel を呼び出す必要がある
     * (確認ダイアログを出す必要があるので、読み込みとレンダリングエンジンへの追加は別処理)
     */
    const QFuture<IModelSharedPtr> future = QtConcurrent::run(this, &SceneLoader::loadModelFromBytesAsync, bytes, type);
    handleFuture(future, modelPtr);
    return modelPtr;
}

bool SceneLoader::loadModelMotion(const QString &path, IMotionSharedPtr &motionPtr)
{
    /* モーションをファイルから読み込む。モデルへの追加は setModelMotion を使う必要がある */
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data.constData());
        IMotionSharedPtr newMotionPtr(m_factoryRef->newMotion(Factory::findMotionType(ptr, data.size()), 0),
                                      &Scene::deleteMotionUnlessReferred);
        motionPtr.swap(newMotionPtr);
        if (!motionPtr->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            motionPtr.clear();
            return false;
        }
        return true;
    }
    return false;
}

bool SceneLoader::loadModelMotion(const QString &path, QList<IModelSharedPtr> &models, IMotionSharedPtr &motionPtr)
{
    /* モーションをファイルから読み込み、対象の全てのモデルに対してモーションを適用する */
    if (loadModelMotion(path, IModelSharedPtr(), motionPtr)) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            IModelSharedPtr model(m_project->findModel(modelUUIDs[i]), &Scene::deleteModelUnlessReferred);
            setModelMotion(motionPtr, model);
            models.append(model);
        }
    }
    return motionPtr;
}

bool SceneLoader::loadModelMotion(const QString &path, IModelSharedPtr model, IMotionSharedPtr &motionPtr)
{
    /* loadModelMotion に setModelMotion の追加が入ったショートカット的なメソッド */
    if (loadModelMotion(path, motionPtr)) {
        setModelMotion(motionPtr, model);
        return true;
    }
    return false;
}

VPDFilePtr SceneLoader::loadModelPose(const QString &path, IModelSharedPtr model)
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
    newProject();
    if (m_project->load(path.toLocal8Bit().constData())) {
        /* プロジェクト保存時の状態を設定 */
        restoreSceneStatesFromProject(m_project.data());
        /* プロジェクト内のモデル数をプログレスバーに対して発行する */
        int progress = 0;
        QSet<IModel *> lostModels;
        QList<IModelSharedPtr> assets;
        const QString &loadingProgressText = tr("Loading a project... (%1 of %2)");
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        const int nmodels = modelUUIDs.size();
        emit projectDidOpenProgress(tr("Loading progress of %1").arg(QFileInfo(path).baseName()), false);
        emit projectDidUpdateProgress(0, nmodels, loadingProgressText.arg(0).arg(nmodels));
        /* 事前に Scene に入ってるカメラモーションを削除する（カメラモーションの重複を防ぐ） */
        deleteCameraMotion();
        Array<IMotion *> motions;
        m_project->getMotionRefs(motions);
        const int nmotions = motions.count();
        /* Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        Quaternion rotation;
        Scene *sceneObject = sceneRef();
        Scene::AccelerationType accelerationType = globalAccelerationType();
        sceneObject->setAccelerationType(accelerationType);
        for (int i = 0; i < nmodels; i++) {
            const Project::UUID &modelUUIDString = modelUUIDs[i];
            IModelSharedPtr model(m_project->findModel(modelUUIDString), &Scene::deleteModelUnlessReferred);
            const std::string &name = m_project->modelSetting(model.data(), Project::kSettingNameKey);
            const std::string &uri = m_project->modelSetting(model.data(), Project::kSettingURIKey);
            const std::string &inArchive = m_project->modelSetting(model.data(), Project::kSettingArchiveURIKey);
            const QString &filename = QString::fromStdString(uri);
            const QFuture<bool> future = QtConcurrent::run(this,
                                                           &SceneLoader::loadModelFromFileDirectAsync,
                                                           FilePathPair(filename, QString::fromStdString(inArchive)),
                                                           kAllLoadable,
                                                           kAllExtensions,
                                                           model);
            while (!future.isResultReadyAt(0))
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            if (future.result()) {
                const QFileInfo fileInfo(filename);
                m_renderContextRef->addModelPath(model.data(), Util::fromQString(filename));
                if (IRenderEnginePtr enginePtr = createModelEngine(model, fileInfo.absoluteDir())) {
                    /* モデル名がないときはファイル名で補完する */
                    if (!model->name()) {
                        const String s(Util::fromQString(fileInfo.fileName()));
                        model->setName(&s);
                    }
                    const QString &order = QString::fromStdString(m_project->modelSetting(model.data(), Project::kSettingOrderKey));
                    sceneObject->addModel(model.data(), enginePtr.data(), order.toInt());
                    IModel::Type type = model->type();
                    if (type == IModel::kPMDModel || type == IModel::kPMXModel) {
                        /* ModelInfoWidget でエッジ幅の値を設定するので modelDidSelect を呼ぶ前に設定する */
                        const Vector3 &color = UIGetVector3(m_project->modelSetting(model.data(), "edge.color"), kZeroV3);
                        model->setEdgeColor(color);
                        model->setEdgeWidth(QString::fromStdString(m_project->modelSetting(model.data(), "edge.offset")).toFloat());
                        model->setWorldPosition(UIGetVector3(m_project->modelSetting(model.data(), "offset.position"), kZeroV3));
                        const std::string &os = m_project->modelSetting(model.data(), "opacity");
                        model->setOpacity(os.empty() ? 1 : QString::fromStdString(os).toFloat());
                        /* 角度で保存されるので、オイラー角を用いて Quaternion を構築する */
                        const Vector3 &angle = UIGetVector3(m_project->modelSetting(model.data(), "offset.rotation"), kZeroV3);
                        rotation.setEulerZYX(radian(angle.x()), radian(angle.y()), radian(angle.z()));
                        model->setWorldRotation(rotation);
                        emit modelDidAdd(model, QUuid(modelUUIDString.c_str()));
                        emit projectDidUpdateProgress(++progress, nmodels, loadingProgressText.arg(0).arg(nmodels));
                        /* (Bone|Morph)MotionModel#loadMotion で弾かれることを防ぐために先に選択状態にする */
                        if (isModelSelected(model.data()))
                            setSelectedModel(model);
                        emitMotionDidAdd(motions, model);
                        continue;
                    }
                    else if (type == IModel::kAssetModel) {
                        String s(Util::fromQString(fileInfo.completeBaseName().toUtf8()));
                        model->setName(&s);
                        assets.append(model);
                        /* (Bone|Morph)MotionModel#loadMotion で弾かれることを防ぐために先に選択状態にする */
                        if (isAssetSelected(model.data()))
                            setSelectedModel(model);
                        emitMotionDidAdd(motions, model);
                        continue;
                    }
                }
            }
            /* 読み込みに失敗したモデルは後で Project から削除するため失敗したリストに追加する */
            qWarning("Model(uuid=%s, name=%s, path=%s) cannot be loaded",
                     modelUUIDString.c_str(),
                     name.c_str(),
                     qPrintable(filename));
            lostModels.insert(model.data());
            emit projectDidUpdateProgress(++progress, nmodels, loadingProgressText.arg(0).arg(nmodels));
        }
        sceneObject->setAccelerationType(accelerationType);
        /* カメラモーションの読み込み(親モデルがないことが前提。複数存在する場合最後に読み込まれたモーションが適用される) */
        Array<IMotion *> motionsToRetain;
        motionsToRetain.copy(motions);
        bool cameraMotionDidFind = false;
        for (int i = 0; i < nmotions; i++) {
            IMotionSharedPtr motion(motionsToRetain[i], &Scene::deleteMotionUnlessReferred);
            /* カメラモーションは最低でも２つ以上のキーフレームが必要 */
            if (!motion->parentModelRef() && motion->countKeyframes(IKeyframe::kCameraKeyframe) > 1) {
                const QUuid uuid(m_project->motionUUID(motion.data()).c_str());
                setCameraMotion(motion, uuid, false);
                cameraMotionDidFind = true;
            }
        }
        /* カメラモーションがプロジェクト内になければ新しく作り直す（プロジェクト読み込み時にカメラモーションを削除しているため） */
        if (!cameraMotionDidFind) {
            createCameraMotion();
        }
        /* 読み込みに失敗したモデルに従属するモーションを Project から削除する */
        motionsToRetain.clear();
        m_project->getMotionRefs(motionsToRetain);
        const int nmotions2 = motionsToRetain.count();
        for (int i = 0; i < nmotions2; i++) {
            IMotion *motion = motionsToRetain[i];
            if (lostModels.contains(motion->parentModelRef())) {
                m_project->removeMotion(motion);
            }
        }
        /* 読み込みに失敗したモデルとアクセサリを Project から削除する */
        foreach (IModel *model, lostModels) {
            m_renderContextRef->removeModel(model);
            m_project->removeModel(model);
        }
        /* 並列スキニングの設定が有効であれば並列スキニングを有効にする（モデルが全てアップロードされた後に行う） */
        applyParallelSkinning(isParallelSkinningEnabled());
        /* ボーン追従の関係で assetDidAdd/assetDidSelect は全てのモデルを読み込んだ後にアクセサリ読み込みを行う */
        foreach (IModelSharedPtr model, assets) {
            const QUuid assetUUID(m_project->modelUUID(model.data()).c_str());
            model->setWorldPosition(assetPosition(model.data()));
            model->setWorldRotation(assetRotation(model.data()));
            model->setScaleFactor(assetScaleFactor(model.data()));
            model->setOpacity(assetOpacity(model.data()));
            model->setParentModelRef(assetParentModel(model.data()).data());
            model->setParentBoneRef(assetParentBone(model.data()));
            emit modelDidAdd(model, assetUUID);
            emit projectDidUpdateProgress(++progress, nmodels, loadingProgressText.arg(0).arg(nmodels));
        }
        updateDepthBuffer(shadowMapSize());
        sort();
        m_project->setDirty(false);
        /* エフェクトが有効であればエフェクトボタンを有効にする */
        emit effectDidEnable(isEffectEnabled());
        /* 読み込み完了 */
        emit projectDidLoad(true);
    }
    else {
        qDebug("Failed loading project %s", qPrintable(path));
        newProject();
        emit projectDidLoad(false);
    }
}

void SceneLoader::createCameraMotion()
{
    IMotionSharedPtr cameraMotion;
    newCameraMotion(cameraMotion);
    setCameraMotion(cameraMotion, QUuid::createUuid(), true);
}

void SceneLoader::newCameraMotion(IMotionSharedPtr &motionPtr) const
{
    /* 0番目に空のキーフレームが入ったカメラのモーションを作成する */
    IMotionSharedPtr newCameraMotionPtr(m_factoryRef->newMotion(IMotion::kVMDMotion, 0), &Scene::deleteMotionUnlessReferred);
    motionPtr.swap(newCameraMotionPtr);
    QScopedPointer<ICameraKeyframe> cameraKeyframe(m_factoryRef->createCameraKeyframe(motionPtr.data()));
    QScopedPointer<ILightKeyframe> lightKeyframe(m_factoryRef->createLightKeyframe(motionPtr.data()));
    ICamera *camera = m_project->camera();
    ILight *light = m_project->light();
    cameraKeyframe->setDefaultInterpolationParameter();
    cameraKeyframe->setLookAt(camera->lookAt());
    cameraKeyframe->setAngle(camera->angle());
    cameraKeyframe->setFov(camera->fov());
    cameraKeyframe->setDistance(camera->distance());
    lightKeyframe->setColor(light->color());
    lightKeyframe->setDirection(light->direction());
    motionPtr->addKeyframe(cameraKeyframe.take());
    motionPtr->addKeyframe(lightKeyframe.take());
    motionPtr->update(IKeyframe::kCameraKeyframe);
    motionPtr->update(IKeyframe::kLightKeyframe);
}

void SceneLoader::newModelMotion(IModelSharedPtr model, IMotionSharedPtr &motionPtr) const
{
    /* 全ての可視ボーンと頂点モーフに対して0番目に空のキーフレームが入ったモデルのモーションを作成する */
    if (model) {
        IMotionSharedPtr newMotionPtr(m_factoryRef->newMotion(IMotion::kVMDMotion, 0),
                                      &Scene::deleteMotionUnlessReferred);
        motionPtr.swap(newMotionPtr);
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
                boneKeyframe->setLocalPosition(bone->localPosition());
                boneKeyframe->setLocalRotation(bone->localRotation());
                motionPtr->addKeyframe(boneKeyframe.take());
            }
        }
        motionPtr->update(IKeyframe::kBoneKeyframe);
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
        motionPtr->update(IKeyframe::kMorphKeyframe);
    }
}

void SceneLoader::releaseProject()
{
    m_selectedAssetRef.clear();
    m_selectedModelRef.clear();
    const Project::UUIDList &motionUUIDs = m_project->motionUUIDs();
    for (Project::UUIDList::const_iterator it = motionUUIDs.begin(); it != motionUUIDs.end(); it++) {
        const Project::UUID &motionUUID = *it;
        IMotionSharedPtr motion(m_project->findMotion(motionUUID), &Scene::deleteMotionUnlessReferred);
        if (motion)
            emit motionWillDelete(motion, QUuid(motionUUID.c_str()));
    }
    const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
    for (Project::UUIDList::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        const Project::UUID &modelUUID = *it;
        IModelSharedPtr model(m_project->findModel(modelUUID), &Scene::deleteModelUnlessReferred);
        if (model)
            emit modelWillDelete(model, QUuid(modelUUID.c_str()));
    }
    deleteCameraMotion();
    m_project.reset();
}

void SceneLoader::renderWindow()
{
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    m_project->getRenderEnginesByRenderOrder(enginesForPreProcess,
                                             enginesForStandard,
                                             enginesForPostProcess,
                                             nextPostEffects);
    const int nEnginesForPostProcess = enginesForPostProcess.count();
    /* ポストプロセスの前処理 */
    for (int i = nEnginesForPostProcess - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        /*
         * レンダリングエンジンの状態が自動更新されないことが原因でポストエフェクトで正しく処理されない問題があるため、
         * アクセサリの場合のみポストエフェクト処理前に事前にレンダリングエンジンの状態の更新を行う
         * (具体例は VIEWPORTPIXELSIZE が (0,0) になってしまい、それに依存するポストエフェクトが正しく描画されない問題)
         */
        if (engine->parentModelRef()->type() == IModel::kAssetModel)
            engine->update();
        engine->preparePostProcess();
    }
    /* 画面サイズを更新してレンダーターゲットをウィンドウに戻す */
    Vector3 size;
    m_renderContextRef->getViewport(size);
    glViewport(0, 0, size.x(), size.y());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* プリプロセス */
    const int nEnginesForPreProcess = enginesForPreProcess.count();
    for (int i = 0; i < nEnginesForPreProcess; i++) {
        IRenderEngine *engine = enginesForPreProcess[i];
        /* 上のレンダリングエンジンの状態が自動更新されないことが原因で云々と同じ理由のため省略 */
        if (engine->parentModelRef()->type() == IModel::kAssetModel)
            engine->update();
        engine->performPreProcess();
    }
    emit preprocessDidPerform();
    /* 通常の描写 */
    const int nEnginesForStandard = enginesForStandard.count();
    for (int i = 0; i < nEnginesForStandard; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        IModel *model = engine->parentModelRef();
        if (isProjectiveShadowEnabled(model) && !isSelfShadowEnabled(model)) {
            engine->renderShadow();
        }
        engine->renderModel();
        engine->renderEdge();
    }
    /* ポストプロセス */
    for (int i = 0; i < nEnginesForPostProcess; i++) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *const *effect = nextPostEffects[engine];
        engine->performPostProcess(*effect);
    }
}

void SceneLoader::setMousePosition(const QMouseEvent *event, const QRect &geometry)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    const QSizeF &size = geometry.size() / 2;
    const qreal w = size.width(), h = size.height();
    const glm::vec2 &value = glm::vec2((pos.x() - w) / w, (pos.y() - h) / -h);
    Qt::MouseButtons buttons = event->buttons();
    m_renderContextRef->setMousePosition(value, buttons & Qt::LeftButton, IRenderContext::kMouseLeftPressPosition);
    m_renderContextRef->setMousePosition(value, buttons & Qt::MiddleButton, IRenderContext::kMouseMiddlePressPosition);
    m_renderContextRef->setMousePosition(value, buttons & Qt::RightButton, IRenderContext::kMouseRightPressPosition);
    m_renderContextRef->setMousePosition(value, false, IRenderContext::kMouseCursorPosition);
}

void SceneLoader::renderOffscreen()
{
    m_renderContextRef->renderOffscreen();
}

void SceneLoader::updateDepthBuffer(const QSize &value)
{
    int width = 1024, height = 1024;
    if (!value.isEmpty()) {
        width = value.width();
        height = value.height();
    }
    m_renderContextRef->createShadowMap(Vector3(width, height, 0));
}

const QList<QUuid> SceneLoader::renderOrderList() const
{
    Array<IModel *> models;
    QList<QUuid> modelUUIDs;
    m_project->getModelRefs(models);
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        const Project::UUID &uuid = m_project->modelUUID(model);
        modelUUIDs.append(QUuid(uuid.c_str()));
    }
    return modelUUIDs;
}

void SceneLoader::saveMetadataFromAsset(const QString &path, IModelSharedPtr asset)
{
    /* 現在のアセットの位置情報からファイルに書き出す。行毎の意味は loadMetadataFromAsset を参照 */
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        const char lineSeparator[] = "\r\n";
        stream << Util::toQStringFromModel(asset.data()) << lineSeparator;
        stream << m_name2assets.key(asset.data()) << lineSeparator;
        stream << asset->scaleFactor() << lineSeparator;
        const Vector3 &position = asset->worldPosition();
        stream << QString("%1,%2,%3").arg(position.x(), 0, 'f', 1)
                  .arg(position.y(), 0, 'f', 1).arg(position.z(), 0, 'f', 1) << lineSeparator;
        const Quaternion &rotation = asset->worldRotation();
        stream << QString("%1,%2,%3").arg(rotation.x(), 0, 'f', 1)
                  .arg(rotation.y(), 0, 'f', 1).arg(rotation.z(), 0, 'f', 1) << lineSeparator;
        const IBone *bone = asset->parentBoneRef();
        stream << (bone ? Util::toQStringFromBone(bone) : "地面") << lineSeparator;
        stream << 1 << lineSeparator;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(path), qPrintable(file.errorString()));
    }
}

void SceneLoader::saveProject(const QString &path)
{
    QTemporaryFile file(path + ".vpvm_project_XXXXXXXXXXXX");
    if (file.open()) {
        const QByteArray &bytes = file.fileName().toLocal8Bit();
        commitAssetProperties();
        const ICamera *camera = m_project->camera();
        m_project->setGlobalSetting("state.camera.angle", UIVector3String(camera->angle()));
        m_project->setGlobalSetting("state.camera.distance", UIFloatString(camera->distance()));
        m_project->setGlobalSetting("state.camera.fov", UIFloatString(camera->fov()));
        m_project->setGlobalSetting("state.camera.lookat", UIVector3String(camera->lookAt()));
        const ILight *light = m_project->light();
        m_project->setGlobalSetting("state.light.color", UIVector3String(light->color()));
        m_project->setGlobalSetting("state.light.direction", UIVector3String(light->direction()));
        m_project->save(bytes.constData());
        // TODO: windows
        int ret = rename(bytes.constData(), path.toLocal8Bit().constData());
        emit projectDidSave(ret == 0);
    }
    else {
        emit projectDidSave(false);
    }
}

void SceneLoader::setCameraMotion(IMotionSharedPtr motion, const QUuid &uuid, bool addToScene)
{
    deleteCameraMotion();
    m_camera = motion;
    m_project->camera()->setMotion(motion.data());
    if (addToScene) {
        m_project->addMotion(motion.data(), uuid.toString().toStdString());
    }
    if (motion) {
        motion->update(IKeyframe::kCameraKeyframe);
    }
    emit cameraMotionDidSet(motion, uuid);
}

void SceneLoader::setLightColor(const Vector3 &color)
{
    if (m_project) {
        m_project->light()->setColor(color);
        m_project->setGlobalSetting("state.light.color", UIVector3String(color));
    }
    emit lightColorDidSet(color);
}

void SceneLoader::setLightDirection(const Vector3 &position)
{
    if (m_project) {
        m_project->light()->setDirection(position);
        m_project->setGlobalSetting("state.light.direction", UIVector3String(position));
    }
    emit lightDirectionDidSet(position);
}

void SceneLoader::setModelMotion(IMotionSharedPtr motion, IModelSharedPtr model)
{
    if (model) {
        const QUuid &uuid = QUuid::createUuid();
#ifdef IS_VPVM
        /* 物理削除を伴うので、まず配列のコピーを用意してそこにコピーしてから削除。そうしないと SEGV が起きる */
        Array<IMotion *> motions;
        m_project->getMotionRefs(motions);
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *m = motions[i];
            if (m->parentModelRef() == model) {
                m_project->removeMotion(m);
            }
        }
#endif
        motion->setParentModelRef(model.data());
        m_project->addMotion(motion.data(), uuid.toString().toStdString());
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
            m_project->setModelSetting(model, Project::kSettingOrderKey, n);
        }
        i++;
    }
    sort();
}

void SceneLoader::sort()
{
    if (m_project) {
        m_project->sort();
    }
}

void SceneLoader::startPhysicsSimulation()
{
    bool value = isPhysicsEnabled();
    Array<IModel *> models;
    m_project->getModelRefs(models);
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        model->setPhysicsEnable(value);
    }
    if (value) {
        m_project->setWorldRef(m_world->dynamicWorldRef());
        m_project->update(Scene::kResetMotionState);
        m_world->stepSimulation(m_world->fixedTimeStep());
    }
}

void SceneLoader::stopPhysicsSimulation()
{
    Array<IModel *> models;
    m_project->getModelRefs(models);
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        model->setPhysicsEnable(false);
    }
    m_project->setWorldRef(0);
}

const Vector3 SceneLoader::worldGravity() const
{
    static const Vector3 defaultGravity(0.0f, -9.8f, 0.0f);
    const Vector3 &gravity = m_project ? UIGetVector3(m_project->globalSetting("physics.gravity"), defaultGravity) : defaultGravity;
    return gravity;
}

int SceneLoader::worldMaxSubSteps() const
{
    if (m_project) {
        return QVariant(QString::fromStdString(m_project->globalSetting("physics.substeps"))).toInt();
    }
    else {
        return m_world->maxSubSteps();
    }
}

bool SceneLoader::worldFloorEnabled() const
{
    if (m_project) {
        return QVariant(QString::fromStdString(m_project->globalSetting("physics.floor"))).toBool();
    }
    return true;
}

unsigned long SceneLoader::worldRandSeed() const
{
    if (m_project) {
        return QVariant(QString::fromStdString(m_project->globalSetting("physics.randseed"))).toULongLong();
    }
    else {
        return 0;
    }
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
        m_world->setGravity(value);
        m_project->setGlobalSetting("physics.gravity", UIVector3String(value));
    }
}

void SceneLoader::setWorldMaxSubSteps(int value)
{
    if (m_project) {
        m_world->setMaxSubSteps(value);
        m_project->setGlobalSetting("physics.substeps", QVariant(value).toString().toStdString());
    }
}

void SceneLoader::setWorldFloorEnable(bool value)
{
    if (m_project) {
        m_floor->isInWorld();
        if (value && !m_floor->isInWorld()) {
            m_world->addRigidBody(m_floor.data());
        }
        else if (!value && m_floor->isInWorld()) {
            m_world->removeRigidBody(m_floor.data());
        }
        m_project->setGlobalSetting("physics.floor", QVariant(value).toString().toStdString());
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
    if (m_project && isProjectiveShadowEnabled(model) != value) {
        m_project->setModelSetting(model, "shadow.projective", value ? "true" : "false");
    }
}

bool SceneLoader::isSelfShadowEnabled(const IModel *model) const
{
    bool enabled = m_project ? m_project->modelSetting(model, "shadow.ss") == "true" : false;
    return enabled;
}

void SceneLoader::setSelfShadowEnable(const IModel *model, bool value)
{
    if (m_project && isSelfShadowEnabled(model) != value) {
        m_project->setModelSetting(model, "shadow.ss", value ? "true" : "false");
    }
}

IModelSharedPtr SceneLoader::selectedModelRef() const
{
    return m_selectedModelRef;
}

bool SceneLoader::isModelSelected(const IModel *value) const
{
    bool selected = m_project ? m_project->modelSetting(value, "selected") == "true" : false;
    return selected;
}

void SceneLoader::setSelectedModel(IModelSharedPtr value)
{
    if (m_project && value != m_selectedModelRef) {
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        Project::UUIDList::const_iterator it = modelUUIDs.begin(), end = modelUUIDs.end();
        while (it != end) {
            IModel *model = m_project->findModel(*it);
            IModel::Type type = model->type();
            if (type == IModel::kPMDModel || type == IModel::kPMXModel)
                m_project->setModelSetting(model, "selected", "false");
            ++it;
        }
        m_selectedModelRef = value;
        m_project->setModelSetting(value.data(), "selected", "true");
        emit modelDidSelect(value);
    }
}

void SceneLoader::setModelEdgeOffset(IModelSharedPtr model, float value)
{
    if (m_project && model) {
        model->setEdgeWidth(value);
        m_project->setModelSetting(model.data(), "edge.offset", UIFloatString(value));
    }
}

void SceneLoader::setModelOpacity(IModelSharedPtr model, const Scalar &value)
{
    if (m_project && model) {
        model->setOpacity(value);
        m_project->setModelSetting(model.data(), "opacity", UIFloatString(value));
    }
}

void SceneLoader::setModelPosition(IModelSharedPtr model, const Vector3 &value)
{
    if (m_project && model) {
        model->setWorldPosition(value);
        m_project->setModelSetting(model.data(), "offset.position", UIVector3String(value));
    }
}

const Vector3 SceneLoader::modelRotation(IModelSharedPtr value) const
{
    const Vector3 &rotation = m_project ? UIGetVector3(m_project->modelSetting(value.data(), "offset.rotation"), kZeroV3) : kZeroV3;
    return rotation;
}

void SceneLoader::setModelRotation(IModelSharedPtr model, const Vector3 &value)
{
    if (m_project && model) {
        m_project->setModelSetting(model.data(), "offset.rotation", UIVector3String(value));
        Quaternion rotation;
        rotation.setEulerZYX(radian(value.x()), radian(value.y()), radian(value.z()));
        model->setWorldRotation(rotation);
    }
}

void SceneLoader::setModelEdgeColor(IModelSharedPtr model, const QColor &value)
{
    if (m_project) {
        float red = value.redF(), green = value.greenF(), blue = value.blueF();
        model->setEdgeColor(Color(red, green, blue, 1.0));
        m_project->setModelSetting(model.data(), "edge.color", UIColorString(value));
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
int SceneLoader::timeIndexPlayFrom() const
{
    int value = globalSetting("play.frame_index.from", 0);
    return value;
}

void SceneLoader::setTimeIndexPlayFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::timeIndexPlayTo() const
{
    int value = globalSetting("play.frame_index.to", int(m_project->maxTimeIndex()));
    return value;
}

void SceneLoader::setTimeIndexPlayTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.to", QVariant(value).toString().toStdString());
}

Scalar SceneLoader::sceneFPSForPlay() const
{
    Scalar value = globalSetting("play.fps", 30.0f);
    return value;
}

void SceneLoader::setSceneFPSForPlay(const Scalar &value)
{
    if (m_project) {
        m_project->setGlobalSetting("play.fps", QVariant(value).toString().toStdString());
    }
}

int SceneLoader::timeIndexEncodeVideoFrom() const
{
    int value = globalSetting("video.frame_index.from", 0);
    return value;
}

void SceneLoader::setTimeIndexEncodeVideoFrom(int value)
{

    if (m_project && QString::fromStdString(m_project->globalSetting("video.frame_index.from")).toInt() != value) {
        m_project->setGlobalSetting("video.frame_index.from", QVariant(value).toString().toStdString());
        emit timeIndexEncodeVideoFromDidChange(value);
    }
}

int SceneLoader::timeIndexEncodeVideoTo() const
{
    int value = globalSetting("video.frame_index.to", int(m_project->maxTimeIndex()));
    return value;
}

void SceneLoader::setTimeIndexEncodeVideoTo(int value)
{
    if (m_project && QString::fromStdString(m_project->globalSetting("video.frame_index.to")).toInt() != value) {
        m_project->setGlobalSetting("video.frame_index.to", QVariant(value).toString().toStdString());
        emit timeIndexEncodeVideoToDidChange(value);
    }
}

Scalar SceneLoader::sceneFPSForEncodeVideo() const
{
    Scalar value = globalSetting("video.fps", 60.0f);
    return value;
}

void SceneLoader::setSceneFPSForEncodeVideo(const Scalar &value)
{
    if (m_project && QString::fromStdString(m_project->globalSetting("video.fps")).toFloat() != value) {
        m_project->setGlobalSetting("video.fps", QVariant(value).toString().toStdString());
        emit sceneFPSForEncodeVideoDidChange(value);
    }
}

int SceneLoader::sceneWidth() const
{
    int value = globalSetting("video.width", 0);
    return value;
}

void SceneLoader::setSceneWidth(int value)
{
    if (m_project && QString::fromStdString(m_project->globalSetting("video.width")).toInt() != value) {
        m_project->setGlobalSetting("video.width", QVariant(value).toString().toStdString());
        emit sceneWidthDidChange(value);
    }
}

int SceneLoader::sceneHeight() const
{
    int value = globalSetting("video.height", 0);
    return value;
}

void SceneLoader::setSceneHeight(int value)
{
    if (m_project && QString::fromStdString(m_project->globalSetting("video.height")).toInt() != value) {
        m_project->setGlobalSetting("video.height", QVariant(value).toString().toStdString());
        emit sceneHeightDidChange(value);
    }
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
    if (m_project && (QString::fromStdString(m_project->globalSetting("grid.video")) == "true") != value) {
        m_project->setGlobalSetting("grid.video", value ? "true" : "false");
        emit gridIncludedDidChange(value);
    }
}

const QString SceneLoader::backgroundAudio() const
{
    const QString &path = m_project ? QString::fromStdString(m_project->globalSetting("audio.path")) : "";
    return path;
}

void SceneLoader::setBackgroundAudioPath(const QString &value)
{
    const std::string &v = value.toStdString();
    if (m_project && m_project->globalSetting("audio.path") != v) {
        m_project->setGlobalSetting("audio.path", v);
        emit backgroundAudioPathDidChange(value);
    }
}

const Vector3 SceneLoader::assetPosition(const IModel *asset)
{
    const Vector3 &position = m_project ? UIGetVector3(m_project->modelSetting(asset, "position"), kZeroV3) : kZeroV3;
    return position;
}

void SceneLoader::setAssetPosition(const IModel *asset, const Vector3 &value)
{
    if (m_project) {
        m_project->setModelSetting(asset, "position", UIVector3String(value));
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
        m_project->setModelSetting(asset, "rotation",UIQuaterionString(value));
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
        m_project->setModelSetting(asset, "opacity", UIFloatString(value));
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
        m_project->setModelSetting(asset, "scale", UIFloatString(value));
    }
}

IModelSharedPtr SceneLoader::assetParentModel(const IModel *asset) const
{
    if (m_project) {
        IModel *parentModel = m_project->findModel(m_project->modelSetting(asset, "parent.model"));
        return IModelSharedPtr(parentModel, &Scene::deleteModelUnlessReferred);
    }
    else {
        return IModelSharedPtr();
    }
}

void SceneLoader::setAssetParentModel(const IModel *asset, IModelSharedPtr model)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.model", m_project->modelUUID(model.data()));
}

IBone *SceneLoader::assetParentBone(const IModel *asset) const
{
    IModel *model = 0;
    if (m_project && (model = assetParentModel(asset).data())) {
        const QString &name = QString::fromStdString(m_project->modelSetting(asset, "parent.bone"));
        String s(Util::fromQString(name));
        return model->findBone(&s);
    }
    return 0;
}

void SceneLoader::setAssetParentBone(const IModel *asset, IBone *bone)
{
    if (m_project)
        m_project->setModelSetting(asset, "parent.bone", Util::toQStringFromBone(bone).toStdString());
}

IModelSharedPtr SceneLoader::selectedAsset() const
{
    return m_selectedAssetRef;
}

bool SceneLoader::isAssetSelected(const IModel *value) const
{
    bool selected = m_project ? m_project->modelSetting(value, "selected") == "true" : false;
    return selected;
}

void SceneLoader::setSelectedAsset(IModelSharedPtr value)
{
    if (m_project && value != m_selectedAssetRef) {
        commitAssetProperties();
        const Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        Project::UUIDList::const_iterator it = modelUUIDs.begin(), end = modelUUIDs.end();
        while (it != end) {
            IModel *model = m_project->findModel(*it);
            if (model->type() == IModel::kAssetModel)
                m_project->setModelSetting(model, "selected", "false");
            ++it;
        }
        m_selectedAssetRef = value;
        m_project->setModelSetting(value.data(), "selected", "true");
        emit modelDidSelect(value);
    }
}

void SceneLoader::setPreferredFPS(const Scalar &value)
{
    m_project->setPreferredFPS(value);
}

void SceneLoader::setScreenColor(const QColor &value)
{
    if (m_project) {
        m_project->setGlobalSetting("screen.color", UIColorString(value));
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

const Vector3 SceneLoader::shadowCenter() const
{
    const Vector3 &value = m_project ? UIGetVector3(m_project->globalSetting("shadow.center"), kZeroV3) : kZeroV3;
    return value;
}

void SceneLoader::setShadowCenter(const Vector3 &value)
{
    if (m_project) {
        m_project->setGlobalSetting("shadow.center", UIVector3String(value));
        if (IShadowMap *shadowMap = m_project->shadowMapRef()) {
            shadowMap->setPosition(value);
        }
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

bool SceneLoader::isParallelSkinningEnabled() const
{
    bool enabled = m_project ? m_project->globalSetting("skinning.parallel") == "true" : false;
    return enabled;
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
        if (value) {
            m_project->setAccelerationType(Scene::kOpenCLAccelerationType1);
        }
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

const Scalar SceneLoader::shadowDistance() const
{
    float value = m_project ? UIGetFloat(m_project->globalSetting("shadow.distance"), 7.5f) : 7.5f;
    return value;
}

void SceneLoader::setVertexShaderSkinningType1Enable(bool value)
{
    if (m_project && isVertexShaderSkinningType1Enabled() != value) {
        m_project->setGlobalSetting("skinning.vs.type1", value ? "true" : "false");
        if (value)
            m_project->setAccelerationType(Scene::kVertexShaderAccelerationType1);
        applyParallelSkinning(false);
    }
}

void SceneLoader::setParallelSkinningEnable(bool value)
{
    if (m_project && isParallelSkinningEnabled() != value) {
        m_project->setGlobalSetting("skinning.parallel", value ? "true" : "false");
        applyParallelSkinning(value);
    }
}

void SceneLoader::setSoftwareSkinningEnable(bool value)
{
    if (m_project && value) {
        m_project->setAccelerationType(Scene::kSoftwareFallback);
        applyParallelSkinning(false);
    }
}

void SceneLoader::setEffectEnable(bool value)
{
    if (m_project && isEffectEnabled() != value) {
        m_project->setGlobalSetting("effect.enabled", value ? "true" : "false");
        emit effectDidEnable(value);
    }
}

void SceneLoader::setShadowDistance(const Scalar &value)
{
    if (m_project && shadowDistance() != value) {
        m_project->setGlobalSetting("shadow.distance", QVariant(value).toString().toStdString());
        if (IShadowMap *shadowMap = m_project->shadowMapRef()) {
            shadowMap->setDistance(value);
        }
    }
}

void SceneLoader::updatePhysicsSimulation(const Scalar &timeStep)
{
    if (isPhysicsEnabled()) {
        m_world->stepSimulation(timeStep);
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

float SceneLoader::globalSetting(const char *key, float def) const
{
    if (m_project) {
        bool ok = false;
        float value = QString::fromStdString(m_project->globalSetting(key)).toFloat(&ok);
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

Scene *SceneLoader::sceneRef() const
{
    return m_project.data();
}

qt::RenderContext *SceneLoader::renderContextRef() const
{
    return m_renderContextRef;
}

qt::World *SceneLoader::worldRef() const
{
    return m_world.data();
}

void SceneLoader::emitMotionDidAdd(const Array<IMotion *> &motions, IModelSharedPtr model)
{
    const int nmotions = motions.count();
    /* モデルに属するモーションを取得し、追加する */
    for (int i = 0; i < nmotions; i++) {
        IMotionSharedPtr motion(motions[i], &Scene::deleteMotionUnlessReferred);
        if (motion->parentModelRef() == model) {
            /*
             * プロジェクト読み込み時点では仕様上インスタンス化のみで実際にモデルを読み込んだ状態ではないので、
             * ここでモーションに親のモデルを設定してモーションを再度読み込みし直す必要がある
             */
            motion->setParentModelRef(model.data());
            const Project::UUID &motionUUIDString = m_project->motionUUID(motion.data());
            const QUuid motionUUID(motionUUIDString.c_str());
            emit motionDidAdd(motion, model, motionUUID);
        }
    }
}

} /*  namespace vpvm */

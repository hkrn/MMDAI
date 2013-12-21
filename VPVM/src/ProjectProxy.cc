/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Pose.h>
#include <vpvl2/extensions/World.h>
#include <vpvl2/extensions/XMLProject.h>
#include <vpvl2/extensions/qt/Encoding.h>
#include <vpvl2/extensions/qt/String.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QUndoGroup>
#include <QUndoStack>

#include "BaseKeyframeRefObject.h"
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "LabelRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphRefObject.h"
#include "MorphMotionTrack.h"
#include "MotionProxy.h"
#include "ProjectProxy.h"
#include "Util.h"
#include "WorldProxy.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::qt;

namespace {

struct ProjectDelegate : public XMLProject::IDelegate {
    ProjectDelegate(ProjectProxy *proxy)
        : m_projectRef(proxy)
    {
    }
    ~ProjectDelegate() {
        m_projectRef = 0;
    }
    const std::string toStdFromString(const IString *value) const {
        return Util::toQString(value).toStdString();
    }
    const IString *toStringFromStd(const std::string &value) const {
        return String::create(value);
    }
    bool loadModel(const XMLProject::UUID &uuid, const StringMap &settings, IModel::Type /* type */, IModel *&model, IRenderEngine *&engine, int &priority) {
        const std::string &uri = settings.value(XMLProject::kSettingURIKey, std::string());
        if (ModelProxy *modelProxy = m_projectRef->loadModel(QUrl::fromLocalFile(QString::fromStdString(uri)), QUuid(QString::fromStdString(uuid)), true)) {
            m_projectRef->internalAddModel(modelProxy, settings.value("selected", std::string("false")) == "true", true);
            priority = XMLProject::toIntFromString(settings.value(XMLProject::kSettingOrderKey, std::string("0")));;
            /* upload render engine later */
            model = modelProxy->data();
        }
        engine = 0;
        return model;
    }
    ProjectProxy *m_projectRef;
};

class LoadingModelTask : public QRunnable {
public:
    LoadingModelTask(const Factory *factoryRef, const QUrl &fileUrl)
        : m_factoryRef(factoryRef),
          m_fileUrl(fileUrl),
          m_result(false),
          m_running(true)
    {
        setAutoDelete(false);
    }

    inline IModel *takeModel() { return m_model.take(); }
    inline QString errorString() const { return m_errorString; }
    inline bool isRunning() const { return m_running; }

private:
    void run() {
        QFile file(m_fileUrl.toLocalFile());
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            const uint8_t *ptr = reinterpret_cast<const uint8_t *>(bytes.constData());
            m_model.reset(m_factoryRef->createModel(ptr, file.size(), m_result));
            if (m_result) {
                /* set filename of the model if the name of the model is null such as asset */
                if (!m_model->name(IEncoding::kDefaultLanguage)) {
                    const qt::String s(QFileInfo(file.fileName()).fileName());
                    m_model->setName(&s, IEncoding::kDefaultLanguage);
                }
            }
            else {
                m_errorString = QApplication::tr("Cannot load model %1: %2").arg(m_fileUrl.toDisplayString()).arg(m_model->error());
                m_model.reset();
            }
        }
        else {
            m_errorString = QApplication::tr("Cannot open model %1: %2").arg(m_fileUrl.toDisplayString()).arg(file.errorString());
        }
        m_running = false;
    }

    const Factory *m_factoryRef;
    const QUrl m_fileUrl;
    QScopedPointer<IModel> m_model;
    QString m_errorString;
    bool m_result;
    volatile bool m_running;
};

class LoadingMotionTask : public QRunnable {
public:
    LoadingMotionTask(const ModelProxy *modelProxy, const Factory *factoryRef, const QUrl &fileUrl)
        : m_modelProxy(modelProxy),
          m_factoryRef(factoryRef),
          m_fileUrl(fileUrl),
          m_result(false),
          m_running(true)
    {
        setAutoDelete(false);
    }

    inline IMotion *takeMotion() { return m_motion.take(); }
    inline QString errorString() const { return m_errorString; }
    inline bool isRunning() const { return m_running; }

//private:
    void run() {
        QFile file(m_fileUrl.toLocalFile());
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            const uint8_t *ptr = reinterpret_cast<const uint8_t *>(bytes.constData());
            IModel *modelRef = m_modelProxy ? m_modelProxy->data() : 0;
            m_motion.reset(m_factoryRef->createMotion(ptr, file.size(), modelRef, m_result));
            if (!m_result) {
                m_errorString = QApplication::tr("Cannot load motion %1").arg(m_fileUrl.toDisplayString());
                m_motion.reset();
            }
        }
        else {
            m_errorString = QApplication::tr("Cannot open motion %1: %2").arg(m_fileUrl.toDisplayString()).arg(file.errorString());
        }
        m_running = false;
    }

    const ModelProxy *m_modelProxy;
    const Factory *m_factoryRef;
    const QUrl m_fileUrl;
    QScopedPointer<IMotion> m_motion;
    QString m_errorString;
    bool m_result;
    volatile bool m_running;
};

static void convertStringFromVariant(const QVariant &value, std::string &result)
{
    switch (value.type()) {
    case QVariant::Vector3D: {
        const QVector3D &v = value.value<QVector3D>();
        result.assign(XMLProject::toStringFromVector3(Vector3(v.x(), v.y(), v.z())));
        break;
    }
    case QVariant::Vector4D: {
        const QVector4D &v = value.value<QVector4D>();
        result.assign(XMLProject::toStringFromVector4(Vector4(v.x(), v.y(), v.z(), v.w())));
        break;
    }
    case QVariant::Quaternion: {
        const QQuaternion &v = value.value<QQuaternion>();
        result.assign(XMLProject::toStringFromQuaternion(Quaternion(v.x(), v.y(), v.z(), v.scalar())));
        break;
    }
    default:
        result.assign(value.toString().toStdString());
        break;
    }
}

}

ProjectProxy::ProjectProxy(QObject *parent)
    : QObject(parent),
      m_encoding(new Encoding(&m_dictionary)),
      m_factory(new Factory(m_encoding.data())),
      m_delegate(new ProjectDelegate(this)),
      m_project(new XMLProject(m_delegate.data(), m_factory.data(), false)),
      m_cameraRefObject(new CameraRefObject(this)),
      m_lightRefObject(new LightRefObject(this)),
      m_worldProxy(new WorldProxy(this)),
      m_undoGroup(new QUndoGroup()),
      m_title(tr("Untitled Project")),
      m_screenColor(Qt::white),
      m_currentModelRef(0),
      m_currentMotionRef(0),
      m_currentTimeIndex(0),
      m_accelerationType(ParallelAcceleration),
      m_language(DefaultLauguage),
      m_initialized(false)
{
    QMap<QString, IEncoding::ConstantType> str2const;
    str2const.insert("arm", IEncoding::kArm);
    str2const.insert("asterisk", IEncoding::kAsterisk);
    str2const.insert("center", IEncoding::kCenter);
    str2const.insert("elbow", IEncoding::kElbow);
    str2const.insert("finger", IEncoding::kFinger);
    str2const.insert("left", IEncoding::kLeft);
    str2const.insert("leftknee", IEncoding::kLeftKnee);
    str2const.insert("opacity", IEncoding::kOpacityMorphAsset);
    str2const.insert("right", IEncoding::kRight);
    str2const.insert("rightknee", IEncoding::kRightKnee);
    str2const.insert("root", IEncoding::kRootBone);
    str2const.insert("scale", IEncoding::kScaleBoneAsset);
    str2const.insert("spaextension", IEncoding::kSPAExtension);
    str2const.insert("sphextension", IEncoding::kSPHExtension);
    str2const.insert("wrist", IEncoding::kWrist);
    QMapIterator<QString, IEncoding::ConstantType> it(str2const);
    QSettings settings(":data/words.dic", QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    while (it.hasNext()) {
        it.next();
        const QVariant &value = settings.value("constants." + it.key());
        m_dictionary.insert(it.value(), String::create(value.toString().toStdString()));
    }
    connect(this, &ProjectProxy::currentModelChanged, this, &ProjectProxy::updateParentBindingModel);
    connect(this, &ProjectProxy::parentBindingDidUpdate, this, &ProjectProxy::availableParentBindingBonesChanged);
    connect(this, &ProjectProxy::parentBindingDidUpdate, this, &ProjectProxy::availableParentBindingModelsChanged);
    connect(this, &ProjectProxy::projectDidLoad, this, &ProjectProxy::screenColorChanged);
    connect(this, &ProjectProxy::projectDidLoad, this, &ProjectProxy::languageChanged);
    connect(this, &ProjectProxy::projectDidLoad, this, &ProjectProxy::loopChanged);
    connect(this, &ProjectProxy::projectDidCreate, this, &ProjectProxy::durationTimeIndexChanged);
    connect(this, &ProjectProxy::projectDidLoad, this, &ProjectProxy::durationTimeIndexChanged);
    connect(this, &ProjectProxy::undoDidPerform, this, &ProjectProxy::durationTimeIndexChanged);
    connect(this, &ProjectProxy::redoDidPerform, this, &ProjectProxy::durationTimeIndexChanged);
    connect(this, &ProjectProxy::modelDidAdd, this, &ProjectProxy::availableModelsChanged);
    connect(this, &ProjectProxy::modelDidRemove, this, &ProjectProxy::availableModelsChanged);
    connect(this, &ProjectProxy::motionDidLoad, this, &ProjectProxy::availableMotionsChanged);
    connect(this, &ProjectProxy::motionWillDelete, this, &ProjectProxy::availableMotionsChanged);
    connect(m_undoGroup.data(), &QUndoGroup::canUndoChanged, this, &ProjectProxy::canUndoChanged);
    connect(m_undoGroup.data(), &QUndoGroup::canRedoChanged, this, &ProjectProxy::canRedoChanged);
}

ProjectProxy::~ProjectProxy()
{
    /* explicitly release XMLProject (Scene) instance to invalidation of Effect correctly before destorying RenderContext */
    release(true);
    /* explicitly release World instance to ensure release btRigidBody */
    m_worldProxy.reset();
    m_dictionary.releaseAll();
}

void ProjectProxy::initializeOnce()
{
    if (!m_initialized) {
        assignCamera();
        assignLight();
        setDirty(false);
        m_initialized = true;
    }
}

void ProjectProxy::createAsync()
{
    emit projectWillCreate();
    /* delete (release) all remain models/motions before creating project */
    connect(this, &ProjectProxy::enqueuedModelsDidDelete, this, &ProjectProxy::internalCreateAsync);
    release(false);
}

void ProjectProxy::loadAsync(const QUrl &fileUrl)
{
    Q_ASSERT(fileUrl.isValid());
    emit projectWillLoad();
    /* delete (release) all remain models/motions before loading project */
    connect(this, &ProjectProxy::enqueuedModelsDidDelete, this, &ProjectProxy::internalLoadAsync);
    release(false);
    m_fileUrl = fileUrl;
}

bool ProjectProxy::save(const QUrl &fileUrl)
{
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return false;
    }
    emit projectWillSave();
    bool saved = false, committed = false;
    if (m_currentModelRef) {
        setModelSetting(m_currentModelRef, "selected", true);
    }
    QTemporaryFile temp;
    if (temp.open()) {
        saved = m_project->save(temp.fileName().toUtf8().constData());
        QSaveFile saveFile(fileUrl.toLocalFile());
        if (saveFile.open(QFile::WriteOnly | QFile::Unbuffered)) {
            saveFile.write(temp.readAll());
            committed = saveFile.commit();
        }
    }
    setDirty(false);
    emit projectDidSave();
    return saved && committed;
}

ModelProxy *ProjectProxy::findModel(const QUuid &uuid)
{
    return m_uuid2ModelProxyRefs.value(uuid);
}

MotionProxy *ProjectProxy::findMotion(const QUuid &uuid)
{
    return m_uuid2MotionProxyRefs.value(uuid);
}

bool ProjectProxy::loadModel(const QUrl &fileUrl)
{
    Q_ASSERT(fileUrl.isValid());
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = loadModel(fileUrl, uuid, false);
    return modelProxy;
}

void ProjectProxy::addModel(ModelProxy *value)
{
    internalAddModel(value, true, false);
    emit modelDidCommitUploading();
}

void ProjectProxy::deleteModel(ModelProxy *value)
{
    internalDeleteModel(value);
    emit modelDidCommitDeleting();
}

bool ProjectProxy::loadMotion(const QUrl &fileUrl, ModelProxy *modelProxy, MotionType type)
{
    Q_ASSERT(fileUrl.isValid());
    QScopedPointer<LoadingMotionTask> task(new LoadingMotionTask(modelProxy, m_factory.data(), fileUrl));
    QThreadPool::globalInstance()->start(task.data());
    while (task->isRunning()) {
        qApp->processEvents(QEventLoop::AllEvents);
    }
    m_errorString.clear();
    if (IMotion *motion = task->takeMotion()) {
        const QUuid &uuid = QUuid::createUuid();
        MotionProxy *motionProxy = createMotionProxy(motion, uuid, fileUrl, true);
        if (modelProxy && type == ModelMotion) {
            VPVL2_VLOG(1, "The mode motion of " << modelProxy->name().toStdString() << " from " << fileUrl.toString().toStdString() << " will be allocated as " << uuid.toString().toStdString());
            deleteMotion(modelProxy->childMotion(), false);
            motionProxy->setModelProxy(modelProxy, m_factory.data());
            modelProxy->setChildMotion(motionProxy, true);
        }
        else if (type == CameraMotion) {
            VPVL2_VLOG(1, "The camera motion from " << fileUrl.toString().toStdString() << " will be allocated as " << uuid.toString().toStdString());
            m_cameraRefObject->assignCameraRef(m_project->cameraRef(), motionProxy);
        }
        else if (type == LightMotion) {
            VPVL2_VLOG(1, "The light motion from " << fileUrl.toString().toStdString() << " will be allocated as " << uuid.toString().toStdString());
            m_lightRefObject->assignLightRef(m_project->lightRef(), motionProxy);
        }
        else {
            VPVL2_VLOG(1, "The unknown motion from " << fileUrl.toString().toStdString() << " will be allocated as " << uuid.toString().toStdString());
        }
        emit motionDidLoad(motionProxy);
    }
    else {
        setErrorString(task->errorString());
    }
    return modelProxy;
}

bool ProjectProxy::loadPose(const QUrl &fileUrl, ModelProxy *modelProxy)
{
    bool result = false;
    if (modelProxy) {
        QFile file(fileUrl.toLocalFile());
        if (file.open(QFile::ReadOnly)) {
            Pose pose(m_encoding.data());
            QByteArray bytes = file.readAll();
            std::istringstream stream(bytes.constData());
            if (pose.load(stream)) {
                IModel *modelRef = modelProxy->data();
                pose.bind(modelRef);
                emit poseDidLoad(modelProxy);
                result = true;
            }
            else {
                setErrorString(tr("Cannot load pose %1").arg(fileUrl.toDisplayString()));
            }
        }
        else {
            setErrorString(tr("Cannot open pose %1: %2").arg(fileUrl.toDisplayString()).arg(file.errorString()));
        }
    }
    else {
        setErrorString(tr("Current model is not set. You should select the model to load a pose."));
    }
    return result;
}

void ProjectProxy::seek(qreal timeIndex)
{
    internalSeek(timeIndex, false);
}

void ProjectProxy::rewind()
{
    foreach (ModelProxy *model, m_modelProxies) {
        foreach (BoneRefObject *bone, model->allBoneRefs()) {
            bone->setLocalOrientation(QQuaternion());
            bone->setLocalTranslation(QVector3D());
            resetIKEffectorBones(bone);
        }
    }
    internalSeek(0, true);
    m_worldProxy->rewind();
    emit rewindDidPerform();
}

void ProjectProxy::refresh()
{
    foreach (MotionProxy *motionProxy, m_motionProxies) {
        motionProxy->applyParentModel();
    }
    m_cameraRefObject->refresh();
}

void ProjectProxy::ray(qreal x, qreal y, int width, int height)
{
    // This implementation based on the below page.
    // http://softwareprodigy.blogspot.com/2009/08/gluunproject-for-iphone-opengl-es.html
    const glm::vec2 win(x, height - y);
    const glm::vec4 viewport(0, 0, width, height);
    const ICamera *camera = m_project->cameraRef();
    const Transform &transform = camera->modelViewTransform();
    float m[16];
    transform.getOpenGLMatrix(m);
    const glm::mat4 &worldView = glm::make_mat4(m);
    const glm::mat4 &projection = glm::perspective(camera->fov(), width / glm::mediump_float(height), 0.1f, 100000.0f);
    const glm::vec3 &cnear = glm::unProject(glm::vec3(win, 0), worldView, projection, viewport);
    const glm::vec3 &cfar = glm::unProject(glm::vec3(win, 1), worldView, projection, viewport);
    const Vector3 from(cnear.x, cnear.y, cnear.z), to(cfar.x, cfar.y, cfar.z);
    if (BoneRefObject *bone = m_worldProxy->ray(from, to)) {
        m_currentModelRef->selectBone(bone);
    }
}

void ProjectProxy::undo()
{
    if (m_undoGroup->canUndo()) {
        m_undoGroup->undo();
        /* force seeking to get latest motion value after undo and render it */
        internalSeek(m_currentTimeIndex, true);
        emit undoDidPerform();
    }
}

void ProjectProxy::redo()
{
    if (m_undoGroup->canRedo()) {
        m_undoGroup->redo();
        /* force seeking to get latest motion value after redo and render it */
        internalSeek(m_currentTimeIndex, true);
        emit redoDidPerform();
    }
}

void ProjectProxy::internalAddModel(ModelProxy *value, bool selected, bool isProject)
{
    Q_ASSERT(value);
    m_modelProxies.append(value);
    m_instance2ModelProxyRefs.insert(value->data(), value);
    m_uuid2ModelProxyRefs.insert(value->uuid(), value);
    if (selected) {
        setCurrentModel(value);
    }
    setDirty(true);
    connect(this, &ProjectProxy::modelBoneDidPick, value, &ModelProxy::selectBone);
    emit modelDidAdd(value, isProject);
    const QUuid &uuid = QUuid::createUuid();
    VPVL2_VLOG(1, "The initial motion of the model " << value->name().toStdString() << " will be allocated as " << uuid.toString().toStdString());
    if (MotionProxy *motionProxy = createMotionProxy(m_factory->newMotion(IMotion::kVMDMotion, value->data()), uuid, QUrl(), false)) {
        motionProxy->setModelProxy(value, m_factory.data());
        value->setChildMotion(motionProxy, true);
        emit motionDidLoad(motionProxy);
    }
}

void ProjectProxy::internalDeleteModel(ModelProxy *value)
{
    if (value && m_instance2ModelProxyRefs.contains(value->data())) {
        emit modelWillRemove(value);
        value->releaseBindings();
        if (m_currentModelRef == value) {
            value->resetTargets();
            setCurrentModel(0);
        }
        deleteMotion(value->childMotion(), false);
        m_worldProxy->leaveWorld(value);
        setDirty(true);
        m_modelProxies.removeOne(value);
        m_instance2ModelProxyRefs.remove(value->data());
        m_uuid2ModelProxyRefs.remove(value->uuid());
        emit modelDidRemove(value);
    }
}

void ProjectProxy::update(int flags)
{
    m_project->update(flags);
}

QList<ModelProxy *> ProjectProxy::modelProxies() const
{
    return m_modelProxies;
}

QList<MotionProxy *> ProjectProxy::motionProxies() const
{
    return m_motionProxies;
}

QQmlListProperty<ModelProxy> ProjectProxy::availableModels()
{
    return QQmlListProperty<ModelProxy>(this, m_modelProxies);
}

QQmlListProperty<MotionProxy> ProjectProxy::availableMotions()
{
    return QQmlListProperty<MotionProxy>(this, m_motionProxies);
}

QQmlListProperty<QObject> ProjectProxy::availableParentBindingModels()
{
    return QQmlListProperty<QObject>(this, m_parentModelProxyRefs);
}

QQmlListProperty<QObject> ProjectProxy::availableParentBindingBones()
{
    return QQmlListProperty<QObject>(this, m_parentModelBoneRefs);
}

QString ProjectProxy::title() const
{
    return m_title;
}

void ProjectProxy::setTitle(const QString &value)
{
    if (value != m_title) {
        setGlobalString("title", value);
        m_title = value;
        emit titleChanged();
    }
}

ModelProxy *ProjectProxy::currentModel() const
{
    return m_currentModelRef;
}

void ProjectProxy::setCurrentModel(ModelProxy *value)
{
    if (value != m_currentModelRef) {
        m_worldProxy->joinWorld(value);
        m_currentModelRef = value;
        emit currentModelChanged();
    }
}

MotionProxy *ProjectProxy::currentMotion() const
{
    return m_currentMotionRef;
}

void ProjectProxy::setCurrentMotion(MotionProxy *value)
{
    if (value != m_currentMotionRef) {
        QUndoStack *stack = m_motion2UndoStacks.value(value);
        m_undoGroup->setActiveStack(stack);
        m_currentMotionRef = value;
        emit currentMotionChanged();
    }
}

QUrl ProjectProxy::audioSource() const
{
    return globalSetting("audio.path").toUrl();
}

void ProjectProxy::setAudioSource(const QUrl &value)
{
    if (value != audioSource()) {
        setGlobalString("audio.path", value);
        emit audioSourceChanged();
    }
}

QColor ProjectProxy::screenColor() const
{
    return m_screenColor;
}

void ProjectProxy::setScreenColor(const QColor &value)
{
    if (value != screenColor()) {
        const Vector4 v(value.redF(), value.greenF(), value.blueF(), value.alphaF());
        m_project->setGlobalSetting("screen.color", XMLProject::toStringFromVector4(v));
        m_screenColor = value;
        emit screenColorChanged();
    }
}

ProjectProxy::LanguageType ProjectProxy::language() const
{
    return m_language;
}

void ProjectProxy::setLanguage(LanguageType value)
{
    if (value != language()) {
        m_language = value;
        emit languageChanged();
    }
}

ProjectProxy::AccelerationType ProjectProxy::accelerationType() const
{
    return m_accelerationType;
}

void ProjectProxy::setAccelerationType(AccelerationType value)
{
    if (value != accelerationType()) {
        Array<IRenderEngine *> engines;
        m_project->getRenderEngineRefs(engines);
        const int nengines = engines.count();
        switch (value) {
        case ParallelAcceleration:
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->setUpdateOptions(IRenderEngine::kParallelUpdate);
            }
            break;
        case OpenCLCPUAcceleration:
            m_project->setAccelerationType(Scene::kOpenCLAccelerationType2);
            break;
        case OpenCLGPUAcceleration:
            m_project->setAccelerationType(Scene::kOpenCLAccelerationType1);
            break;
        case VertexShaderSkinning:
            m_project->setAccelerationType(Scene::kVertexShaderAccelerationType1);
            break;
        case NoAcceleration:
        default:
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->setUpdateOptions(IRenderEngine::kNone);
            }
            break;
        }
        m_accelerationType = value;
        emit accelerationTypeChanged();
    }
}

bool ProjectProxy::isDirty() const
{
    return m_project->isDirty();
}

void ProjectProxy::setDirty(bool value)
{
    if (isDirty() != value) {
        m_project->setDirty(value);
        emit dirtyChanged();
    }
}

bool ProjectProxy::isLoop() const
{
    return globalSetting("play.loop", false).toBool();
}

void ProjectProxy::setLoop(bool value)
{
    if (value != isLoop()) {
        setGlobalString("play.loop", value);
        emit loopChanged();
    }
}

bool ProjectProxy::canUndo() const
{
    return m_undoGroup->canUndo();
}

bool ProjectProxy::canRedo() const
{
    return m_undoGroup->canRedo();
}

qreal ProjectProxy::differenceTimeIndex(qreal value) const
{
    static const qreal kZero = 0.0f;
    return m_project ? qMax(qreal(m_project->duration()) - qMax(value, kZero), kZero) : 0;
}

qreal ProjectProxy::differenceDuration(qreal value) const
{
    return millisecondsFromTimeIndex(differenceTimeIndex(value));
}

qreal ProjectProxy::secondsFromTimeIndex(qreal value) const
{
    return value / Scene::defaultFPS();
}

qreal ProjectProxy::millisecondsFromTimeIndex(qreal value) const
{
    return value * Scene::defaultFPS();
}

void ProjectProxy::resetBone(BoneRefObject *bone, ResetBoneType type)
{
    if (bone) {
        ModelProxy *modelProxy = bone->parentLabel()->parentModel();
        if (MotionProxy *motionProxy = modelProxy->childMotion()) {
            const QVector3D &translationToReset = bone->originLocalTranslation();
            const QQuaternion &orientationToReset = bone->originLocalOrientation();
            QVector3D translation = bone->localTranslation();
            QQuaternion orientation = bone->localOrientation();
            switch (type) {
            case TranslationAxisX:
                translation.setX(translationToReset.x());
                break;
            case TranslationAxisY:
                translation.setY(translationToReset.y());
                break;
            case TranslationAxisZ:
                translation.setZ(translationToReset.z());
                break;
            case TranslationAxisXYZ:
                translation = translationToReset;
                break;
            case Orientation:
                orientation = orientationToReset;
                break;
            case AllTranslationAndOrientation:
                translation = translationToReset;
                orientation = orientationToReset;
                break;
            default:
                break;
            }
            bone->setLocalTranslation(translation);
            bone->setLocalOrientation(orientation);
            resetIKEffectorBones(bone);
            motionProxy->updateKeyframe(bone, static_cast<qint64>(m_currentTimeIndex));
            VPVL2_VLOG(2, "reset TYPE=BONE name=" << bone->name().toStdString() << " type=" << type);
        }
    }
    else if (m_currentModelRef) {
        if (MotionProxy *motionProxy = m_currentModelRef->childMotion()) {
            QScopedPointer<QUndoCommand> command(new QUndoCommand());
            foreach (BoneRefObject *boneRef, m_currentModelRef->allBoneRefs()) {
                boneRef->setLocalTranslation(boneRef->originLocalTranslation());
                boneRef->setLocalOrientation(boneRef->originLocalOrientation());
                resetIKEffectorBones(boneRef);
                motionProxy->updateKeyframe(boneRef, static_cast<qint64>(m_currentTimeIndex), command.data());
            }
            if (QUndoStack *stack = m_undoGroup->activeStack()) {
                stack->push(command.take());
                VPVL2_VLOG(2, "resetAll TYPE=BONE");
            }
        }
    }
}

void ProjectProxy::resetMorph(MorphRefObject *morph)
{
    if (morph) {
        ModelProxy *modelProxy = morph->parentLabel()->parentModel();
        if (MotionProxy *motionProxy = modelProxy->childMotion()) {
            morph->setWeight(morph->originWeight());
            motionProxy->updateKeyframe(morph, static_cast<qint64>(m_currentTimeIndex));
            VPVL2_VLOG(2, "reset TYPE=MORPH name=" << morph->name().toStdString());
        }
    }
    else if (m_currentModelRef) {
        if (MotionProxy *motionProxy = m_currentModelRef->childMotion()) {
            QScopedPointer<QUndoCommand> command(new QUndoCommand());
            foreach (MorphRefObject *morphRef, m_currentModelRef->allMorphRefs()) {
                morphRef->setWeight(morphRef->originWeight());
                motionProxy->updateKeyframe(morphRef, static_cast<qint64>(m_currentTimeIndex), command.data());
            }
            if (QUndoStack *stack = m_undoGroup->activeStack()) {
                stack->push(command.take());
                VPVL2_VLOG(2, "resetAll TYPE=MORPH");
            }
        }
    }
}

void ProjectProxy::updateParentBindingModel()
{
    m_parentModelProxyRefs.clear();
    foreach (ModelProxy *modelProxy, m_modelProxies) {
        if (modelProxy != m_currentModelRef) {
            m_parentModelProxyRefs.append(modelProxy);
        }
    }
    m_parentModelBoneRefs.clear();
    if (m_currentModelRef) {
        if (const ModelProxy *parentModel = m_currentModelRef->parentBindingModel()) {
            foreach (BoneRefObject *boneRef, parentModel->allBoneRefs()) {
                m_parentModelBoneRefs.append(boneRef);
            }
        }
    }
    emit parentBindingDidUpdate();
}

ModelProxy *ProjectProxy::loadModel(const QUrl &fileUrl, const QUuid &uuid, bool skipConfirm)
{
    Q_ASSERT(fileUrl.isValid() && !uuid.isNull());
    ModelProxy *modelProxy = 0;
    QScopedPointer<LoadingModelTask> task(new LoadingModelTask(m_factory.data(), fileUrl));
    QThreadPool::globalInstance()->start(task.data());
    while (task->isRunning()) {
        qApp->processEvents(QEventLoop::AllEvents);
    }
    if (IModel *model = task->takeModel()) {
        modelProxy = createModelProxy(model, uuid, fileUrl, skipConfirm);
    }
    else {
        setErrorString(task->errorString());
    }
    return modelProxy;
}

bool ProjectProxy::loadEffect(const QUrl &fileUrl)
{
    ModelProxy *modelProxy = new ModelProxy(this, m_factory->newModel(IModel::kPMXModel), QUuid::createUuid(), fileUrl, QUrl());
    m_modelProxies.append(modelProxy);
    m_instance2ModelProxyRefs.insert(modelProxy->data(), modelProxy);
    m_uuid2ModelProxyRefs.insert(modelProxy->uuid(), modelProxy);
    emit effectDidAdd(modelProxy);
    emit effectDidCommitUploading();
    return true;
}

ModelProxy *ProjectProxy::createModelProxy(IModel *model, const QUuid &uuid, const QUrl &fileUrl, bool skipConfirm)
{
    QUrl faviconUrl;
    const QFileInfo finfo(fileUrl.toLocalFile());
    QStringList filters; filters << "favicon.*";
    const QStringList &faviconLocations = finfo.absoluteDir().entryList(filters);
    if (!faviconLocations.isEmpty()) {
        faviconUrl = QUrl::fromLocalFile(finfo.absoluteDir().filePath(faviconLocations.first()));
    }
    model->setPhysicsEnable(m_worldProxy->simulationType() != WorldProxy::DisableSimulation);
    ModelProxy *modelProxy = new ModelProxy(this, model, uuid, fileUrl, faviconUrl);
    emit modelWillLoad(modelProxy);
    modelProxy->initialize();
    emit modelDidLoad(modelProxy, skipConfirm);
    return modelProxy;
}

MotionProxy *ProjectProxy::createMotionProxy(IMotion *motion, const QUuid &uuid, const QUrl &fileUrl, bool emitSignal)
{
    MotionProxy *motionProxy = 0;
    if (motion && !resolveMotionProxy(motion)) {
        QUndoStack *undoStack = new QUndoStack(m_undoGroup.data());
        motionProxy = new MotionProxy(this, motion, uuid, fileUrl, undoStack);
        if (emitSignal) {
            emit motionWillLoad(motionProxy);
        }
        m_undoGroup->addStack(undoStack);
        m_motionProxies.append(motionProxy);
        m_motion2UndoStacks.insert(motionProxy, undoStack);
        m_instance2MotionProxyRefs.insert(motion, motionProxy);
        m_uuid2MotionProxyRefs.insert(uuid, motionProxy);
        m_project->addMotion(motion, uuid.toString().toStdString());
        connect(motionProxy, &MotionProxy::keyframeDidAdd, this, &ProjectProxy::durationTimeIndexChanged);
        connect(motionProxy, &MotionProxy::keyframeDidRemove, this, &ProjectProxy::durationTimeIndexChanged);
    }
    return motionProxy;
}

ModelProxy *ProjectProxy::resolveModelProxy(const IModel *value) const
{
    return m_instance2ModelProxyRefs.value(value);
}

MotionProxy *ProjectProxy::resolveMotionProxy(const IMotion *value) const
{
    return m_instance2MotionProxyRefs.value(value);
}

void ProjectProxy::internalDeleteAllMotions(bool fromDestructor)
{
    m_cameraRefObject->releaseMotion();
    m_lightRefObject->releaseMotion();
    /* copy motion proxies because m_motionProxies will be mutated using removeOne */
    QList<MotionProxy *> motionProxies = m_motionProxies;
    foreach (MotionProxy *motionProxy, motionProxies) {
        deleteMotion(motionProxy, fromDestructor);
    }
}

void ProjectProxy::deleteMotion(MotionProxy *value, bool fromDestructor)
{
    if (value && m_uuid2MotionProxyRefs.contains(value->uuid())) {
        emit motionWillDelete(value);
        if (m_currentMotionRef == value) {
            setCurrentMotion(0);
        }
        if (ModelProxy *modelProxy = value->parentModel()) {
            /* no signal should be emitted when fromDestructor is true */
            modelProxy->setChildMotion(0, !fromDestructor);
            value->data()->setParentModelRef(0);
        }
        m_undoGroup->removeStack(m_motion2UndoStacks.value(value));
        m_motion2UndoStacks.remove(value);
        m_motionProxies.removeOne(value);
        m_instance2MotionProxyRefs.remove(value->data());
        m_uuid2MotionProxyRefs.remove(value->uuid());
        m_project->removeMotion(value->data());
        delete value;
    }
}

QVariant ProjectProxy::globalSetting(const QString &key, const QVariant &defaultValue) const
{
    Q_ASSERT(!key.isEmpty());
    /* XXX: cannot convert Vector3/Vector4/Quaternion correctly */
    const std::string &value = m_project->globalSetting(key.toStdString());
    return value.empty() ? defaultValue : QVariant(QString::fromStdString(value));
}

QVector3D ProjectProxy::globalSetting(const QString &key, const QVector3D &defaultValue) const
{
    Q_ASSERT(!key.isEmpty());
    const std::string &value = m_project->globalSetting(key.toStdString());
    return value.empty() ? defaultValue : Util::fromVector3(XMLProject::toVector3FromString(value));
}

void ProjectProxy::setGlobalString(const QString &key, const QVariant &value)
{
    Q_ASSERT(!key.isEmpty());
    std::string result;
    convertStringFromVariant(value, result);
    m_project->setGlobalSetting(key.toStdString(), result);
}


QVariant ProjectProxy::modelSetting(const ModelProxy *modelProxy, const QString &key, const QVariant &defaultValue) const
{
    Q_ASSERT(!key.isEmpty());
    /* XXX: cannot convert Vector3/Vector4/Quaternion correctly */
    const std::string &value = m_project->modelSetting(modelProxy->data(), key.toStdString());
    return value.empty() ? defaultValue : QVariant(QString::fromStdString(value));
}

void ProjectProxy::setModelSetting(const ModelProxy *modelProxy, const QString &key, const QVariant &value) const
{
    Q_ASSERT(modelProxy && !key.isEmpty());
    std::string result;
    convertStringFromVariant(value, result);
    m_project->setModelSetting(modelProxy->data(), key.toStdString(), result);
}

qreal ProjectProxy::currentTimeIndex() const
{
    return m_currentTimeIndex;
}

qreal ProjectProxy::durationTimeIndex() const
{
    return differenceTimeIndex(0);
}

qreal ProjectProxy::durationMilliseconds() const
{
    return differenceDuration(0);
}

QString ProjectProxy::errorString() const
{
    return m_errorString;
}

CameraRefObject *ProjectProxy::camera() const
{
    return m_cameraRefObject.data();
}

LightRefObject *ProjectProxy::light() const
{
    return m_lightRefObject.data();
}

WorldProxy *ProjectProxy::world() const
{
    return m_worldProxy.data();
}

IEncoding *ProjectProxy::encodingInstanceRef() const
{
    return m_encoding.data();
}

Factory *ProjectProxy::factoryInstanceRef() const
{
    return m_factory.data();
}

XMLProject *ProjectProxy::projectInstanceRef() const
{
    return m_project.data();
}

void ProjectProxy::createProjectInstance()
{
    m_project->clear();
    m_errorString = QString();
    setTitle(tr("Untitled Project"));
    setAudioSource(QUrl());
    setAccelerationType(ParallelAcceleration);
    setLanguage(DefaultLauguage);
    setLoop(false);
    setScreenColor(Qt::white);
    m_worldProxy->resetProjectInstance(this);
}

void ProjectProxy::internalCreateAsync()
{
    disconnect(this, &ProjectProxy::enqueuedModelsDidDelete, this, &ProjectProxy::internalCreateAsync);
    createProjectInstance();
    assignCamera();
    assignLight();
    setDirty(false);
    emit projectDidCreate();
}

void ProjectProxy::internalLoadAsync()
{
    disconnect(this, &ProjectProxy::enqueuedModelsDidDelete, this, &ProjectProxy::internalLoadAsync);
    createProjectInstance();
    m_project->load(m_fileUrl.toLocalFile().toUtf8().constData());
    Array<IMotion *> motionRefs;
    m_project->getMotionRefs(motionRefs);
    const int nmotions = motionRefs.count();
    assignCamera();
    assignLight();
    /* Override model motions to the project holds */
    for (int i = 0; i < nmotions; i++) {
        IMotion *motionRef = motionRefs[i];
        const XMLProject::UUID &uuid = m_project->motionUUID(motionRef);
        MotionProxy *motionProxy = resolveMotionProxy(motionRef);
        if (motionProxy && motionProxy->parentModel()) {
            /* remove previous (initial) model motion */
            deleteMotion(motionProxy, false);
        }
        else {
            motionProxy = createMotionProxy(motionRef, QUuid(QString::fromStdString(uuid)), QUrl(), false);
            if (ModelProxy *modelProxy = resolveModelProxy(motionRef->parentModelRef())) {
                /* this is a model motion */
                motionProxy->setModelProxy(modelProxy, m_factory.data());
                modelProxy->setChildMotion(motionProxy, true);
                if (modelSetting(modelProxy, "selected").toBool()) {
                    /* call setCurrentMotion to paint timeline correctly */
                    setCurrentMotion(motionProxy);
                }
            }
            else if (motionProxy->data()->countKeyframes(IKeyframe::kCameraKeyframe) > 1) {
                /* this is a camera motion and delete previous camera motion */
                deleteMotion(m_cameraRefObject->releaseMotion(), false);
                m_cameraRefObject->assignCameraRef(m_cameraRefObject->data(), motionProxy);
            }
            else if (motionProxy->data()->countKeyframes(IKeyframe::kLightKeyframe) > 1) {
                /* this is a light motion and delete previous light motion */
                deleteMotion(m_lightRefObject->releaseMotion(), false);
                m_lightRefObject->assignLightRef(m_lightRefObject->data(), motionProxy);
            }
        }
    }
    const QString &title = globalSetting("title").toString();
    setTitle(title.isEmpty() ? QFileInfo(m_fileUrl.toLocalFile()).fileName() : title);
    const QString &screenColorValue = globalSetting("screen.color").toString();
    if (!screenColorValue.isEmpty()) {
        const Vector4 &v = XMLProject::toVector4FromString(screenColorValue.toStdString());
        setScreenColor(QColor::fromRgbF(v.x(), v.y(), v.z(), v.w()));
    }
    updateParentBindingModel();
    setDirty(false);
    emit projectDidLoad();
}

void ProjectProxy::resetIKEffectorBones(BoneRefObject *bone)
{
    const IBone *boneRef = bone->data();
    if (boneRef->hasInverseKinematics()) {
        Array<IBone *> effectorBones;
        boneRef->getEffectorBones(effectorBones);
        const int numEffectorBones = effectorBones.count();
        for (int i = 0; i < numEffectorBones; i++) {
            IBone *effectorBone = effectorBones[i];
            effectorBone->setLocalTranslation(kZeroV3);
            effectorBone->setLocalOrientation(Quaternion::getIdentity());
        }
    }
}

void ProjectProxy::assignCamera()
{
    const QUuid &uuid = QUuid::createUuid();
    QScopedPointer<IMotion> motion(m_factory->newMotion(IMotion::kVMDMotion, 0));
    VPVL2_VLOG(1, "The camera motion will be allocated as " << uuid.toString().toStdString());
    deleteMotion(m_cameraRefObject->releaseMotion(), false);
    MotionProxy *motionProxy = createMotionProxy(motion.data(), uuid, QUrl(), false);
    ICamera *cameraRef = m_project->cameraRef();
    m_cameraRefObject->assignCameraRef(cameraRef, motionProxy);
    QScopedPointer<ICameraKeyframe> keyframe(m_factory->createCameraKeyframe(motion.take()));
    keyframe->setDefaultInterpolationParameter();
    keyframe->setAngle(cameraRef->angle());
    keyframe->setDistance(cameraRef->distance());
    keyframe->setFov(cameraRef->fov());
    keyframe->setLookAt(cameraRef->lookAt());
    CameraMotionTrack *track = m_cameraRefObject->track();
    track->addKeyframe(track->convertCameraKeyframe(keyframe.take()), true);
}

void ProjectProxy::assignLight()
{
    const QUuid &uuid = QUuid::createUuid();
    QScopedPointer<IMotion> motion(m_factory->newMotion(IMotion::kVMDMotion, 0));
    VPVL2_VLOG(1, "The light motion will be allocated as " << uuid.toString().toStdString());
    deleteMotion(m_lightRefObject->releaseMotion(), false);
    MotionProxy *motionProxy = createMotionProxy(motion.data(), uuid, QUrl(), false);
    ILight *lightRef = m_project->lightRef();
    m_lightRefObject->assignLightRef(lightRef, motionProxy);
    QScopedPointer<ILightKeyframe> keyframe(m_factory->createLightKeyframe(motion.take()));
    keyframe->setColor(lightRef->color());
    keyframe->setDirection(lightRef->direction());
    LightMotionTrack *track = m_lightRefObject->track();
    track->addKeyframe(track->convertLightKeyframe(keyframe.take()), true);
}

void ProjectProxy::internalSeek(const qreal &timeIndex, bool forceUpdate)
{
    if (forceUpdate || !qFuzzyCompare(timeIndex, m_currentTimeIndex)) {
        MotionProxy *cameraMotionProxy = 0;
        if (!m_cameraRefObject->isSeekable()) {
            /* save camera motion not to apply camera motion */
            cameraMotionProxy = m_cameraRefObject->motion();
            m_cameraRefObject->data()->setMotion(0);
        }
        m_project->seek(timeIndex, Scene::kUpdateAll);
        if (cameraMotionProxy) {
            /* restore camera motion */
            m_cameraRefObject->data()->setMotion(cameraMotionProxy->data());
        }
        m_worldProxy->stepSimulation(timeIndex);
        if (m_currentModelRef) {
            updateOriginValues();
            foreach (BoneRefObject *bone, m_currentModelRef->allTargetBones()) {
                bone->sync();
            }
            if (MorphRefObject *morph = m_currentModelRef->firstTargetMorph()) {
                morph->sync();
            }
        }
        m_cameraRefObject->refresh();
        m_lightRefObject->refresh();
        m_currentTimeIndex = timeIndex;
        emit currentTimeIndexChanged();
    }
}

void ProjectProxy::updateOriginValues()
{
    Q_ASSERT(m_currentModelRef);
    if (MotionProxy *motionProxy = m_currentModelRef->childMotion()) {
        foreach (BoneRefObject *bone, m_currentModelRef->allBoneRefs()) {
            if (const BoneMotionTrack *track = motionProxy->findBoneMotionTrack(bone)) {
                if (BaseKeyframeRefObject *keyframe = track->findKeyframeByTimeIndex(static_cast<qint64>(m_currentTimeIndex))) {
                    BoneKeyframeRefObject *boneKeyframeRef = qobject_cast<BoneKeyframeRefObject *>(keyframe);
                    Q_ASSERT(boneKeyframeRef);
                    bone->setOriginLocalTranslation(boneKeyframeRef->localTranslation());
                    bone->setOriginLocalOrientation(boneKeyframeRef->localOrientation());
                }
            }
        }
        foreach (MorphRefObject *morph, m_currentModelRef->allMorphRefs()) {
            if (const MorphMotionTrack *track = motionProxy->findMorphMotionTrack(morph)) {
                if (BaseKeyframeRefObject *keyframe = track->findKeyframeByTimeIndex(static_cast<qint64>(m_currentTimeIndex))) {
                    MorphKeyframeRefObject *morphKeyframeRef = qobject_cast<MorphKeyframeRefObject *>(keyframe);
                    Q_ASSERT(morphKeyframeRef);
                    morph->setOriginWeight(morphKeyframeRef->weight());
                }
            }
        }
    }
}

void ProjectProxy::setErrorString(const QString &value)
{
    if (m_errorString != value) {
        VPVL2_LOG(WARNING, value.toStdString());
        m_errorString = value;
        emit errorStringChanged();
    }
}

void ProjectProxy::reset()
{
    m_fileUrl = QUrl();
    m_currentTimeIndex = 0;
    if (m_currentModelRef) {
        m_currentModelRef->resetTargets();
    }
    setCurrentModel(0);
    setCurrentMotion(0);
}

void ProjectProxy::release(bool fromDestructor)
{
    VPVL2_VLOG(1, "The project will be released");
    reset();
    m_undoGroup.reset(new QUndoGroup());
    internalDeleteAllMotions(fromDestructor);
    /* copy motion proxies because m_modelProxies will be mutated using removeOne */
    QList<ModelProxy *> modelProxies = m_modelProxies;
    foreach (ModelProxy *modelProxy, modelProxies) {
        internalDeleteModel(modelProxy);
    }
    m_project->setWorldRef(0);
    connect(m_undoGroup.data(), &QUndoGroup::canUndoChanged, this, &ProjectProxy::canUndoChanged);
    connect(m_undoGroup.data(), &QUndoGroup::canRedoChanged, this, &ProjectProxy::canRedoChanged);
    emit projectDidRelease();
}

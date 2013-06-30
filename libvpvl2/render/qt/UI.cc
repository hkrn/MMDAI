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

#include "UI.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/AudioSource.h>
#include <vpvl2/extensions/World.h>
#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include <vpvl2/qt/ApplicationContext.h>
#include <vpvl2/qt/CustomGLContext.h>
#include <vpvl2/qt/DebugDrawer.h>
#include <vpvl2/qt/TextureDrawHelper.h>
#include <vpvl2/qt/Util.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QApplication>
#include <QDesktopWidget>
#pragma clang diagnostic pop
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrent>
#endif

#if defined(VPVL2_LINK_ASSIMP3) || defined(VPVL2_LINK_ASSIMP)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#if defined(VPVL2_LINK_ASSIMP3)
#include <assimp/Importer.hpp>
#elif defined(VPVL2_LINK_ASSIMP)
#include <assimp/assimp.hpp>
#include <assimp/DefaultLogger.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#endif
#pragma clang diagnostic pop
#endif

#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <glm/gtc/matrix_transform.hpp>

/* internal headers */
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/vmd/Motion.h"

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

using namespace vpvl2;
using namespace vpvl2::qt;

QDebug operator<<(QDebug debug, const Vector3 &v)
{
    debug.nospace() << "(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Color &v)
{
    debug.nospace() << "(r=" << v.x() << ", g=" << v.y() << ", b=" << v.z() << ", a=" << v.w() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const IString *str)
{
    if (str) {
        debug.nospace() << Util::toQString(static_cast<const String *>(str)->value());
    }
    else {
        debug.nospace() << "(null)";
    }
    return debug;
}

QDebug operator<<(QDebug debug, const pmx::Bone *bone)
{
    if (!bone) {
        debug.nospace() << "Bone is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Bone id                          = " << bone->index();
    debug << "\n";
    debug << "     name                        = " << bone->name();
    debug << "\n";
    debug << "     english                     = " << bone->englishName();
    debug << "\n";
    debug << "     origin                      = " << bone->origin();
    debug << "\n";
    if (bone->parentBoneRef()) {
        debug << "     parent                      = " << bone->parentBoneRef()->name();
        debug << "\n";
    }
    debug << "     index                       = " << bone->layerIndex();
    debug << "\n";
    debug << "     offset                      = " << bone->origin();
    debug << "\n";
    if (bone->hasInverseKinematics()) {
        debug << "     targetBone                  = " << bone->effectorBoneRef()->name();
        debug << "\n";
        debug << "     constraintAngle             = " << bone->constraintAngle();
        debug << "\n";
    }
    if (bone->hasInherentTranslation() || bone->hasInherentRotation()) {
        debug << "     parentInherentBoneRef       = " << bone->parentInherentBoneRef()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasFixedAxes()) {
        debug << "     axis                        = " << bone->axis();
        debug << "\n";
    }
    if (bone->hasLocalAxes()) {
        debug << "     axisX                       = " << bone->axisX();
        debug << "\n";
        debug << "     axisZ                       = " << bone->axisZ();
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Material *material)
{
    if (!material) {
        debug.nospace() << "Material is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Material id                      = " << material->index();
    debug << "\n";
    debug << "         name                    = " << material->name();
    debug << "\n";
    debug << "         english                 = " << material->englishName();
    debug << "\n";
    debug << "         mainTexture             = " << material->mainTexture();
    debug << "\n";
    debug << "         sphereTexture           = " << material->sphereTexture();
    debug << "\n";
    debug << "         toonTexture             = " << material->toonTexture();
    debug << "\n";
    debug << "         ambient                 = " << material->ambient();
    debug << "\n";
    debug << "         diffuse                 = " << material->diffuse();
    debug << "\n";
    debug << "         specular                = " << material->specular();
    debug << "\n";
    debug << "         edgeColor               = " << material->edgeColor();
    debug << "\n";
    debug << "         shininess               = " << material->shininess();
    debug << "\n";
    debug << "         edgeSize                = " << material->edgeSize();
    debug << "\n";
    debug << "         indices                 = " << material->indexRange().count;
    debug << "\n";
    debug << "         isSharedToonTextureUsed = " << material->isSharedToonTextureUsed();
    debug << "\n";
    debug << "         isCullingDisabled       = " << material->isCullingDisabled();
    debug << "\n";
    debug << "         hasShadow               = " << material->hasShadow();
    debug << "\n";
    debug << "         isSelfShadowEnabled     = " << material->isSelfShadowEnabled();
    debug << "\n";
    debug << "         isEdgeEnabled           = " << material->isEdgeEnabled();
    debug << "\n";
    switch (material->sphereTextureRenderMode()) {
    case pmx::Material::kAddTexture:
        debug << "         sphere                  = add";
        break;
    case pmx::Material::kMultTexture:
        debug << "         sphere                  = modulate";
        break;
    case pmx::Material::kNone:
        debug << "         sphere                  = none";
        break;
    case pmx::Material::kSubTexture:
        debug << "         sphere                  = subtexture";
        break;
    case pmx::Material::kMaxSphereTextureRenderModeType:
        break;
    }
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Morph *morph)
{
    if (!morph) {
        debug.nospace() << "Morph is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Morph id       = " << morph->index();
    debug << "\n";
    debug << "      name     = " << morph->name();
    debug << "\n";
    debug << "      english  = " << morph->englishName();
    debug << "\n";
    debug << "      weight   = " << morph->weight();
    debug << "\n";
    debug << "      category = ";
    switch (morph->category()) {
    case IMorph::kBase:
        debug << "kBase";
        break;
    case IMorph::kEyeblow:
        debug << "kEyeblow";
        break;
    case IMorph::kEye:
        debug << "kEye";
        break;
    case IMorph::kLip:
        debug << "kLip";
        break;
    case IMorph::kOther:
        debug << "kOther";
        break;
    default:
        debug << "(unknown)";
        break;
    }
    debug << "\n";
    debug << "      type     = ";
    switch (morph->type()) {
    case IMorph::kGroupMorph:
        debug << "kGroup";
        break;
    case IMorph::kVertexMorph:
        debug << "kVertex";
        break;
    case IMorph::kBoneMorph:
        debug << "kBone";
        break;
    case IMorph::kTexCoordMorph:
        debug << "kTexCoord";
        break;
    case IMorph::kUVA1Morph:
        debug << "kUVA1";
        break;
    case IMorph::kUVA2Morph:
        debug << "kUVA2";
        break;
    case IMorph::kUVA3Morph:
        debug << "kUVA3";
        break;
    case IMorph::kUVA4Morph:
        debug << "kUVA4";
        break;
    case IMorph::kMaterialMorph:
        debug << "kMaterial";
        break;
    case IMorph::kFlipMorph:
        debug << "kFlip";
        break;
    case IMorph::kImpulseMorph:
        debug << "kImpulse";
        break;
    default:
        debug << "(unknown)";
        break;
    }
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Label *label)
{
    if (!label) {
        debug.nospace() << "Label is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Label id        = " << label->index();
    debug << "\n";
    debug << "      name      = " << label->name();
    debug << "\n";
    debug << "      english   = " << label->englishName();
    debug << "\n";
    debug << "      isSpecial = " << label->isSpecial();
    debug << "\n";
    debug << "      count     = " << label->count();
    debug << "\n";
    for (int i = 0; i < label->count(); i++) {
        if (IBone *bone = label->boneRef(i)) {
            debug << "      bone      = " << bone->name();
        }
        else if (IMorph *morph = label->morphRef(i)) {
            debug << "      morph     = " << morph->name();
        }
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::RigidBody *body)
{
    if (!body) {
        debug.nospace() << "RigidBody is null";
        return debug.space();
    }
    debug.nospace();
    debug << "RigidBody id                 = " << body->index();
    debug << "\n";
    debug << "          name               = " << body->name();
    debug << "\n";
    debug << "          english            = " << body->englishName();
    debug << "\n";
    debug << "          size               = " << body->size();
    debug << "\n";
    debug << "          position           = " << body->position();
    debug << "\n";
    debug << "          rotation           = " << body->rotation();
    debug << "\n";
    debug << "          mass               = " << body->mass();
    debug << "\n";
    debug << "          linearDamping      = " << body->linearDamping();
    debug << "\n";
    debug << "          angularDamping     = " << body->angularDamping();
    debug << "\n";
    debug << "          restitution        = " << body->restitution();
    debug << "\n";
    debug << "          friction           = " << body->friction();
    debug << "\n";
    debug << "          groupID            = " << body->groupID();
    debug << "\n";
    debug << "          collisionGroupMask = " << body->collisionGroupMask();
    debug << "\n";
    debug << "          collisionGroupID   = " << body->collisionGroupID();
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Joint *joint)
{
    if (!joint) {
        debug.nospace() << "Joint is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Joint id                 = " << joint->index();
    debug << "\n";
    debug << "      name               = " << joint->name();
    debug << "\n";
    debug << "      english            = " << joint->englishName();
    debug << "\n";
    debug << "      position           = " << joint->position();
    debug << "\n";
    debug << "      rotation           = " << joint->rotation();
    debug << "\n";
    debug << "      positionLowerLimit = " << joint->positionLowerLimit();
    debug << "\n";
    debug << "      positionUpperLimit = " << joint->positionUpperLimit();
    debug << "\n";
    debug << "      rotationLowerLimit = " << joint->rotationLowerLimit();
    debug << "\n";
    debug << "      rotationUpperLimit = " << joint->rotationUpperLimit();
    debug << "\n";
    debug << "      positionStiffness  = " << joint->positionStiffness();
    debug << "\n";
    debug << "      rotationStiffness  = " << joint->rotationStiffness();
    debug << "\n";
    if (IRigidBody *rigidBodyRef = joint->rigidBody1Ref()) {
        debug << "      rigidBody1         = " << rigidBodyRef->name();
        debug << "\n";
    }
    if (IRigidBody *rigidBodyRef = joint->rigidBody2Ref()) {
        debug << "      rigidBody2         = " << rigidBodyRef->name();
        debug << "\n";
    }
    return debug.space();
}

namespace vpvl2
{
namespace render
{
namespace qt
{

const QString kWindowTitle = "MMDAI2 rendering test program with Qt (FPS:%1)";

static void UIToggleFlags(int target, int &flags)
{
    if ((flags & target) != target) {
        flags -= target;
    }
    else {
        flags += target;
    }
}

template<typename T>
static void UIWaitFor(const QFuture<T> &future)
{
    while (!future.isResultReadyAt(0)) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

UI::UI(const QGLFormat &format)
    : QGLWidget(new CustomGLContext(format), 0),
      m_world(new World()),
      m_scene(new Scene(false)),
      m_audioSource(new AudioSource()),
      m_manualTimeIndex(0),
      m_debugFlags(0),
      m_automaticMotion(false)
{
    setMouseTracking(true);
    setWindowTitle(kWindowTitle.arg("N/A"));
}

UI::~UI()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::DefaultLogger::kill();
#endif
    m_audioSource.reset();
    m_world.take();
    m_dictionary.releaseAll();
}

void UI::load(const QString &filename)
{
    Util::loadDictionary(&m_dictionary);
    m_encoding.reset(new Encoding(&m_dictionary));
    m_settings.reset(new QSettings(filename, QSettings::IniFormat, this));
    m_settings->setIniCodec("UTF-8");
    m_factory.reset(new Factory(m_encoding.data()));
    QHash<QString, QString> settings;
    foreach (const QString &key, m_settings->allKeys()) {
        const QString &value = m_settings->value(key).toString();
        settings.insert(key, value);
        m_stringMapRef.insert(std::make_pair(Util::fromQString(key), Util::fromQString(value)));
    }
}

void UI::rotate(float x, float y)
{
    ICamera *camera = m_scene->cameraRef();
    Vector3 angle = camera->angle();
    angle.setX(angle.x() + x);
    angle.setY(angle.y() + y);
    camera->setAngle(angle);
}

void UI::translate(float x, float y)
{
    ICamera *camera = m_scene->cameraRef();
    const Vector3 &diff = camera->modelViewTransform() * Vector3(x, y, 0);
    const Vector3 &position = camera->lookAt() + diff;
    camera->setLookAt(position + diff);
}

void UI::closeEvent(QCloseEvent *event)
{
    QThreadPool *pool = QThreadPool::globalInstance();
    if (pool->activeThreadCount() > 0)
        pool->waitForDone();
    event->accept();
}

void UI::initializeGL()
{
    GLenum err = 0;
    if (!Scene::initialize(&err)) {
        qFatal("Cannot initialize GLEW: %d", err);
    }
    QGLFormat f = format(); Q_UNUSED(f);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    qDebug("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    const QSize s(m_settings->value("window.width", 960).toInt(), m_settings->value("window.height", 640).toInt());
    const QSize &margin = qApp->desktop()->screenGeometry().size() - s;
    move((margin / 2).width(), (margin / 2).height());
    resize(s);
    setMinimumSize(640, 480);
    m_applicationContext.reset(new ApplicationContext(m_scene.data(), m_encoding.data(), &m_stringMapRef));
    m_applicationContext->initialize(m_settings->value("enable.debug", false).toBool());
    m_applicationContext->updateCameraMatrices(glm::vec2(width(), height()));
    m_scene->setPreferredFPS(qMax(m_settings->value("scene.fps", 30).toFloat(), Scene::defaultFPS()));
    if (m_settings->value("enable.opencl", false).toBool()) {
        m_scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
    }
    else if (m_settings->value("enable.vss", false).toBool()) {
        m_scene->setAccelerationType(Scene::kVertexShaderAccelerationType1);
    }
    ICamera *camera = m_scene->cameraRef();
    camera->setZNear(qMax(m_settings->value("scene.znear", 0.1f).toFloat(), 0.1f));
    camera->setZFar(qMax(m_settings->value("scene.zfar", 10000.0).toFloat(), 100.0f));
    ILight *light = m_scene->lightRef();
    light->setToonEnable(m_settings->value("enable.toon", true).toBool());
    m_helper.reset(new TextureDrawHelper(size()));
    m_helper->load(QRectF(0, 0, 1, 1));
    m_helper->resize(size());
    m_drawer.reset(new DebugDrawer(m_applicationContext.data(), &m_stringMapRef));
    m_drawer->load();
    if (m_settings->value("enable.sm", false).toBool() && Scene::isSelfShadowSupported()) {
        m_applicationContext->createShadowMap(Vector3(2048, 2048, 0));
    }
    if (loadScene()) {
        const QString &path = m_settings->value("audio.file").toString();
        if (!m_audioSource->load(path.toUtf8().constData())) {
            qDebug("Cannot load audio file: %s", m_audioSource->errorString());
        }
        else if (!m_audioSource->play()) {
            qDebug("Cannot play audio source: %s", m_audioSource->errorString());
        }
        unsigned int interval = m_settings->value("window.fps", 30).toUInt();
        m_timeHolder.setUpdateInterval(btSelect(interval, interval / 1.0f, 60.0f)); //60;
        m_updateTimer.start(int(btSelect(interval, 1000.0f / interval, 0.0f)), this);
        m_timeHolder.start();
    }
    else {
        qFatal("Unable to load scene");
    }
}

void UI::timerEvent(QTimerEvent * /* event */)
{
    if (m_automaticMotion) {
        double offset, latency;
        m_audioSource->getOffsetLatency(offset, latency);
        if (offset != 0 || latency != 0) {
            qDebug("offset: %.2f", offset + latency);
        }
        m_timeHolder.saveElapsed(qRound64(offset + latency));
        seekScene(m_timeHolder.timeIndex(), m_timeHolder.delta());
    }
    m_applicationContext->updateCameraMatrices(glm::vec2(width(), height()));
    m_scene->update(Scene::kUpdateAll);
    setWindowTitle(kWindowTitle.arg(m_counter.value()));
    updateGL();
}

void UI::seekScene(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::TimeIndex &delta)
{
    m_scene->seek(timeIndex, Scene::kUpdateAll);
    if (m_scene->isReachedTo(m_scene->duration())) {
        m_scene->seek(0, Scene::kUpdateAll);
        m_scene->update(Scene::kResetMotionState);
        m_timeHolder.reset();
    }
    m_world->stepSimulation(delta);
}

void UI::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_A:
        UIToggleFlags(btIDebugDraw::DBG_DrawAabb, m_debugFlags);
        break;
    case Qt::Key_C:
        UIToggleFlags(btIDebugDraw::DBG_DrawConstraints, m_debugFlags);
        break;
    case Qt::Key_L:
        UIToggleFlags(btIDebugDraw::DBG_DrawConstraintLimits, m_debugFlags);
        break;
    case Qt::Key_N:
        seekScene(++m_manualTimeIndex, 1);
        break;
    case Qt::Key_P:
        seekScene(qMax(--m_manualTimeIndex, IKeyframe::TimeIndex(0)), 1);
        break;
    case Qt::Key_W:
        UIToggleFlags(btIDebugDraw::DBG_DrawWireframe, m_debugFlags);
        break;
    }
}

void UI::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButtons buttons = event->buttons();
    m_prevPos = event->pos();
    if (buttons & Qt::LeftButton) {
        m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseLeftPressPosition, true);
    }
    if (buttons & Qt::MiddleButton) {
        m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseMiddlePressPosition, true);
    }
    if (buttons & Qt::RightButton) {
        m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseRightPressPosition,  true);
    }
    setMousePositions(event);
}

void UI::mouseMoveEvent(QMouseEvent *event)
{
    Qt::MouseButtons buttons = event->buttons();
    bool handled = m_applicationContext->handleUIMouseMotion(event->x(), event->y());
    if (!handled && (buttons & Qt::LeftButton)) {
        const QPoint &diff = event->pos() - m_prevPos;
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers & Qt::ShiftModifier) {
            translate(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotate(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
    setMousePositions(event);
}

void UI::mouseReleaseEvent(QMouseEvent *event)
{
    m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseLeftPressPosition, false);
    m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseMiddlePressPosition, false);
    m_applicationContext->handleUIMouseAction(IApplicationContext::kMouseRightPressPosition, false);
    setMousePositions(event);
}

void UI::wheelEvent(QWheelEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();
    ICamera *camera = m_scene->cameraRef();
    int delta = event->delta();
    if (!m_applicationContext->handleUIMouseWheel(delta)) {
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            const qreal step = 1.0;
            camera->setFov(qMax(delta > 0 ? camera->fov() - step : camera->fov() + step, 0.0));
        }
        else {
            qreal step = 4.0;
            if (modifiers & Qt::ControlModifier) {
                step *= 5.0f;
            }
            else if (modifiers & Qt::ShiftModifier) {
                step *= 0.2f;
            }
            if (step != 0.0f) {
                camera->setDistance(delta > 0 ? camera->distance() - step : camera->distance() + step);
            }
        }
    }
}

void UI::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    if (m_helper) {
        m_helper->resize(QSize(w, h));
    }
}

void UI::paintGL()
{
    if (m_applicationContext) {
        m_applicationContext->renderShadowMap();
        m_applicationContext->renderOffscreen();
        m_applicationContext->updateCameraMatrices(glm::vec2(width(), height()));
        renderWindow();
        if (IShadowMap *shadowMap = m_scene->shadowMapRef()) {
            if (const GLuint *bufferRef = static_cast<GLuint *>(shadowMap->textureRef())) {
                m_helper->draw(QRectF(0, 0, 256, 256), *bufferRef);
            }
        }
        m_drawer->drawWorld(m_world.data(), m_debugFlags);
        if (m_audioSource->isRunning()) {
            m_audioSource->update();
            double offset, latency;
            m_audioSource->getOffsetLatency(offset, latency);
            qDebug("elapsed:%.1f timeIndex:%.2f offset:%.2f latency:%2f",
                   m_timeHolder.elapsed(),
                   m_timeHolder.timeIndex(),
                   offset,
                   latency);
        }
        m_applicationContext->renderControls();
    }
    else {
        glViewport(0, 0, width(), height());
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    bool flushed; /* unused */
    m_counter.update(m_timeHolder.elapsed(), flushed);
}

void UI::renderWindow()
{
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    m_scene->getRenderEnginesByRenderOrder(enginesForPreProcess,
                                           enginesForStandard,
                                           enginesForPostProcess,
                                           nextPostEffects);
    for (int i = enginesForPostProcess.count() - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        engine->update();
        engine->preparePostProcess();
    }
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    for (int i = 0, nengines = enginesForPreProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPreProcess[i];
        engine->performPreProcess();
    }
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
        if (!m_scene->shadowMapRef()) {
            engine->renderShadow();
        }
    }
    for (int i = 0, nengines = enginesForPostProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *const *nextPostEffect = nextPostEffects[engine];
        engine->performPostProcess(*nextPostEffect);
    }
}

void UI::setMousePositions(QMouseEvent *event)
{
#ifdef VPVL2_ENABLE_NVIDIA_CG
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    const QSizeF &size = geometry().size() / 2;
    const qreal w = size.width(), h = size.height();
    const glm::vec2 value((pos.x() - w) / w, (pos.y() - h) / -h);
    Qt::MouseButtons buttons = event->buttons();
    m_applicationContext->setMousePosition(value, buttons & Qt::LeftButton, IApplicationContext::kMouseLeftPressPosition);
    m_applicationContext->setMousePosition(value, buttons & Qt::MiddleButton, IApplicationContext::kMouseMiddlePressPosition);
    m_applicationContext->setMousePosition(value, buttons & Qt::RightButton, IApplicationContext::kMouseRightPressPosition);
    m_applicationContext->setMousePosition(value, false, IApplicationContext::kMouseCursorPosition);
#else
    Q_UNUSED(event);
#endif
}

bool UI::loadScene()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
    Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
#endif
    QScopedPointer<btStaticPlaneShape> ground(new btStaticPlaneShape(Vector3(0, 1, 0), 0));
    btRigidBody::btRigidBodyConstructionInfo info(0, 0, ground.take(), kZeroV3);
    QScopedPointer<btRigidBody> body(new btRigidBody(info));
    m_world->dynamicWorldRef()->addRigidBody(body.take(), 0x10, 0);
    m_scene->setWorldRef(m_world->dynamicWorldRef());
    const QString &modelMotionPath = QDir(m_settings->value("dir.motion").toString())
            .absoluteFilePath(m_settings->value("file.motion").toString());
    const QString &cameraMotionPath = QDir(m_settings->value("dir.camera").toString())
            .absoluteFilePath(m_settings->value("file.camera").toString());
    QProgressDialog dialog(this);
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.setLabelText("Loading scene...");
    dialog.setMaximum(-1);
    dialog.show();
    int nmodels = m_settings->beginReadArray("models");
    for (int i = 0; i < nmodels; i++) {
        m_settings->setArrayIndex(i);
        const QString &path = m_settings->value("path").toString();
        const bool enableEffect = m_settings->value("enable.effects", true).toBool();
        if (!path.isNull()) {
            if (IModelSharedPtr model = addModel(path, dialog, i, enableEffect)) {
                addMotion(modelMotionPath, model.data());
                m_applicationContext->setCurrentModelRef(model.data());
            }
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
    m_settings->endArray();
    IMotionSmartPtr cameraMotion(loadMotion(cameraMotionPath, 0));
    if (IMotion *motion = cameraMotion.release()) {
        m_scene->cameraRef()->setMotion(motion);
    }
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog.setValue(dialog.value() + 1);
    m_scene->seek(0, Scene::kUpdateAll);
    m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    m_automaticMotion = m_settings->value("enable.playing", true).toBool();
    return true;
}

UI::ModelSet UI::createModelAsync(const QString &path)
{
    IModelSharedPtr model;
    QFile file(path);
    bool ok = true;
    if (path.endsWith(".zip")) {
        ArchiveSharedPtr archive(new Archive(m_encoding.data()));
        Archive::EntryNames entries;
        String archivePath(Util::fromQString(path));
        if (archive->open(&archivePath, entries)) {
            const UnicodeString targetExtension(".pmx");
            for (Archive::EntryNames::const_iterator it = entries.begin(); it != entries.end(); it++) {
                const UnicodeString &filename = *it;
                if (filename.endsWith(targetExtension)) {
                    archive->uncompressEntry(filename);
                    const std::string *bytes = archive->dataRef(filename);
                    const uint8 *data = reinterpret_cast<const uint8 *>(bytes->data());
                    const QFileInfo finfo(Util::toQString(filename));
                    archive->setBasePath(Util::fromQString(finfo.path()));
                    model = IModelSharedPtr(m_factory->createModel(data, bytes->size(), ok), &Scene::deleteModelUnlessReferred);
                    break;
                }
            }
            return ModelSet(model, archive);
        }
    }
    else if (file.open(QFile::ReadOnly)) {
        const uint8 *data = static_cast<const uint8 *>(file.map(0, file.size()));
        model = IModelSharedPtr(m_factory->createModel(data, file.size(), ok), &Scene::deleteModelUnlessReferred);
        return ModelSet(model, ArchiveSharedPtr());
    }
    else {
        qWarning("Failed loading the model");
    }
    return ModelSet();
}

IMotion *UI::createMotionAsync(const QString &path, IModel *model)
{
    IMotionSmartPtr motion;
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        bool ok = true;
        const uint8 *data = static_cast<const uint8 *>(file.map(0, file.size()));
        motion.reset(m_factory->createMotion(data, file.size(), model, ok));
    }
    else {
        qWarning("Failed parsing the model motion, skipped...");
    }
    return motion.release();
}

static IEffect *CreateEffectAsync(ApplicationContext *context, IModel *model, const IString *dir)
{
    return context->createEffectRef(model, dir);
}

IModelSharedPtr UI::addModel(const QString &path, QProgressDialog &dialog, int index, bool enableEffect)
{
    const QFileInfo info(path);
    QFuture<ModelSet> future = QtConcurrent::run(this, &UI::createModelAsync, path);
    dialog.setLabelText(QString("Loading %1...").arg(info.fileName()));
    dialog.setRange(0, 0);
    UIWaitFor(future);
    const ModelSet &set = future.result();
    IModelSharedPtr modelPtr = set.first;
    if (future.isCanceled() || !modelPtr) {
        return IModelSharedPtr();
    }
    if (modelPtr->error() != IModel::kNoError) {
        qWarning("Failed parsing the model: %d", modelPtr->error());
        return IModelSharedPtr();
    }
    String s1(Util::fromQString(info.absoluteDir().absolutePath()));
    ApplicationContext::ModelContext modelContext(m_applicationContext.data(), set.second.data(), &s1);
    m_applicationContext->addModelPath(modelPtr.data(), Util::fromQString(info.absoluteFilePath()));
    QFuture<IEffect *> future2 = QtConcurrent::run(&CreateEffectAsync,
                                                   m_applicationContext.data(),
                                                   modelPtr.data(), &s1);
    dialog.setLabelText(QString("Loading an effect of %1...").arg(info.fileName()));
    UIWaitFor(future2);
    IEffect *effectRef = future2.result();
    int flags = enableEffect ? Scene::kEffectCapable : 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (!effectRef) {
        qWarning() << "Effect" <<  m_applicationContext->effectFilePath(modelPtr.data(), &s1) << "does not exists";
    }
    else if (!effectRef->internalPointer()) {
        CGcontext c = static_cast<CGcontext>(effectRef->internalContext());
        qWarning() << cgGetLastListing(c);
    }
    else {
        effectRef->createFrameBufferObject();
    }
#else
    Q_UNUSED(effectRef)
#endif
    QScopedPointer<IRenderEngine> enginePtr(m_scene->createRenderEngine(m_applicationContext.data(),
                                                                        modelPtr.data(), flags));
    enginePtr->setEffect(effectRef, IEffect::kAutoDetection, &modelContext);
    if (enginePtr->upload(&modelContext)) {
        m_applicationContext->parseOffscreenSemantic(effectRef, &s1);
        modelPtr->setEdgeWidth(m_settings->value("edge.width", 1.0).toFloat());
        modelPtr->setPhysicsEnable(m_settings->value("enable.physics", true).toBool());
        if (!modelPtr->name()) {
            String s(Util::fromQString(info.fileName()));
            modelPtr->setName(&s);
        }
        bool parallel = m_settings->value("enable.parallel", true).toBool();
        enginePtr->setUpdateOptions(parallel ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone);
        m_scene->addModel(modelPtr.data(), enginePtr.take(), index);
    }
    else {
        return IModelSharedPtr();
    }
#if 0
    if (pmx::Model *pmx = dynamic_cast<pmx::Model*>(model)) {
        const Array<pmx::Material *> &materials = pmx->materials();
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++)
            qDebug() << materials[i];
        const Array<pmx::Bone *> &bones = pmx->bones();
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++)
            qDebug() << bones[i];
        const Array<pmx::Morph *> &morphs = pmx->morphs();
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++)
            qDebug() << morphs.at(i);
        const Array<pmx::Label *> &labels = pmx->labels();
        const int nlabels = labels.count();
        for (int i = 0; i < nlabels; i++)
            qDebug() << labels.at(i);
        const Array<pmx::RigidBody *> &bodies = pmx->rigidBodies();
        const int nbodies = bodies.count();
        for (int i = 0; i < nbodies; i++)
            qDebug() << bodies.at(i);
        const Array<pmx::Joint *> &joints = pmx->joints();
        const int njoints = joints.count();
        for (int i = 0; i < njoints; i++)
            qDebug() << joints.at(i);
    }
#endif
    return modelPtr;
}

IMotion *UI::addMotion(const QString &path, IModel *model)
{
    IMotionSmartPtr motion(loadMotion(path, model));
    if (IMotion *m = motion.get()) {
        m_scene->addMotion(m);
    }
    return motion.release();
}

IMotion *UI::loadMotion(const QString &path, IModel *model)
{
    QFuture<IMotion *> future = QtConcurrent::run(this, &UI::createMotionAsync, path, model);
    UIWaitFor(future);
    IMotionSmartPtr motionPtr(future.result());
    return future.isCanceled() ? 0 : motionPtr.release();
}

}
}
}

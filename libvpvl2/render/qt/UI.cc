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

#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include "UI.h"

#include <btBulletDynamicsCommon.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/World.h>
#include <vpvl2/qt/CustomGLContext.h>
#include <vpvl2/qt/DebugDrawer.h>
#include <vpvl2/qt/RenderContext.h>
#include <vpvl2/qt/TextureDrawHelper.h>
#include <vpvl2/qt/Util.h>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QApplication>
#include <QDesktopWidget>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrent>
#endif

#include <glm/gtc/matrix_transform.hpp>
#if !defined(_MSC_VER) && !defined(__APPLE__)
#include <GL/glext.h>
#else
#pragma warning(push)
#pragma warning(disable:4005)
#include <vpvl2/extensions/gl/khronos/glext.h>
#pragma warning(pop)
#endif /* _MSC_VER */

#ifdef VPVL2_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
BT_DECLARE_HANDLE(aiScene);
#endif

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
        debug << "     targetBone                  = " << bone->targetBoneRef()->name();
        debug << "\n";
        debug << "     constraintAngle             = " << bone->constraintAngle();
        debug << "\n";
    }
    if (bone->hasPositionInherence()) {
        debug << "     parentPositionInherenceBone = " << bone->parentInherenceBoneRef()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasRotationInherence()) {
        debug << "     parentRotationInherenceBone = " << bone->parentInherenceBoneRef()->name();
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
    debug << "         isCullDisabled          = " << material->isCullFaceDisabled();
    debug << "\n";
    debug << "         hasShadow               = " << material->hasShadow();
    debug << "\n";
    debug << "         isShadowMapDrawin       = " << material->isShadowMapDrawn();
    debug << "\n";
    debug << "         isEdgeDrawn             = " << material->isEdgeDrawn();
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
        if (IBone *bone = label->bone(i))
            debug << "      bone      = " << bone->name();
        else if (IMorph *morph = label->morph(i))
            debug << "      morph     = " << morph->name();
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
    if (joint->rigidBody1()) {
        debug << "      rigidBody1         = " << joint->rigidBody1()->name();
        debug << "\n";
    }
    if (joint->rigidBody2()) {
        debug << "      rigidBody2         = " << joint->rigidBody2()->name();
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

static void UIToggleFlags(int target, int &flags)
{
    if ((flags & target) != target) {
        flags -= target;
    }
    else {
        flags += target;
    }
}

UI::UI(const QGLFormat &format)
    : QGLWidget(new CustomGLContext(format), 0),
      m_world(new World()),
      m_scene(new Scene(false)),
      m_prevElapsed(0),
      m_currentFrameIndex(0),
      m_debugFlags(0),
      m_automaticMotion(false)
{
    setMouseTracking(true);
    setWindowTitle("MMDAI2 rendering test program with Qt");
}

UI::~UI()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::DefaultLogger::kill();
#endif
    m_world.take();
    m_dictionary.releaseAll();
}

void UI::load(const QString &filename)
{
    m_settings.reset(new QSettings(filename, QSettings::IniFormat, this));
    m_settings->setIniCodec("UTF-8");
    createEncoding(m_settings.data());
    m_factory.reset(new Factory(m_encoding.data()));
    QHash<QString, QString> settings;
    foreach (const QString &key, m_settings->allKeys()) {
        const QString &value = m_settings->value(key).toString();
        settings.insert(key, value);
        m_stringMapRef.insert(std::make_pair(Util::fromQString(key), Util::fromQString(value)));
    }
    const QSize s(m_settings->value("window.width", 960).toInt(), m_settings->value("window.height", 640).toInt());
    const QSize &margin = qApp->desktop()->screenGeometry().size() - s;
    move((margin / 2).width(), (margin / 2).height());
    resize(s);
    setMinimumSize(640, 480);
    m_renderContext.reset(new RenderContext(m_scene.data(), &m_stringMapRef));
    m_renderContext->updateCameraMatrices(glm::vec2(width(), height()));
    m_scene->setPreferredFPS(qMax(m_settings->value("scene.fps", 30).toFloat(), Scene::defaultFPS()));
    if (m_settings->value("enable.opencl", false).toBool())
        m_scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
    else if (m_settings->value("enable.vss", false).toBool())
        m_scene->setAccelerationType(Scene::kVertexShaderAccelerationType1);
    ICamera *camera = m_scene->camera();
    camera->setZNear(qMax(m_settings->value("scene.znear", 0.1f).toFloat(), 0.1f));
    camera->setZFar(qMax(m_settings->value("scene.zfar", 10000.0).toFloat(), 100.0f));
    ILight *light = m_scene->light();
    light->setToonEnable(m_settings->value("enable.toon", true).toBool());
    m_helper.reset(new TextureDrawHelper(size()));
    m_helper->load(QRectF(0, 0, 1, 1));
    m_helper->resize(size());
    m_drawer.reset(new DebugDrawer(m_renderContext.data(), &m_stringMapRef));
    m_drawer->load();
    if (m_settings->value("enable.sm", false).toBool() && Scene::isSelfShadowSupported()) {
        m_renderContext->createShadowMap(Vector3(2048, 2048, 0));
    }
    if (loadScene()) {
        m_updateTimer.start(0, this);
        m_refreshTimer.start();
    }
    else {
        qFatal("Unable to load scene");
    }
}

void UI::rotate(float x, float y)
{
    ICamera *camera = m_scene->camera();
    Vector3 angle = camera->angle();
    angle.setX(angle.x() + x);
    angle.setY(angle.y() + y);
    camera->setAngle(angle);
}

void UI::translate(float x, float y)
{
    ICamera *camera = m_scene->camera();
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
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    qDebug("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void UI::timerEvent(QTimerEvent * /* event */)
{
    if (m_automaticMotion) {
        const IKeyframe::TimeIndex &elapsed = m_refreshTimer.elapsed() / static_cast<Scalar>(60.0f);
        IKeyframe::TimeIndex delta(elapsed - m_prevElapsed);
        m_prevElapsed = elapsed;
        if (delta < 0)
            delta = elapsed;
        proceedScene(delta);
    }
    m_renderContext->updateCameraMatrices(glm::vec2(width(), height()));
    m_scene->update(Scene::kUpdateAll);
    updateGL();
}

void UI::proceedScene(const IKeyframe::TimeIndex &delta)
{
    m_scene->advance(delta, Scene::kUpdateAll);
    if (m_scene->isReachedTo(m_scene->maxTimeIndex())) {
        m_scene->seek(0, Scene::kUpdateAll);
        m_scene->update(Scene::kResetMotionState);
        m_currentFrameIndex = 0;
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
        proceedScene(+1);
        break;
    case Qt::Key_P:
        proceedScene(-1);
        break;
    case Qt::Key_W:
        UIToggleFlags(btIDebugDraw::DBG_DrawWireframe, m_debugFlags);
        break;
    }
}

void UI::mousePressEvent(QMouseEvent *event)
{
    m_prevPos = event->pos();
    setMousePositions(event);
}

void UI::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        const QPoint &diff = event->pos() - m_prevPos;
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
    setMousePositions(event);
}

void UI::wheelEvent(QWheelEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();
    ICamera *camera = m_scene->camera();
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        const qreal step = 1.0;
        camera->setFov(qMax(event->delta() > 0 ? camera->fov() - step : camera->fov() + step, 0.0));
    }
    else {
        qreal step = 4.0;
        if (modifiers & Qt::ControlModifier)
            step *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            step *= 0.2f;
        if (step != 0.0f)
            camera->setDistance(event->delta() > 0 ? camera->distance() - step : camera->distance() + step);
    }
}

void UI::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    if (m_helper)
        m_helper->resize(QSize(w, h));
}

void UI::paintGL()
{
    if (m_renderContext) {
        m_renderContext->renderShadowMap();
        m_renderContext->renderOffscreen();
        m_renderContext->updateCameraMatrices(glm::vec2(width(), height()));
        renderWindow();
        if (IShadowMap *shadowMap = m_scene->shadowMapRef()) {
            if (const GLuint *bufferRef = static_cast<GLuint *>(shadowMap->textureRef())) {
                m_helper->draw(QRectF(0, 0, 256, 256), *bufferRef);
            }
        }
        m_drawer->drawWorld(m_world.data(), m_debugFlags);
    }
    else {
        glViewport(0, 0, width(), height());
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
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
        if (!m_scene->shadowMapRef())
            engine->renderShadow();
    }
    for (int i = 0, nengines = enginesForPostProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *const *nextPostEffect = nextPostEffects[engine];
        engine->performPostProcess(*nextPostEffect);
    }
}

void UI::setMousePositions(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    const QSizeF &size = geometry().size() / 2;
    const qreal w = size.width(), h = size.height();
    const glm::vec2 value((pos.x() - w) / w, (pos.y() - h) / -h);
    Qt::MouseButtons buttons = event->buttons();
    m_renderContext->setMousePosition(value, buttons & Qt::LeftButton, IRenderContext::kMouseLeftPressPosition);
    m_renderContext->setMousePosition(value, buttons & Qt::MiddleButton, IRenderContext::kMouseMiddlePressPosition);
    m_renderContext->setMousePosition(value, buttons & Qt::RightButton, IRenderContext::kMouseRightPressPosition);
    m_renderContext->setMousePosition(value, false, IRenderContext::kMouseCursorPosition);
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
            IModelSmartPtr model(addModel(path, dialog, i, enableEffect));
            if (IModel *m = model.release())
                addMotion(modelMotionPath, m);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
    m_settings->endArray();
    IMotionSmartPtr cameraMotion(loadMotion(cameraMotionPath, 0));
    if (IMotion *motion = cameraMotion.release())
        m_scene->camera()->setMotion(motion);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog.setValue(dialog.value() + 1);
    m_scene->seek(0, Scene::kUpdateAll);
    m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    m_automaticMotion = m_settings->value("enable.playing", true).toBool();
    return true;
}

IModel *UI::createModelAsync(const QString &path)
{
    IModelSmartPtr model;
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        bool ok = true;
        const uint8_t *data = static_cast<const uint8_t *>(file.map(0, file.size()));
        model.reset(m_factory->createModel(data, file.size(), ok));
    }
    else {
        qWarning("Failed loading the model");
    }
    return model.release();
}

IMotion *UI::createMotionAsync(const QString &path, IModel *model)
{
    IMotionSmartPtr motion;
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        bool ok = true;
        const uint8_t *data = static_cast<const uint8_t *>(file.map(0, file.size()));
        motion.reset(m_factory->createMotion(data, file.size(), model, ok));
    }
    else {
        qWarning("Failed parsing the model motion, skipped...");
    }
    return motion.release();
}

static IEffect *CreateEffectAsync(RenderContext *context, IModel *model, const IString *dir)
{
    return context->createEffectRef(model, dir);
}

IModel *UI::addModel(const QString &path, QProgressDialog &dialog, int index, bool enableEffect)
{
    const QFileInfo info(path);
    QFuture<IModel *> future = QtConcurrent::run(this, &UI::createModelAsync, path);
    dialog.setLabelText(QString("Loading %1...").arg(info.fileName()));
    dialog.setRange(0, 0);
    while (!future.isResultReadyAt(0))
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    IModelSmartPtr modelPtr(future.result());
    if (!modelPtr.get() || future.isCanceled()) {
        return 0;
    }
    if (modelPtr->error() != IModel::kNoError) {
        qWarning("Failed parsing the model: %d", modelPtr->error());
        return 0;
    }
    String s1(Util::fromQString(info.absoluteDir().absolutePath()));
    m_renderContext->addModelPath(modelPtr.get(), Util::fromQString(info.absoluteFilePath()));
    QFuture<IEffect *> future2 = QtConcurrent::run(&CreateEffectAsync,
                                                   m_renderContext.data(),
                                                   modelPtr.get(), &s1);
    dialog.setLabelText(QString("Loading an effect of %1...").arg(info.fileName()));
    while (!future2.isResultReadyAt(0))
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    IEffect *effectRef = future2.result();
    int flags = enableEffect ? Scene::kEffectCapable : 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (!effectRef) {
        qWarning() << "Effect" <<  m_renderContext->effectFilePath(modelPtr.get(), &s1) << "does not exists";
    }
    else if (!effectRef->internalPointer()) {
        CGcontext c = static_cast<CGcontext>(effectRef->internalContext());
        qWarning() << cgGetLastListing(c);
    }
    else {
        effectRef->createFrameBufferObject();
    }
#else
    Q_UNUSED(effect)
#endif
    QScopedPointer<IRenderEngine> enginePtr(m_scene->createRenderEngine(m_renderContext.data(),
                                                                        modelPtr.get(), flags));
    enginePtr->setEffect(IEffect::kAutoDetection, effectRef, &s1);
    if (enginePtr->upload(&s1)) {
        m_renderContext->parseOffscreenSemantic(effectRef, &s1);
        modelPtr->setEdgeWidth(m_settings->value("edge.width", 1.0).toFloat());
        modelPtr->setPhysicsEnable(m_settings->value("enable.physics", true).toBool());
        if (!modelPtr->name()) {
            String s(Util::fromQString(info.fileName()));
            modelPtr->setName(&s);
        }
        bool parallel = m_settings->value("enable.parallel", true).toBool();
        enginePtr->setUpdateOptions(parallel ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone);
        m_scene->addModel(modelPtr.get(), enginePtr.take(), index);
    }
    else {
        return 0;
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
    return modelPtr.release();
}

IMotion *UI::addMotion(const QString &path, IModel *model)
{
    IMotionSmartPtr motion(loadMotion(path, model));
    if (IMotion *m = motion.get())
        m_scene->addMotion(m);
    return motion.release();
}

IMotion *UI::loadMotion(const QString &path, IModel *model)
{
    QFuture<IMotion *> future = QtConcurrent::run(this, &UI::createMotionAsync, path, model);
    while (!future.isResultReadyAt(0))
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    IMotionSmartPtr motionPtr(future.result());
    return future.isCanceled() ? 0 : motionPtr.release();
}

void UI::createEncoding(QSettings *settings)
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
    while (it.hasNext()) {
        it.next();
        const QVariant &value = settings->value("constants." + it.key());
        m_dictionary.insert(it.value(), new String(Util::fromQString(value.toString())));
    }
    m_encoding.reset(new Encoding(&m_dictionary));
}

}
}
}

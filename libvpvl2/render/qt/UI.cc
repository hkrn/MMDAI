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

#include "UI.h"
#include "Delegate.h"
#include "Encoding.h"
#include "CString.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>

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
#include "vpvl2/pmd/Model.h"
#include "vpvl2/vmd/Motion.h"

/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>

using namespace vpvl2;
using namespace vpvl2::render::qt;

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
        debug.nospace() << static_cast<const CString *>(str)->value();
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
    if (bone->parentBone()) {
        debug << "     parent                      = " << bone->parentBone()->name();
        debug << "\n";
    }
    debug << "     index                       = " << bone->layerIndex();
    debug << "\n";
    debug << "     offset                      = " << bone->origin();
    debug << "\n";
    if (bone->hasInverseKinematics()) {
        debug << "     targetBone                  = " << bone->targetBone()->name();
        debug << "\n";
        debug << "     constraintAngle             = " << bone->constraintAngle();
        debug << "\n";
    }
    if (bone->hasPositionInherence()) {
        debug << "     parentPositionInherenceBone = " << bone->parentInherenceBone()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasRotationInherence()) {
        debug << "     parentRotationInherenceBone = " << bone->parentInherenceBone()->name();
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
    debug << "         indices                 = " << material->indices();
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
    debug << "Morph id      = " << morph->index();
    debug << "\n";
    debug << "      name    = " << morph->name();
    debug << "\n";
    debug << "      english = " << morph->englishName();
    debug << "\n";
    debug << "      weight  = " << morph->weight();
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

UI::UI()
    : QGLWidget(QGLFormat(QGL::SampleBuffers), 0),
      #ifndef VPVL2_NO_BULLET
      m_dispatcher(&m_config),
      m_broadphase(Vector3(-10000.0f, -10000.0f, -10000.0f), Vector3(10000.0f, 10000.0f, 10000.0f), 1024),
      m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config),
      #endif /* VPVL2_NO_BULLET */
      m_settings(0),
      m_fbo(0),
      m_delegate(0),
      m_factory(0),
      m_encoding(0),
      m_depthTextureID(0),
      m_frameBufferID(0),
      m_renderBufferID(0),
      m_prevElapsed(0),
      m_currentFrameIndex(0)
{
    Encoding *encoding = new Encoding();
    m_encoding = encoding;
    m_factory = new Factory(encoding);
#ifndef VPVL2_NO_BULLET
    m_world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
    m_world.getSolverInfo().m_numIterations = static_cast<int>(10.0f);
#endif /* VPVL2_NO_BULLET */
    setMinimumSize(320, 240);
    setMouseTracking(true);
}

UI::~UI()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::DefaultLogger::kill();
#endif
    glDeleteFramebuffers(1, &m_frameBufferID);
    glDeleteRenderbuffers(1, &m_renderBufferID);
    delete m_fbo;
    delete m_factory;
    delete m_encoding;
}

void UI::load(const QString &filename)
{
    m_settings = new QSettings(filename, QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");
    m_delegate = new Delegate(m_settings, &m_scene, this);
    m_delegate->updateMatrices(size());
    resize(m_settings->value("window.width", 640).toInt(), m_settings->value("window.height", 480).toInt());
    m_scene.setPreferredFPS(qMax(m_settings->value("scene.fps", 30).toFloat(), Scene::defaultFPS()));
    if (m_settings->value("enable.opencl", false).toBool())
        m_scene.setAccelerationType(Scene::kOpenCLAccelerationType1);
    else if (m_settings->value("enable.vss", false).toBool())
        m_scene.setAccelerationType(Scene::kVertexShaderAccelerationType1);
    ICamera *camera = m_scene.camera();
    camera->setZNear(qMax(m_settings->value("scene.znear", 0.1f).toFloat(), 0.1f));
    camera->setZFar(qMax(m_settings->value("scene.zfar", 10000.0).toFloat(), 100.0f));
    ILight *light = m_scene.light();
    light->setToonEnable(m_settings->value("enable.toon", true).toBool());
    light->setSoftShadowEnable(m_settings->value("enable.ss", true).toBool());
    if (m_fbo) {
        m_depthTextureID = m_fbo->texture();
        light->setDepthTextureSize(Vector3(m_fbo->width(), m_fbo->height(), 0.0));
        if (m_settings->value("enable.sm", false).toBool())
            light->setDepthTexture(&m_depthTextureID);
    }
    if (loadScene()) {
        startTimer(1000.0f / 60.0f);
        m_timer.start();
    }
    else {
        qFatal("Unable to load scene");
    }
    glGenFramebuffers(1, &m_frameBufferID);
    glGenRenderbuffers(1, &m_renderBufferID);
}

void UI::rotate(float x, float y)
{
    ICamera *camera = m_scene.camera();
    Vector3 angle = camera->angle();
    angle.setX(angle.x() + x);
    angle.setY(angle.y() + y);
    camera->setAngle(angle);
}

void UI::translate(float x, float y)
{
    ICamera *camera = m_scene.camera();
    const Vector3 &diff = camera->modelViewTransform() * Vector3(x, y, 0);
    Vector3 position = camera->position() + diff;
    camera->setPosition(position + diff);
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
    initializeGLFunctions();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    ILight *light = m_scene.light();
    QGLFramebufferObjectFormat format;
    format.setAttachment(QGLFramebufferObject::Depth);
#if GL_ARB_texture_float
    format.setInternalTextureFormat(GL_RGBA32F_ARB);
    light->setHasFloatTexture(true);
#endif
    m_fbo = new QGLFramebufferObject(1024, 1024, format);
    GLuint textureID = m_depthTextureID = m_fbo->texture();
    if (textureID > 0) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void UI::timerEvent(QTimerEvent *)
{
    float elapsed = m_timer.elapsed() / static_cast<float>(60.0f);
    float diff = elapsed - m_prevElapsed;
    m_prevElapsed = elapsed;
    if (diff < 0)
        diff = elapsed;
    m_scene.advance(diff);
    const Array<IMotion *> &motions = m_scene.motions();
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        if (motion->isReachedTo(motion->maxTimeIndex())) {
            motion->reset();
            m_currentFrameIndex = 0;
        }
    }
    m_world.stepSimulation(diff * (60.0 / m_scene.preferredFPS()));
    m_delegate->updateMatrices(size());
    m_scene.updateRenderEngines();
    updateGL();
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
    ICamera *camera = m_scene.camera();
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
}

void UI::paintGL()
{
    if (!m_delegate) {
        glViewport(0, 0, width(), height());
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }
    renderDepth();
    renderOffscreen();
    renderWindow();
}

void UI::renderDepth()
{
    if (m_scene.light()->depthTexture()) {
        glDisable(GL_BLEND);
        m_fbo->bind();
        Vector3 target = kZeroV3, center;
        Scalar maxRadius = 0, radius;
        const Array<IRenderEngine *> &engines = m_scene.renderEngines();
        const Array<IModel *> &models = m_scene.models();
        const int nengines = engines.count();
        const int nmodels = models.count();
        Array<Scalar> radiusArray;
        Array<Vector3> centerArray;
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            if (model->isVisible()) {
                model->getBoundingSphere(center, radius);
                radiusArray.add(radius);
                centerArray.add(target);
                target += center;
            }
        }
        target /= nmodels;
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            if (model->isVisible()) {
                const Vector3 &c = centerArray[i];
                const Scalar &r = radiusArray[i];
                const Scalar &d = target.distance(c) + r;
                btSetMax(maxRadius, d);
            }
        }
        const Scalar &angle = 45;
        const Scalar &distance = maxRadius / btSin(btRadians(angle) * 0.5);
        const Scalar &margin = 50;
        const Vector3 &eye = -m_scene.light()->direction().normalized() * maxRadius + target;
        QMatrix4x4 lightViewMatrix, lightProjectionMatrix;
        lightViewMatrix.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                               QVector3D(target.x(), target.y(), target.z()),
                               QVector3D(0, 1, 0));
        lightProjectionMatrix.perspective(angle, 1, 1, distance + maxRadius + margin);
        QMatrix4x4 lightWorldMatrix;
        m_delegate->setLightMatrices(lightWorldMatrix, lightViewMatrix, lightProjectionMatrix);
        glViewport(0, 0, m_fbo->width(), m_fbo->height());
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            engine->renderZPlot();
        }
        m_fbo->release();
#ifndef VPVL2_ENABLE_NVIDIA_CG
        lightWorldMatrix.scale(0.5);
        lightWorldMatrix.translate(1, 1, 1);
        m_delegate->setLightMatrices(lightWorldMatrix, lightViewMatrix, lightProjectionMatrix);
#endif
    }
}

void UI::renderOffscreen()
{
#ifdef VPVL2_ENABLE_NVIDIA_CG
    const Array<IRenderEngine *> &engines = m_scene.renderEngines();
    const int nengines = engines.count();
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    static const GLuint buffers[] = { GL_COLOR_ATTACHMENT0 };
    static const int nbuffers = sizeof(buffers) / sizeof(buffers[0]);
    glDrawBuffers(nbuffers, buffers);
    QSize s;
    foreach (const OffscreenRenderTarget &offscreen, m_offscreens) {
        const IEffect::OffscreenRenderTarget &renderTarget = offscreen.first;
        const CGparameter sampler = static_cast<CGparameter>(renderTarget.samplerParameter);
        const CGparameter parameter = static_cast<CGparameter>(renderTarget.textureParameter);
        size_t width = renderTarget.width, height = renderTarget.height;
        GLuint textureID = cgGLGetTextureParameter(sampler);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferID);
        CGannotation clearColor = cgGetNamedParameterAnnotation(parameter, "ClearColor");
        if (cgIsAnnotation(clearColor)) {
            int nvalues;
            const float *color = cgGetFloatAnnotationValues(clearColor, &nvalues);
            if (nvalues == 4) {
                glClearColor(color[0], color[1], color[2], color[3]);
            }
        }
        else {
            glClearColor(1, 1, 1, 1);
        }
        CGannotation clearDepth = cgGetNamedParameterAnnotation(parameter, "ClearDepth");
        if (cgIsAnnotation(clearDepth)) {
            int nvalues;
            const float *depth = cgGetFloatAnnotationValues(clearDepth, &nvalues);
            if (nvalues == 1) {
                glClearDepth(depth[0]);
            }
        }
        else {
            glClearDepth(0);
        }
        s.setWidth(width);
        s.setHeight(height);
        m_delegate->updateMatrices(s);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            const IModel *model = engine->model();
            const IString *name = model->name();
            const QString &n = name ? static_cast<const CString *>(name)->value() : m_delegate->findModelPath(model);
            foreach (const EffectAttachment &attachment, offscreen.second) {
                IEffect *effect = attachment.second;
                if (attachment.first.exactMatch(n)) {
                    engine->setEffect(IEffect::kStandardOffscreen, effect, 0);
                    break;
                }
            }
            engine->update();
            engine->renderModel();
            engine->renderEdge();
        }
#if 0
        glBindTexture(GL_TEXTURE_2D, textureID);
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        image.save(QString("%1/texture%2.png").arg(QDir::homePath()).arg(textureID));
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
    }
    m_delegate->updateMatrices(size());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
}

void UI::renderWindow()
{
    const Array<IRenderEngine *> &engines = m_scene.renderEngines();
    const int nengines = engines.count();
    glViewport(0, 0, width(), height());
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPostProcess);
        engine->setEffect(IEffect::kPostProcess, effect, 0);
        engine->preparePostProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPreProcess);
        engine->setEffect(IEffect::kPreProcess, effect, 0);
        engine->performPreProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kStandard);
        engine->setEffect(IEffect::kStandard, effect, 0);
        engine->renderModel();
        engine->renderEdge();
        engine->renderShadow();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPostProcess);
        engine->setEffect(IEffect::kPostProcess, effect, 0);
        engine->performPostProcess();
    }
}

void UI::setMousePositions(QMouseEvent *event)
{
    const QPointF &pos = event->posF();
    const QSizeF &size = geometry().size() / 2;
    const qreal w = size.width(), h = size.height();
    const Vector3 &value = Vector3((pos.x() - w) / w, (pos.y() - h) / -h, 0);
    Qt::MouseButtons buttons = event->buttons();
    m_delegate->setMousePosition(value, buttons & Qt::LeftButton, IRenderDelegate::kMouseLeftPressPosition);
    m_delegate->setMousePosition(value, buttons & Qt::MiddleButton, IRenderDelegate::kMouseMiddlePressPosition);
    m_delegate->setMousePosition(value, buttons & Qt::RightButton, IRenderDelegate::kMouseRightPressPosition);
    m_delegate->setMousePosition(value, false, IRenderDelegate::kMouseCursorPosition);
}

bool UI::loadScene()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
    Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
    // addModel(QDir(kStageDir).absoluteFilePath(kStageName));
    // addModel(kStage2Name, kStageDir);
#endif
    const QString &modelPath = QDir(m_settings->value("dir.model").toString())
            .absoluteFilePath(m_settings->value("file.model").toString());
    const QString &assetPath = QDir(m_settings->value("dir.asset").toString())
            .absoluteFilePath(m_settings->value("file.asset").toString());
    const QString &modelMotionPath = QDir(m_settings->value("dir.motion").toString())
            .absoluteFilePath(m_settings->value("file.motion").toString());
    const QString &cameraMotionPath = QDir(m_settings->value("dir.camera").toString())
            .absoluteFilePath(m_settings->value("file.camera").toString());
    QProgressDialog dialog(this);
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.setLabelText("Loading scene...");
    dialog.setMaximum(4);
    dialog.show();
    IModel *model = addModel(modelPath, dialog);
    if (model)
        addMotion(modelMotionPath, model);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog.setValue(dialog.value() + 1);
    addModel(assetPath, dialog);
    IMotion *cameraMotion = loadMotion(cameraMotionPath, 0);
    if (cameraMotion)
        m_scene.camera()->setMotion(cameraMotion);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog.setValue(dialog.value() + 1);
    m_scene.seek(0);
    return true;
}

IModel *UI::createModelAsync(const QString &path) const
{
    QByteArray bytes;
    if (!UISlurpFile(path, bytes)) {
        qWarning("Failed loading the model");
        return 0;
    }
    bool ok = true;
    const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
    return m_factory->createModel(data, bytes.size(), ok);
}

IEffect *UI::createEffectAsync(const IString *path)
{
    IEffect *effect = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    const QString &key = static_cast<const CString *>(path)->value();
    m_effectCachesLock.lock();
    if (m_effectCaches.contains(key)) {
        effect = m_effectCaches[key];
    }
    else {
        m_effectCachesLock.unlock();
        effect = m_scene.createEffect(path, m_delegate);
        if (!effect->internalPointer())
            qWarning() << cgGetLastListing(static_cast<CGcontext>(effect->internalContext()));
        m_effectCachesLock.lock();
        m_effectCaches.insert(key, effect);
    }
    m_effectCachesLock.unlock();
#else
    Q_UNUSED(path)
#endif
    return effect;
}

IEffect *UI::createEffectAsync(IModel *model, const IString *dir)
{
    IEffect *effect = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    const QString &key = m_delegate->effectFilePath(model, dir);
    m_effectCachesLock.lock();
    if (m_effectCaches.contains(key)) {
        effect = m_effectCaches[key];
    }
    else {
        effect = m_scene.createEffect(dir, model, m_delegate);
        if (!effect->internalPointer())
            qWarning() << cgGetLastListing(static_cast<CGcontext>(effect->internalContext()));
        m_effectCaches.insert(key, effect);
        m_delegate->setEffectOwner(effect, model);
    }
    m_effectCachesLock.unlock();
#else
    Q_UNUSED(model)
    Q_UNUSED(dir)
#endif
    return effect;
}

IMotion *UI::createMotionAsync(const QString &path, IModel *model) const
{
    QByteArray bytes;
    if (UISlurpFile(path, bytes)) {
        bool ok = true;
        IMotion *motion = m_factory->createMotion(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size(), model, ok);
        motion->seek(0);
        return motion;
    }
    else {
        qWarning("Failed parsing the model motion, skipped...");
    }
    return 0;
}

IModel *UI::addModel(const QString &path, QProgressDialog &dialog)
{
    const QFileInfo info(path);
    const QFuture<IModel *> &future = QtConcurrent::run(this, &UI::createModelAsync, path);
    dialog.setLabelText(QString("Loading %1...").arg(info.fileName()));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QScopedPointer<IModel> modelPtr(future.result());
    dialog.setValue(dialog.value() + 1);
    if (!modelPtr || future.isCanceled()) {
        return 0;
    }
    if (modelPtr->error() != IModel::kNoError) {
        qWarning("Failed parsing the model: %d", modelPtr->error());
        return 0;
    }
    QScopedPointer<IRenderEngine> enginePtr(m_scene.createRenderEngine(m_delegate, modelPtr.data()));
    CString s1(info.absoluteDir().absolutePath());
    IModel *model = 0;
    IRenderEngine *engine = 0;
    if (enginePtr->upload(&s1)) {
        modelPtr->setEdgeWidth(m_settings->value("edge.width", 1.0).toFloat());
        modelPtr->joinWorld(&m_world);
        m_scene.addModel(modelPtr.data(), enginePtr.data());
        m_delegate->addModelPath(modelPtr.data(), info.fileName());
        model = modelPtr.take();
        engine = enginePtr.take();
    }
    else {
        modelPtr.reset();
        enginePtr.reset();
        return 0;
    }
    const QFuture<IEffect *> &future2 = QtConcurrent::run(this, &UI::createEffectAsync, model, &s1);
    dialog.setLabelText(QString("Loading an effect of %1...").arg(info.fileName()));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QScopedPointer<IEffect> effectPtr(future2.result());
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (!effectPtr->internalPointer()) {
        CGcontext c = static_cast<CGcontext>(effectPtr->internalContext());
        qWarning() << cgGetLastListing(c);
    }
    else {
        const QDir &baseDir = info.dir();
        const QRegExp fxRegExp(".fx$");
        Array<IEffect::OffscreenRenderTarget> offscreenRenderTargets;
        IEffect *effect = effectPtr.data();
        engine->setEffect(IEffect::kAutoDetection, effectPtr.take(), &s1);
        effect->getOffscreenRenderTargets(offscreenRenderTargets);
        const int nOffscreenRenderTargets = offscreenRenderTargets.count();
        for (int i = 0; i < nOffscreenRenderTargets; i++) {
            const IEffect::OffscreenRenderTarget &renderTarget = offscreenRenderTargets[i];
            const CGparameter parameter = static_cast<const CGparameter>(renderTarget.textureParameter);
            const CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "DefaultEffect");
            const QStringList defaultEffect = QString(cgGetStringAnnotationValue(annotation)).split(";");
            QList<EffectAttachment> attachments;
            foreach (const QString &line, defaultEffect) {
                const QStringList &pair = line.split('=');
                if (pair.size() == 2) {
                    const QString &key = pair.at(0).trimmed();
                    const QString &value = pair.at(1).trimmed();
                    QRegExp regexp(key, Qt::CaseSensitive, QRegExp::Wildcard);
                    if (key == "self") {
                        const QString &name = m_delegate->effectOwnerName(effect);
                        regexp.setPattern(name);
                    }
                    if (value != "hide" && value != "none") {
                        QString path = baseDir.absoluteFilePath(value);
                        path.replace(fxRegExp, ".cgfx");
                        CString s2(path);
                        const QFuture<IEffect *> &future3 = QtConcurrent::run(this, &UI::createEffectAsync, &s2);
                        IEffect *offscreenEffect = future3.result();
                        offscreenEffect->setParentEffect(effect);
                        attachments.append(EffectAttachment(regexp, offscreenEffect));
                    }
                    else {
                        attachments.append(EffectAttachment(regexp, 0));
                    }
                }
            }
            m_offscreens.append(OffscreenRenderTarget(renderTarget, attachments));
        }
    }
#else
    Q_UNUSED(effectPtr)
#endif
#if 0
    pmx::Model *pmx = dynamic_cast<pmx::Model*>(res);
    if (pmx) {
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
    return model;
}

IMotion *UI::addMotion(const QString &path, IModel *model)
{
    IMotion *motion = loadMotion(path, model);
    if (motion)
        m_scene.addMotion(motion);
    return motion;
}

IMotion *UI::loadMotion(const QString &path, IModel *model)
{
    const QFuture<IMotion *> &future = QtConcurrent::run(this, &UI::createMotionAsync, path, model);
    IMotion *motion = future.result();
    if (!motion || future.isCanceled()) {
        delete motion;
        return 0;
    }
    return motion;
}

}
}
}

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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#ifdef VPVL2_OPENGL_RENDERER
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#endif

#ifdef VPVL2_ENABLE_NVIDIA_CG
#include "vpvl2/cg/AssetRenderEngine.h"
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/PMXRenderEngine.h"
#else
BT_DECLARE_HANDLE(CGcontext);
#endif /* VPVL2_ENABLE_NVIDIA_CG */

#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#else
namespace vpvl2 {
namespace cl {
class Context;
class PMXAccelerator;
}
}
#endif /* VPVL2_ENABLE_OPENCL */

namespace
{

using namespace vpvl2;

class Light : public ILight {
public:
    Light() :
        m_motion(0),
        m_color(kZeroV3),
        m_direction(kZeroV3),
        m_depthTextureSize(kZeroV3),
        m_enableToon(false),
        m_enableSoftShadow(false),
        m_hasFloatTexture(false),
        m_depthTexture(0)
    {
        resetDefault();
    }
    ~Light() {
        delete m_motion;
        m_motion = 0;
        m_depthTexture = 0;
        m_enableToon = false;
        m_enableSoftShadow = false;
        m_hasFloatTexture = false;
        m_color.setZero();
        m_direction.setZero();
        m_depthTextureSize.setZero();
    }

    const Vector3 &color() const { return m_color; }
    const Vector3 &direction() const { return m_direction; }
    const Vector3 &depthTextureSize() const { return m_depthTextureSize; }
    void *depthTexture() const { return m_depthTexture; }
    bool hasFloatTexture() const { return m_hasFloatTexture; }
    bool isToonEnabled() const { return m_enableToon; }
    bool isSoftShadowEnabled() const { return m_enableSoftShadow; }
    IMotion *motion() const { return m_motion; }
    void setColor(const Vector3 &value) { m_color = value; }
    void setDirection(const Vector3 &value) { m_direction = value; }
    void setDepthTextureSize(const Vector3 &value) { m_depthTextureSize = value; }
    void setHasFloatTexture(bool value) { m_hasFloatTexture = value; }
    void setMotion(IMotion *value) { m_motion = value; }
    void setDepthTexture(void *value) { m_depthTexture = value; }
    void setToonEnable(bool value) { m_enableToon = value; }
    void setSoftShadowEnable(bool value) { m_enableSoftShadow = value; }
    void copyFrom(const ILight *value) {
        setColor(value->color());
        setDirection(value->direction());
    }
    void resetDefault() {
        setColor(Vector3(0.6f, 0.6f, 0.6f));
        setDirection(Vector3(-0.5f, -1.0f, -0.5f));
    }

private:
    IMotion *m_motion;
    Vector3 m_color;
    Vector3 m_direction;
    Vector3 m_depthTextureSize;
    bool m_enableToon;
    bool m_enableSoftShadow;
    bool m_hasFloatTexture;
    void *m_depthTexture;
};

class Camera : public ICamera {
public:
    Camera()
        : m_motion(0),
          m_transform(Transform::getIdentity()),
          m_lookAt(kZeroV3),
          m_position(kZeroV3),
          m_angle(kZeroV3),
          m_distance(kZeroV3),
          m_fov(0),
          m_znear(0.1f),
          m_zfar(10000)
    {
        resetDefault();
    }
    ~Camera()
    {
        delete m_motion;
        m_motion = 0;
        m_transform.setIdentity();
        m_lookAt.setZero();
        m_position.setZero();
        m_angle.setZero();
        m_distance.setZero();
        m_fov = 0;
        m_znear = 0;
        m_zfar = 0;
    }

    const Transform &modelViewTransform() const { return m_transform; }
    const Vector3 &lookAt() const { return m_lookAt; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &angle() const { return m_angle; }
    Scalar fov() const { return m_fov; }
    Scalar distance() const { return m_distance.z(); }
    Scalar znear() const { return m_znear; }
    Scalar zfar() const { return m_zfar; }
    IMotion *motion() const { return m_motion; }
    void setLookAt(const Vector3 &value) { m_lookAt = value; }
    void setAngle(const Vector3 &value) { m_angle = value; }
    void setFov(Scalar value) { m_fov = value; }
    void setZNear(Scalar value) { m_znear = value; }
    void setZFar(Scalar value) { m_zfar = value; }
    void setMotion(IMotion *value) { m_motion = value; }
    void setDistance(Scalar value) {
        m_distance.setZ(value);
        m_position = m_lookAt + m_distance;
    }
    void copyFrom(const ICamera *value) {
        setLookAt(value->lookAt());
        setAngle(value->angle());
        setFov(value->fov());
        setDistance(value->distance());
        setZNear(value->znear());
        setZFar(value->zfar());
    }
    void resetDefault() {
        setLookAt(Vector3(0, 10, 0));
        setFov(30);
        setDistance(50);
        updateTransform();
    }

    void updateTransform() {
        static const Vector3 kUnitX(1, 0, 0), kUnitY(0, 1, 0), kUnitZ(0, 0, 1);
        const Quaternion rotationX(kUnitX, vpvl2::radian(m_angle.x())),
                rotationY(kUnitY, vpvl2::radian(m_angle.y())),
                rotationZ(kUnitZ, vpvl2::radian(m_angle.z()));
        m_transform.setIdentity();
        m_transform.setRotation(rotationZ * rotationX * rotationY);
        m_transform.setOrigin((m_transform * -m_lookAt) - m_distance);
    }

private:
    IMotion *m_motion;
    Transform m_transform;
    Quaternion m_rotation;
    Vector3 m_lookAt;
    Vector3 m_position;
    Vector3 m_angle;
    Vector3 m_distance;
    Scalar m_fov;
    Scalar m_znear;
    Scalar m_zfar;
};

}

namespace vpvl2
{

struct Scene::PrivateContext {
    PrivateContext()
        : computeContext(0),
          accelerationType(Scene::kSoftwareFallback),
          effectContext(0),
          preferredFPS(Scene::defaultFPS())
    {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        effectContext = cgCreateContext();
        cgSetParameterSettingMode(effectContext, CG_DEFERRED_PARAMETER_SETTING);
        cgGLSetDebugMode(CG_FALSE);
        cgGLSetManageTextureParameters(effectContext, CG_TRUE);
        cgGLRegisterStates(effectContext);
#endif
    }
    ~PrivateContext() {
        motions.releaseAll();
        engines.releaseAll();
        models.releaseAll();
#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
        delete computeContext;
        computeContext = 0;
#endif /* VPVL2_ENABLE_OPENCL */
#ifdef VPVL2_ENABLE_NVIDIA_CG
        cgDestroyContext(effectContext);
        effectContext = 0;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }

    void updateMotionState() {
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            model->resetMotionState();
        }
    }
    void updateRenderEngines() {
        const int nengines = engines.count();
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            engine->update();
        }
    }
    void updateCamera() {
        camera.updateTransform();
    }

    bool isOpenCLAcceleration() const {
        return accelerationType == kOpenCLAccelerationType1 || accelerationType == kOpenCLAccelerationType2;
    }
#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
    cl_uint hostDeviceType() const {
        switch (accelerationType) {
        case kOpenCLAccelerationType1:
            return CL_DEVICE_TYPE_ALL;
        case kOpenCLAccelerationType2:
            return CL_DEVICE_TYPE_CPU;
        default:
            return CL_DEVICE_TYPE_DEFAULT;
        }
    }
#endif
    cl::Context *createComputeContext(IRenderContext *delegateRef) {
#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
        if (!computeContext) {
            computeContext = new cl::Context(delegateRef);
            if (!computeContext->initialize(hostDeviceType())) {
                delete computeContext;
                computeContext = 0;
            }
        }
#else
        (void) delegateRef;
#endif /* VPVL2_ENABLE_OPENCL */
        return computeContext;
    }
    cl::PMXAccelerator *createPMXAccelerator(IRenderContext *delegate, IModel *modelRef) {
        cl::PMXAccelerator *accelerator = 0;
#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
        if (isOpenCLAcceleration()) {
            if (cl::Context *context = createComputeContext(delegate)) {
                accelerator = new cl::PMXAccelerator(context, modelRef);
                accelerator->createKernelProgram();
            }
        }
#else
        (void) delegate;
#endif /* VPVL2_ENABLE_OPENCL */
        return accelerator;
    }
    IEffect *compileEffect(IString *source) {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        CGeffect effect = 0;
        if (source) {
            static const char *kCompilerArguments[] = {
                "-DVPVM",
                "-DVPVL2_VERSION=" VPVL2_VERSION_STRING,
                0
            };
            effect = cgCreateEffect(effectContext, reinterpret_cast<const char *>(source->toByteArray()), kCompilerArguments);
        }
        delete source;
        return new cg::Effect(effectContext, cgIsEffect(effect) ? effect : 0);
#else
        delete source;
        return 0;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }

    cl::Context *computeContext;
    Scene::AccelerationType accelerationType;
    CGcontext effectContext;
    Hash<HashPtr, IRenderEngine *> model2engineRef;
    Hash<HashString, IModel *> name2modelRef;
    Array<IModel *> models;
    Array<IMotion *> motions;
    Array<IRenderEngine *> engines;
    Light light;
    Camera camera;
    Color lightColor;
    Scalar preferredFPS;
};

ICamera *Scene::createCamera()
{
    return new Camera();
}

ILight *Scene::createLight()
{
    return new Light();
}

bool Scene::isAcceleratorSupported()
{
#if defined(VPVL2_OPENGL_RENDERER) && defined(VPVL2_ENABLE_OPENCL)
    return true;
#else
    return false;
#endif
}

const Scalar &Scene::defaultFPS()
{
    static const Scalar kDefaultFPS = 30;
    return kDefaultFPS;
}

Scene::Scene()
    : m_context(0)
{
    m_context = new Scene::PrivateContext();
}

Scene::~Scene()
{
    delete m_context;
    m_context = 0;
}

IRenderEngine *Scene::createRenderEngine(IRenderContext *delegate, IModel *model, int flags) const
{
    IRenderEngine *engine = 0;
#ifdef VPVL2_OPENGL_RENDERER
    switch (model->type()) {
    case IModel::kAsset: {
#ifdef VPVL2_LINK_ASSIMP
        asset::Model *m = static_cast<asset::Model *>(model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (flags & kEffectCapable)
            engine = new cg::AssetRenderEngine(delegate, this, m_context->effectContext, m);
        else
#endif /* VPVL2_ENABLE_NVIDIA_CG */
            engine = new gl2::AssetRenderEngine(delegate, this, m);
#endif /* VPVL2_LINK_ASSIMP */
        break;
    }
    case IModel::kPMD:
    case IModel::kPMX: {
        cl::PMXAccelerator *accelerator = m_context->createPMXAccelerator(delegate, model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (flags & kEffectCapable)
            engine = new cg::PMXRenderEngine(delegate, this, m_context->effectContext, accelerator, model);
        else
#endif /* VPVL2_ENABLE_NVIDIA_CG */
            engine = new gl2::PMXRenderEngine(delegate, this, accelerator, model);
        break;
    }
    default:
        break;
    }
#ifndef VPVL2_ENABLE_NVIDIA_CG
    (void) flags;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
#else
    (void) delegate;
    (void) model;
    (void) flags;
#endif /* VPVL2_OPENGL_RENDERER */
    return engine;
}

void Scene::addModel(IModel *model, IRenderEngine *engine)
{
    if (model && engine) {
        m_context->models.add(model);
        m_context->engines.add(engine);
        m_context->model2engineRef.insert(model, engine);
        m_context->name2modelRef.insert(model->name()->toHashString(), model);
    }
}

void Scene::addMotion(IMotion *motion)
{
    if (motion)
        m_context->motions.add(motion);
}

IEffect *Scene::createEffect(const IString *path, IRenderContext *delegate)
{
#ifdef VPVL2_OPENGL_RENDERER
    IString *source = delegate->loadShaderSource(IRenderContext::kModelEffectTechniques, path);
    return m_context->compileEffect(source);
#else
    (void) path;
    (void) delegate;
    return 0;
#endif /* VPVL2_OPENGL_RENDERER */
}

IEffect *Scene::createEffect(const IString *dir, const IModel *model, IRenderContext *delegate)
{
#ifdef VPVL2_OPENGL_RENDERER
    IString *source = delegate->loadShaderSource(IRenderContext::kModelEffectTechniques, model, dir, 0);
    return m_context->compileEffect(source);
#else
    (void) dir;
    (void) model;
    (void) delegate;
    return 0;
#endif /* VPVL2_OPENGL_RENDERER */
}

void Scene::deleteModel(IModel *&model)
{
    const HashPtr key(model);
    IRenderEngine **enginePtr = const_cast<IRenderEngine **>(m_context->model2engineRef.find(key));
    if (enginePtr) {
        IRenderEngine *engine = *enginePtr;
        m_context->models.remove(model);
        m_context->engines.remove(engine);
        m_context->model2engineRef.remove(key);
        delete engine;
    }
    delete model;
    model = 0;
}

void Scene::removeMotion(IMotion *motion)
{
    if (motion)
        m_context->motions.remove(motion);
}

void Scene::advance(const IKeyframe::TimeIndex &delta, int flags)
{
    if (flags & kUpdateCamera) {
        Camera &camera = m_context->camera;
        IMotion *cameraMotion = camera.motion();
        if (cameraMotion)
            cameraMotion->advanceScene(delta, this);
    }
    if (flags & kUpdateLight) {
        Light &light = m_context->light;
        IMotion *lightMotion = light.motion();
        if (lightMotion)
            lightMotion->advanceScene(delta, this);
    }
    if (flags & kUpdateModels) {
        const Array<IMotion *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i];
            motion->advance(delta);
        }
    }
    if (flags & kResetMotionState) {
        m_context->updateMotionState();
    }
}

void Scene::seek(const IKeyframe::TimeIndex &timeIndex, int flags)
{
    if (flags & kUpdateCamera) {
        Camera &camera = m_context->camera;
        IMotion *cameraMotion = camera.motion();
        if (cameraMotion)
            cameraMotion->seekScene(timeIndex, this);
    }
    if (flags & kUpdateLight) {
        Light &light = m_context->light;
        IMotion *lightMotion = light.motion();
        if (lightMotion)
            lightMotion->seekScene(timeIndex, this);
    }
    if (flags & kUpdateModels) {
        const Array<IMotion *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i];
            motion->seek(timeIndex);
        }
    }
}

void Scene::updateModel(IModel *model) const
{
    if (model) {
        model->performUpdate();
        if (IRenderEngine *engine = findRenderEngine(model))
            engine->update();
    }
}

void Scene::update(int flags)
{
    if (flags & kUpdateCamera) {
        m_context->updateCamera();
    }
    if (flags & kUpdateRenderEngines) {
        m_context->updateRenderEngines();
    }
    if (flags & kResetMotionState) {
        m_context->updateMotionState();
    }
}

void Scene::setPreferredFPS(const Scalar &value)
{
    m_context->preferredFPS = value;
}

bool Scene::isReachedTo(const IKeyframe::TimeIndex &timeIndex) const
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        if (!motion->isReachedTo(timeIndex))
            return false;
    }
    return true;
}

IKeyframe::TimeIndex Scene::maxFrameIndex() const
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    IKeyframe::TimeIndex maxFrameIndex = 0;
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        btSetMax(maxFrameIndex, motion->maxTimeIndex());
    }
    return maxFrameIndex;
}

const Array<IModel *> &Scene::models() const
{
    return m_context->models;
}

const Array<IMotion *> &Scene::motions() const
{
    return m_context->motions;
}

const Array<IRenderEngine *> &Scene::renderEngines() const
{
    return m_context->engines;
}

IModel *Scene::findModel(const IString *name) const
{
    IModel **model = const_cast<IModel **>(m_context->name2modelRef.find(name->toHashString()));
    return model ? *model : 0;
}

IRenderEngine *Scene::findRenderEngine(IModel *model) const
{
    IRenderEngine **engine = const_cast<IRenderEngine **>(m_context->model2engineRef.find(model));
    return engine ? *engine : 0;
}

ILight *Scene::light() const
{
    return &m_context->light;
}

ICamera *Scene::camera() const
{
    return &m_context->camera;
}

const Scalar &Scene::preferredFPS() const
{
    return m_context->preferredFPS;
}

Scene::AccelerationType Scene::accelerationType() const
{
    return m_context->accelerationType;
}

void Scene::setAccelerationType(AccelerationType value)
{
    m_context->accelerationType = value;
}

}

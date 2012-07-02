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
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#ifdef VPVL2_OPENGL_RENDERER
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMDRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#endif

#ifdef VPVL2_ENABLE_NVIDIA_CG
#include "vpvl2/cg/AssetRenderEngine.h"
#include "vpvl2/cg/PMDRenderEngine.h"
#include "vpvl2/cg/PMXRenderEngine.h"
#else
BT_DECLARE_HANDLE(CGcontext);
#endif /* VPVL2_ENABLE_NVIDIA_CG */

#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMDAccelerator.h"
#include "vpvl2/cl/PMXAccelerator.h"
#else
namespace vpvl2 {
namespace cl {
class Context;
class PMDAccelerator;
class PMXAccelerator;
}
}
#endif /* VPVL2_ENABLE_OPENCL */

namespace
{

using namespace vpvl2;

class Light : public Scene::ILight {
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
    void copyFrom(ILight *value) {
        setColor(value->color());
        setDirection(value->direction());
    }
    void resetDefault() {
        setColor(Vector3(0.6, 0.6, 0.6));
        setDirection(Vector3(-0.5, -1.0, -0.5));
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
class Camera : public Scene::ICamera {
public:
    Camera()
        : m_motion(0),
          m_transform(Transform::getIdentity()),
          m_position(kZeroV3),
          m_angle(kZeroV3),
          m_fov(0),
          m_distance(0),
          m_znear(0.1),
          m_zfar(10000)
    {
        resetDefault();
    }
    ~Camera()
    {
        delete m_motion;
        m_motion = 0;
        m_transform.setIdentity();
        m_position.setZero();
        m_angle.setZero();
        m_fov = 0;
        m_distance = 0;
        m_znear = 0;
        m_zfar = 0;
    }

    const Transform &modelViewTransform() const { return m_transform; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &angle() const { return m_angle; }
    Scalar fov() const { return m_fov; }
    Scalar distance() const { return m_distance; }
    Scalar znear() const { return m_znear; }
    Scalar zfar() const { return m_zfar; }
    IMotion *motion() const { return m_motion; }
    void setPosition(const Vector3 &value) { m_position = value; }
    void setAngle(const Vector3 &value) { m_angle = value; }
    void setFov(Scalar value) { m_fov = value; }
    void setDistance(Scalar value) { m_distance = value; }
    void setZNear(Scalar value) { m_znear = value; }
    void setZFar(Scalar value) { m_zfar = value; }
    void setMotion(IMotion *value) { m_motion = value; }
    void copyFrom(ICamera *value) {
        setPosition(value->position());
        setAngle(value->angle());
        setFov(value->fov());
        setDistance(value->distance());
        setZNear(value->znear());
        setZFar(value->zfar());
    }
    void resetDefault() {
        setPosition(Vector3(0, 10, 0));
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
        m_transform.setOrigin(m_transform * -m_position - Vector3(0, 0, m_distance));
    }

private:
    IMotion *m_motion;
    Transform m_transform;
    Quaternion m_rotation;
    Vector3 m_position;
    Vector3 m_angle;
    Scalar m_fov;
    Scalar m_distance;
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
        cgGLSetDebugMode(CG_TRUE);
        cgGLSetManageTextureParameters(effectContext, CG_TRUE);
        cgGLRegisterStates(effectContext);
#endif
    }
    ~PrivateContext() {
        motions.releaseAll();
        engines.releaseAll();
        models.releaseAll();
#ifdef VPVL2_ENABLE_OPENCL
        delete computeContext;
        computeContext = 0;
#endif /* VPVL2_ENABLE_OPENCL */
#ifdef VPVL2_ENABLE_NVIDIA_CG
        cgDestroyContext(effectContext);
        effectContext = 0;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }

    cl::Context *createComputeContext(IRenderDelegate *delegate) {
#ifdef VPVL2_ENABLE_OPENCL
        if (!computeContext) {
            computeContext = new cl::Context(delegate);
            computeContext->initializeContext();
        }
#else
        (void) delegate;
#endif /* VPVL2_ENABLE_OPENCL */
        return computeContext;
    }
    cl::PMDAccelerator *createPMDAccelerator(IRenderDelegate *delegate) {
        cl::PMDAccelerator *accelerator = 0;
#ifdef VPVL2_ENABLE_OPENCL
        if (accelerationType == kOpenCLAccelerationType1) {
            accelerator = new cl::PMDAccelerator(createComputeContext(delegate));
            accelerator->createKernelProgram();
        }
#else
        (void) delegate;
#endif /* VPVL2_ENABLE_OPENCL */
        return accelerator;
    }
    cl::PMXAccelerator *createPMXAccelerator(IRenderDelegate *delegate) {
        cl::PMXAccelerator *accelerator = 0;
#ifdef VPVL2_ENABLE_OPENCL
        if (accelerationType == kOpenCLAccelerationType1) {
            accelerator = new cl::PMXAccelerator(createComputeContext(delegate));
            accelerator->createKernelProgram();
        }
#else
        (void) delegate;
#endif /* VPVL2_ENABLE_OPENCL */
        return accelerator;
    }

    cl::Context *computeContext;
    Scene::AccelerationType accelerationType;
    CGcontext effectContext;
    Hash<HashPtr, IRenderEngine *> model2engine;
    Array<IModel *> models;
    Array<IMotion *> motions;
    Array<IRenderEngine *> engines;
    Light light;
    Camera camera;
    Color lightColor;
    Scalar preferredFPS;
};

Scene::ICamera *Scene::createCamera()
{
    return new Camera();
}

Scene::ILight *Scene::createLight()
{
    return new Light();
}

bool Scene::isAcceleratorSupported()
{
#ifdef VPVL2_ENABLE_OPENCL
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

IRenderEngine *Scene::createRenderEngine(IRenderDelegate *delegate, IModel *model) const
{
    IRenderEngine *engine = 0;
#ifdef VPVL2_OPENGL_RENDERER
    switch (model->type()) {
    case IModel::kAsset: {
#ifdef VPVL2_LINK_ASSIMP
        asset::Model *m = static_cast<asset::Model *>(model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        engine = new cg::AssetRenderEngine(delegate, this, m_context->effectContext, m);
#else
        engine = new gl2::AssetRenderEngine(delegate, this, m);
#endif /* VPVL2_ENABLE_NVIDIA_CG */
#endif /* VPVL2_LINK_ASSIMP */
        break;
    }
    case IModel::kPMD: {
        cl::PMDAccelerator *accelerator = m_context->createPMDAccelerator(delegate);
        pmd::Model *m = static_cast<pmd::Model *>(model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        engine = new cg::PMDRenderEngine(delegate, this, m_context->effectContext, accelerator, m);
#else
        engine = new gl2::PMDRenderEngine(delegate, this, accelerator, m);
#endif /* VPVL2_ENABLE_NVIDIA_CG */
        break;
    }
    case IModel::kPMX: {
        cl::PMXAccelerator *accelerator = m_context->createPMXAccelerator(delegate);
        pmx::Model *m = static_cast<pmx::Model *>(model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        engine = new cg::PMXRenderEngine(delegate, this, m_context->effectContext, accelerator, m);
#else
        engine = new gl2::PMXRenderEngine(delegate, this, accelerator, m);
#endif /* VPVL2_ENABLE_NVIDIA_CG */
        break;
    }
    default:
        break;
    }
#endif /* VPVL2_OPENGL_RENDERER */
    return engine;
}

void Scene::addModel(IModel *model, IRenderEngine *engine)
{
    const bool isSoftwareSkinning = accelerationType() == kSoftwareFallback;
    switch (model->type()) {
    case IModel::kPMD:
        static_cast<pmd::Model *>(model)->setSkinnningEnable(isSoftwareSkinning);
        break;
    case IModel::kPMX:
        static_cast<pmx::Model *>(model)->setSkinningEnable(isSoftwareSkinning);
        break;
    case IModel::kAsset:
    default:
        break;
    }
    m_context->models.add(model);
    m_context->engines.add(engine);
    m_context->model2engine.insert(HashPtr(model), engine);
}

void Scene::addMotion(IMotion *motion)
{
    m_context->motions.add(motion);
}

void Scene::deleteModel(IModel *&model)
{
    const HashPtr key(model);
    IRenderEngine **enginePtr = const_cast<IRenderEngine **>(m_context->model2engine.find(key));
    if (enginePtr) {
        IRenderEngine *engine = *enginePtr;
        m_context->models.remove(model);
        m_context->engines.remove(engine);
        m_context->model2engine.remove(key);
        delete engine;
        delete model;
        model = 0;
    }
}

void Scene::removeMotion(IMotion *motion)
{
    m_context->motions.remove(motion);
}

void Scene::advance(const IKeyframe::Index &delta)
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        motion->advance(delta);
    }
    updateModels();
    updateRenderEngines();
    Camera &camera = m_context->camera;
    vmd::Motion *cameraMotion = static_cast<vmd::Motion *>(camera.motion());
    if (cameraMotion) {
        vmd::CameraAnimation *animation = cameraMotion->mutableCameraAnimation();
        if (animation->countKeyframes() > 0) {
            animation->advance(delta);
            camera.setPosition(animation->position());
            camera.setAngle(animation->angle());
            camera.setFov(animation->fovy());
            camera.setDistance(animation->distance());
        }
    }
    Light &light = m_context->light;
    vmd::Motion *lightMotion = static_cast<vmd::Motion *>(m_context->light.motion());
    if (lightMotion) {
        vmd::LightAnimation *animation = lightMotion->mutableLightAnimation();
        if (animation->countKeyframes() > 0) {
            animation->seek(delta);
            light.setColor(animation->color());
            light.setDirection(animation->direction());
        }
    }
    updateCamera();
}

void Scene::seek(const IKeyframe::Index &frameIndex)
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        motion->seek(frameIndex);
    }
    updateModels();
    updateRenderEngines();
    Camera &camera = m_context->camera;
    vmd::Motion *cameraMotion = static_cast<vmd::Motion *>(camera.motion());
    if (cameraMotion) {
        vmd::CameraAnimation *animation = cameraMotion->mutableCameraAnimation();
        if (animation->countKeyframes() > 0) {
            animation->seek(frameIndex);
            camera.setPosition(animation->position());
            camera.setAngle(animation->angle());
            camera.setFov(animation->fovy());
            camera.setDistance(animation->distance());
        }
    }
    Light &light = m_context->light;
    vmd::Motion *lightMotion = static_cast<vmd::Motion *>(camera.motion());
    if (lightMotion) {
        vmd::LightAnimation *animation = lightMotion->mutableLightAnimation();
        if (animation->countKeyframes() > 0) {
            animation->seek(frameIndex);
            light.setColor(animation->color());
            light.setDirection(animation->direction());
        }
    }
    updateCamera();
}

void Scene::updateModels()
{
    const Vector3 &lightDirection = light()->direction();
    const Array<IModel *> &models = m_context->models;
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        IModel *model = models[i];
        model->performUpdate(lightDirection);
    }
}

void Scene::updateRenderEngines()
{
    const Array<IRenderEngine *> &renderEngines = m_context->engines;
    const int nRenderEngines = renderEngines.count();
    for (int i = 0; i < nRenderEngines; i++) {
        IRenderEngine *engine = renderEngines[i];
        engine->update();
    }
}

void Scene::updateCamera()
{
    Camera &camera = m_context->camera;
    camera.updateTransform();
}

void Scene::setPreferredFPS(const Scalar &value)
{
    m_context->preferredFPS = value;
}

bool Scene::isReachedTo(const IKeyframe::Index &frameIndex) const
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        if (!motion->isReachedTo(frameIndex))
            return false;
    }
    return true;
}

float Scene::maxFrameIndex() const
{
    const Array<IMotion *> &motions = m_context->motions;
    const int nmotions = motions.count();
    IKeyframe::Index maxFrameIndex = 0;
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        btSetMax(maxFrameIndex, motion->maxFrameIndex());
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

IRenderEngine *Scene::renderEngine(IModel *model) const
{
    IRenderEngine **engine = const_cast<IRenderEngine **>(m_context->model2engine.find(HashPtr(model)));
    return engine ? *engine : 0;
}

Scene::ILight *Scene::light() const
{
    return &m_context->light;
}

Scene::ICamera *Scene::camera() const
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

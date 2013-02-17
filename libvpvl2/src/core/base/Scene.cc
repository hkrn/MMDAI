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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/mvd/Motion.h"
#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#else
#include "vpvl2/pmd2/Model.h"
#endif /* VPVL2_LINK_VPVL */
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#ifdef VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#endif /* VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT */

#ifdef VPVL2_ENABLE_NVIDIA_CG
#include "vpvl2/cg/AssetRenderEngine.h"
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/PMXRenderEngine.h"
#else
BT_DECLARE_HANDLE(CGcontext);
#endif /* VPVL2_ENABLE_NVIDIA_CG */

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/ConstraintSolver/btConstraintSolver.h>

#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
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

#ifdef VPVL2_LINK_GLEW
static bool g_isGLEWInitialized = false;
#endif

static void VPVL2SceneSetParentSceneRef(IModel *model, Scene *scene) {
    if (model) {
        switch (model->type()) {
        case IModel::kAssetModel:
            static_cast<asset::Model *>(model)->setParentSceneRef(scene);
            break;
        case IModel::kPMDModel:
#ifdef VPVL2_LINK_VPVL
            static_cast<pmd::Model *>(model)->setParentSceneRef(scene);
#else
            static_cast<pmd2::Model *>(model)->setParentSceneRef(scene);
#endif
            break;
        case IModel::kPMXModel:
            static_cast<pmx::Model *>(model)->setParentSceneRef(scene);
            break;
        default:
            break;
        }
    }
}

static void VPVL2SceneSetParentSceneRef(IMotion *motion, Scene *scene) {
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVDMotion:
            static_cast<mvd::Motion *>(motion)->setParentSceneRef(scene);
            break;
        case IMotion::kVMDMotion:
            static_cast<vmd::Motion *>(motion)->setParentSceneRef(scene);
            break;
        default:
            break;
        }
    }
}

class Light : public ILight {
public:
    Light(Scene *sceneRef) :
        m_sceneRef(sceneRef),
        m_motion(0),
        m_color(kZeroV3),
        m_direction(kZeroV3),
        m_depthTextureSize(kZeroV3),
        m_enableToon(false),
        m_enableSoftShadow(false),
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
        m_color.setZero();
        m_direction.setZero();
        m_depthTextureSize.setZero();
    }

    Vector3 color() const { return m_color; }
    Vector3 direction() const { return m_direction; }
    Vector3 shadowMapSize() const { return m_depthTextureSize; }
    void *shadowMapTextureRef() const { return m_depthTexture; }
    bool isToonEnabled() const { return m_enableToon; }
    bool isSoftShadowEnabled() const { return m_enableSoftShadow; }
    IMotion *motion() const { return m_motion; }
    void setColor(const Vector3 &value) { m_color = value; }
    void setDirection(const Vector3 &value) { m_direction = value; }
    void setShadowMapSize(const Vector3 &value) { m_depthTextureSize = value; }
    void setShadowMapTextureRef(void *value) { m_depthTexture = value; }
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
    void setMotion(IMotion *value) {
        VPVL2SceneSetParentSceneRef(m_motion, 0);
        m_motion = value;
        VPVL2SceneSetParentSceneRef(value, m_sceneRef);
    }

private:
    Scene *m_sceneRef;
    IMotion *m_motion;
    Vector3 m_color;
    Vector3 m_direction;
    Vector3 m_depthTextureSize;
    bool m_enableToon;
    bool m_enableSoftShadow;
    void *m_depthTexture;
};

class Camera : public ICamera {
public:
    Camera(Scene *sceneRef)
        : m_sceneRef(sceneRef),
          m_motion(0),
          m_transform(Transform::getIdentity()),
          m_lookAt(kZeroV3),
          m_position(kZeroV3),
          m_angle(kZeroV3),
          m_distance(kZeroV3),
          m_fov(0),
          m_znear(0.5f),
          m_zfar(1000000)
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

    Transform modelViewTransform() const { return m_transform; }
    Vector3 lookAt() const { return m_lookAt; }
    Vector3 position() const { return m_position; }
    Vector3 angle() const { return m_angle; }
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
    void setMotion(IMotion *value) {
        VPVL2SceneSetParentSceneRef(m_motion, 0);
        m_motion = value;
        VPVL2SceneSetParentSceneRef(value, m_sceneRef);
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
    Scene *m_sceneRef;
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

#ifdef VPVL2_ENABLE_NVIDIA_CG
static CGbool VPVL2CGFXSetStateDisable(CGstateassignment value, int compare, GLenum name)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == compare)
            glDisable(name);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXSetStateEnable(CGstateassignment value, int compare, GLenum name)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == compare)
            glEnable(name);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXAlphaBlendEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_BLEND);
}

static CGbool VPVL2CGFXAlphaBlendEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_BLEND);
}

static CGbool VPVL2CGFXAlphaTestEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, GL_TRUE, GL_ALPHA_TEST);
}

static CGbool VPVL2CGFXAlphaTestEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, GL_TRUE, GL_ALPHA_TEST);
}

static CGbool VPVL2CGFXBlendFuncSet(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (nvalues == 2 && values[0] != GL_SRC_ALPHA && values[1] != GL_ONE_MINUS_SRC_ALPHA)
            glBlendFunc(values[0], values[1]);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXBlendFuncReset(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (nvalues == 2 && values[0] != GL_SRC_ALPHA && values[1] != GL_ONE_MINUS_SRC_ALPHA)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceSet(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (values[0] != GL_BACK)
            glCullFace(values[0]);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceReset(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (values[0] != GL_BACK)
            glCullFace(GL_BACK);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_CULL_FACE);
}

static CGbool VPVL2CGFXCullFaceEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_CULL_FACE);
}

static CGbool VPVL2CGFXDepthTestEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_DEPTH_TEST);
}

static CGbool VPVL2CGFXDepthTestEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_DEPTH_TEST);
}

static CGbool VPVL2CGFXZWriteEnableSet(CGstateassignment value)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == CG_FALSE)
            glDepthMask(GL_FALSE);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXZWriteEnableReset(CGstateassignment value)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == CG_FALSE)
            glDepthMask(GL_TRUE);
    }
    return CG_TRUE;
}
#endif

}

namespace vpvl2
{

struct Scene::PrivateContext
{
    struct ModelPtr {
        ModelPtr(IModel *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~ModelPtr() {
            if (ownMemory)
                delete value;
        }
        IModel *value;
        int priority;
        bool ownMemory;
    };
    struct MotionPtr {
        MotionPtr(IMotion *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~MotionPtr() {
            if (ownMemory)
                delete value;
        }
        IMotion *value;
        int priority;
        bool ownMemory;
    };
    struct RenderEnginePtr {
        RenderEnginePtr(IRenderEngine *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~RenderEnginePtr() {
            if (ownMemory)
                delete value;
        }
        IRenderEngine *value;
        int priority;
        bool ownMemory;
    };
    template<typename T>
    struct Predication {
        bool operator()(const T *left, const T *right) const {
            return left->priority < right->priority;
        }
    };

    PrivateContext(Scene *sceneRef, bool ownMemory)
        : computeContext(0),
          shadowMapRef(0),
          worldRef(0),
          accelerationType(Scene::kSoftwareFallback),
          effectContext(0),
          light(sceneRef),
          camera(sceneRef),
          preferredFPS(Scene::defaultFPS()),
          ownMemory(ownMemory)
    {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        effectContext = cgCreateContext();
        cgSetParameterSettingMode(effectContext, CG_DEFERRED_PARAMETER_SETTING);
        cgGLSetDebugMode(CG_FALSE);
        cgGLSetManageTextureParameters(effectContext, CG_TRUE);
        cgGLRegisterStates(effectContext);
        /* override state callbacks to override state default parameters */
        CGstate alphaBlendEnableState = cgGetNamedState(effectContext, "AlphaBlendEnable");
        cgSetStateCallbacks(alphaBlendEnableState, VPVL2CGFXAlphaBlendEnableSet, VPVL2CGFXAlphaBlendEnableReset, 0);
        CGstate alphaTestEnableState = cgGetNamedState(effectContext, "AlphaTestEnable");
        cgSetStateCallbacks(alphaTestEnableState, VPVL2CGFXAlphaTestEnableSet, VPVL2CGFXAlphaTestEnableReset, 0);
        CGstate blendFuncState = cgGetNamedState(effectContext, "BlendFunc");
        cgSetStateCallbacks(blendFuncState, VPVL2CGFXBlendFuncSet, VPVL2CGFXBlendFuncReset, 0);
        CGstate cullFaceState = cgGetNamedState(effectContext, "CullFace");
        CGstate cullModeState = cgGetNamedState(effectContext, "CullMode");
        cgSetStateCallbacks(cullFaceState, VPVL2CGFXCullFaceSet, VPVL2CGFXCullFaceReset, 0);
        cgSetStateCallbacks(cullModeState, VPVL2CGFXCullFaceSet, VPVL2CGFXCullFaceReset, 0);
        CGstate cullFaceEnableState = cgGetNamedState(effectContext, "CullFaceEnable");
        cgSetStateCallbacks(cullFaceEnableState, VPVL2CGFXCullFaceEnableSet, VPVL2CGFXCullFaceEnableReset, 0);
        CGstate depthTestEnableState = cgGetNamedState(effectContext, "DepthTestEnable");
        CGstate zenableState = cgGetNamedState(effectContext, "ZEnable");
        cgSetStateCallbacks(depthTestEnableState, VPVL2CGFXDepthTestEnableSet, VPVL2CGFXDepthTestEnableReset, 0);
        cgSetStateCallbacks(zenableState, VPVL2CGFXDepthTestEnableSet, VPVL2CGFXDepthTestEnableReset, 0);
        CGstate zwriteEnableState = cgGetNamedState(effectContext, "ZWriteEnable");
        cgSetStateCallbacks(zwriteEnableState, VPVL2CGFXZWriteEnableSet, VPVL2CGFXZWriteEnableReset, 0);
#endif
    }
    ~PrivateContext() {
        destroyWorld();
        motions.releaseAll();
        engines.releaseAll();
        models.releaseAll();
        shadowMapRef = 0;
        worldRef = 0;
#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
        delete computeContext;
        computeContext = 0;
#endif /* VPVL2_ENABLE_OPENCL */
#ifdef VPVL2_ENABLE_NVIDIA_CG
        cgDestroyContext(effectContext);
        effectContext = 0;
        effectCompilerArguments.releaseAll();
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }

    void addModelPtr(IModel *model, IRenderEngine *engine, int priority) {
        models.append(new ModelPtr(model, priority, ownMemory));
        engines.append(new RenderEnginePtr(engine, priority, ownMemory));
        model2engineRef.insert(model, engine);
        model->joinWorld(worldRef);
    }
    void addMotionPtr(IMotion *motion) {
        motions.append(new MotionPtr(motion, 0, ownMemory));
    }
    void removeModelPtr(IModel *model) {
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            ModelPtr *v = models[i];
            IModel *m = v->value;
            if (m == model) {
                model->leaveWorld(worldRef);
                v->ownMemory = false;
                models.removeAt(i);
                break;
            }
        }
    }
    void removeMotionPtr(IMotion *motion) {
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            MotionPtr *v = motions[i];
            IMotion *m = v->value;
            if (m == motion) {
                v->ownMemory = false;
                motions.removeAt(i);
                break;
            }
        }
    }

    IRenderEngine *removeRenderEnginePtr(IModel *model) {
        const HashPtr key(model);
        IRenderEngine *const *enginePtr = model2engineRef.find(key);
        if (enginePtr) {
            IRenderEngine *engine = *enginePtr;
            const int nengines = engines.count();
            for (int i = 0; i < nengines; i++) {
                RenderEnginePtr *v = engines[i];
                IRenderEngine *e = v->value;
                if (e == engine) {
                    v->ownMemory = false;
                    engines.removeAt(i);
                    break;
                }
            }
            model2engineRef.remove(key);
            return engine;
        }
        return 0;
    }

    void updateMotionState() {
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i]->value;
            model->resetMotionState(worldRef);
        }
        if (worldRef) {
            worldRef->getBroadphase()->resetPool(worldRef->getDispatcher());
            worldRef->getConstraintSolver()->reset();
            worldRef->getForceUpdateAllAabbs();
        }
    }
    void updateRenderEngines() {
        const int nengines = engines.count();
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i]->value;
            engine->update();
        }
    }
    void updateCamera() {
        camera.updateTransform();
    }

    bool isOpenCLAcceleration() const {
        return accelerationType == kOpenCLAccelerationType1 || accelerationType == kOpenCLAccelerationType2;
    }
#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
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
    cl::Context *createComputeContext(IRenderContext *renderContextRef) {
#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
        if (!computeContext) {
            computeContext = new cl::Context(renderContextRef);
            if (!computeContext->initialize(hostDeviceType())) {
                delete computeContext;
                computeContext = 0;
            }
        }
#else
        (void) renderContextRef;
#endif /* VPVL2_ENABLE_OPENCL */
        return computeContext;
    }
    cl::PMXAccelerator *createPMXAccelerator(IRenderContext *renderContext, IModel *modelRef) {
        cl::PMXAccelerator *accelerator = 0;
#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
        if (isOpenCLAcceleration()) {
            if (cl::Context *context = createComputeContext(renderContext)) {
                accelerator = new cl::PMXAccelerator(context, modelRef);
                accelerator->createKernelProgram();
            }
        }
#else
        (void) renderContext;
#endif /* VPVL2_ENABLE_OPENCL */
        return accelerator;
    }
    void getEffectArguments(const IRenderContext *renderContext, Array<const char *> &arguments) {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        effectCompilerArguments.releaseAll();
        renderContext->getEffectCompilerArguments(effectCompilerArguments);
        arguments.clear();
        const int narguments = effectCompilerArguments.count();
        for (int i = 0; i < narguments; i++) {
            if (IString *s = effectCompilerArguments[i])
                arguments.append(reinterpret_cast<const char *>(s->toByteArray()));
        }
        const char constVPVM[] = "-DVPVM";
        arguments.append(constVPVM);
        static const char constVersion[] = "-DVPVL2_VERSION=" VPVL2_VERSION_STRING;
        arguments.append(constVersion);
        arguments.append(0);
#endif
    }
    IEffect *compileEffectFromFile(const IString *pathRef, IRenderContext *renderContextRef) {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        CGeffect effect = 0;
        if (pathRef) {
            Array<const char *> arguments;
            getEffectArguments(renderContextRef, arguments);
            effect = cgCreateEffectFromFile(effectContext,
                                            reinterpret_cast<const char *>(pathRef->toByteArray()),
                                            &arguments[0]);
        }
        return new cg::Effect(renderContextRef, effectContext, cgIsEffect(effect) ? effect : 0);
#else
        return 0;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }
    IEffect *compileEffectFromSource(const IString *source, IRenderContext *renderContextRef) {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        CGeffect effect = 0;
        if (source) {
            Array<const char *> arguments;
            getEffectArguments(renderContextRef, arguments);
            effect = cgCreateEffect(effectContext,
                                    reinterpret_cast<const char *>(source->toByteArray()),
                                    &arguments[0]);
        }
        return new cg::Effect(renderContextRef, effectContext, cgIsEffect(effect) ? effect : 0);
#else
        return 0;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }
    void getModels(Array<IModel *> &value) {
        value.clear();
        int nitems = models.count();
        for (int i = 0; i < nitems; i++) {
            value.append(models[i]->value);
        }
    }
    void getMotions(Array<IMotion *> &value) {
        value.clear();
        int nitems = motions.count();
        for (int i = 0; i < nitems; i++) {
            value.append(motions[i]->value);
        }
    }
    void getRenderEngines(Array<IRenderEngine *> &value) {
        value.clear();
        int nitems = engines.count();
        for (int i = 0; i < nitems; i++) {
            value.append(engines[i]->value);
        }
    }
    void sort() {
        models.sort(Predication<ModelPtr>());
        engines.sort(Predication<RenderEnginePtr>());
    }

    void setWorldRef(btDiscreteDynamicsWorld *world) {
        if (worldRef != world) {
            destroyWorld();
        }
        if (world) {
            int nmodels = models.count();
            for (int i = 0; i < nmodels; i++) {
                ModelPtr *model = models[i];
                if (IModel *m = model->value) {
                    m->joinWorld(world);
                }
            }
        }
        worldRef = world;
    }
    void destroyWorld() {
        if (worldRef) {
            int nmodels = models.count();
            for (int i = 0; i < nmodels; i++) {
                ModelPtr *model = models[i];
                if (IModel *m = model->value) {
                    m->leaveWorld(worldRef);
                }
            }
        }
    }

    cl::Context *computeContext;
    IShadowMap *shadowMapRef;
    btDiscreteDynamicsWorld *worldRef;
    Scene::AccelerationType accelerationType;
    CGcontext effectContext;
    Array<IString *> effectCompilerArguments;
    Hash<HashPtr, IRenderEngine *> model2engineRef;
    Hash<HashString, IModel *> name2modelRef;
    Array<ModelPtr *> models;
    Array<MotionPtr *> motions;
    Array<RenderEnginePtr *> engines;
    Light light;
    Camera camera;
    Scalar preferredFPS;
    bool ownMemory;
};

bool Scene::initialize(void *opaque)
{
    bool ok = true;
#ifdef VPVL2_LINK_GLEW
    if (!g_isGLEWInitialized) {
        GLenum err = glewInit();
        if (GLenum *ptr = static_cast<GLenum *>(opaque)) {
            *ptr = err;
        }
        ok = g_isGLEWInitialized = (err == GLEW_OK);
    }
    /* register default OpenGL states */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
#else
    (void) opaque;
#endif
    return ok;
}

bool Scene::isAcceleratorSupported()
{
#if defined(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
    return true;
#else
    return false;
#endif
}

bool Scene::isSelfShadowSupported()
{
#ifdef VPVL2_LINK_GLEW
    return GLEW_ARB_texture_rg && GLEW_ARB_framebuffer_object && GLEW_ARB_depth_texture;
#else
    return false;
#endif
}

Scalar Scene::defaultFPS()
{
    static const Scalar kDefaultFPS = 30;
    return kDefaultFPS;
}

void Scene::deleteModelUnlessReferred(IModel *model)
{
    if (model && !model->parentSceneRef())
        delete model;
}

void Scene::deleteMotionUnlessReferred(IMotion *motion)
{
    if (motion && !motion->parentSceneRef())
        delete motion;
}

void Scene::deleteRenderEngineUnlessReferred(IRenderEngine *engine)
{
    if (engine && !engine->parentModelRef())
        delete engine;
}

Scene::Scene(bool ownMemory)
    : m_context(0)
{
    m_context = new Scene::PrivateContext(this, ownMemory);
}

Scene::~Scene()
{
    delete m_context;
    m_context = 0;
}

IRenderEngine *Scene::createRenderEngine(IRenderContext *renderContext, IModel *model, int flags)
{
    IRenderEngine *engine = 0;
#ifdef VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT
    if (model) {
        switch (model->type()) {
        case IModel::kAssetModel: {
#ifdef VPVL2_LINK_ASSIMP
            asset::Model *m = static_cast<asset::Model *>(model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
            if (flags & kEffectCapable)
                engine = new cg::AssetRenderEngine(renderContext, this, m);
            else
#endif /* VPVL2_ENABLE_NVIDIA_CG */
                engine = new gl2::AssetRenderEngine(renderContext, this, m);
#endif /* VPVL2_LINK_ASSIMP */
            break;
        }
        case IModel::kPMDModel:
        case IModel::kPMXModel: {
            cl::PMXAccelerator *accelerator = m_context->createPMXAccelerator(renderContext, model);
#ifdef VPVL2_ENABLE_NVIDIA_CG
            if (flags & kEffectCapable)
                engine = new cg::PMXRenderEngine(renderContext, this, accelerator, model);
            else
#endif /* VPVL2_ENABLE_NVIDIA_CG */
                engine = new gl2::PMXRenderEngine(renderContext, this, accelerator, model);
            break;
        }
        default:
            break;
        }
#ifndef VPVL2_ENABLE_NVIDIA_CG
        (void) flags;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    }
#else
    (void) renderContext;
    (void) model;
    (void) flags;
#endif /* VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT */
    return engine;
}

void Scene::addModel(IModel *model, IRenderEngine *engine, int priority)
{
    if (model && engine) {
        m_context->addModelPtr(model, engine, priority);
        VPVL2SceneSetParentSceneRef(model, this);
        if (const IString *name = model->name()) {
            m_context->name2modelRef.insert(name->toHashString(), model);
        }
    }
}

void Scene::addMotion(IMotion *motion)
{
    if (motion) {
        m_context->addMotionPtr(motion);
        VPVL2SceneSetParentSceneRef(motion, this);
    }
}

ICamera *Scene::createCamera()
{
    return new Camera(this);
}

ILight *Scene::createLight()
{
    return new Light(this);
}

IEffect *Scene::createEffectFromSource(const IString *source, IRenderContext *renderContext)
{
#ifdef VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT
    return m_context->compileEffectFromSource(source, renderContext);
#else
    (void) source;
    (void) renderContext;
    return 0;
#endif /* VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT */
}

IEffect *Scene::createEffectFromFile(const IString *path, IRenderContext *renderContext)
{
#ifdef VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT
    return m_context->compileEffectFromFile(path, renderContext);
#else
    (void) path;
    (void) renderContext;
    return 0;
#endif /* VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT */
}

IEffect *Scene::createDefaultStandardEffect(IRenderContext *renderContext)
{
#ifdef VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT
    IString *source = renderContext->loadShaderSource(IRenderContext::kModelEffectTechniques, 0);
    IEffect *effect = m_context->compileEffectFromSource(source, renderContext);
    delete source;
    return effect;
#else
    (void) renderContext;
    return 0;
#endif
}

IEffect *Scene::createEffectFromModel(const IModel *model, const IString *dir, IRenderContext *renderContext)
{
#ifdef VPVL2_ENABLE_NVIDIA_CG
    const IString *pathRef = renderContext->effectFilePath(model, dir);
    return m_context->compileEffectFromFile(pathRef, renderContext);
#else
    (void) dir;
    (void) model;
    (void) renderContext;
    return 0;
#endif /* VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT */
}

void Scene::removeModel(IModel *model)
{
    if (model) {
        m_context->removeRenderEnginePtr(model);
        m_context->removeModelPtr(model);
        VPVL2SceneSetParentSceneRef(model, 0);
    }
}

void Scene::deleteModel(IModel *&model)
{
    IRenderEngine *engine = m_context->removeRenderEnginePtr(model);
    removeModel(model);
    if (m_context->ownMemory) {
        delete engine;
        delete model;
    }
    model = 0;
}

void Scene::removeMotion(IMotion *motion)
{
    if (motion) {
        m_context->removeMotionPtr(motion);
        VPVL2SceneSetParentSceneRef(motion, 0);
    }
}

void Scene::deleteMotion(IMotion *&motion)
{
    removeMotion(motion);
    if (m_context->ownMemory) {
        delete motion;
    }
    motion = 0;
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
    if (flags & kResetMotionState) {
        m_context->updateMotionState();
    }
    if (flags & kUpdateModels) {
        const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i]->value;
            motion->advance(delta);
        }
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
        const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i]->value;
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
    /* resolve motion state first to call RigidBody#setKinematic before RigidBody#performUpdate */
    if (flags & kResetMotionState) {
        m_context->updateMotionState();
    }
    if (flags & kUpdateRenderEngines) {
        m_context->updateRenderEngines();
    }
}

void Scene::getRenderEnginesByRenderOrder(Array<IRenderEngine *> &enginesForPreProcess,
                                          Array<IRenderEngine *> &enginesForStandard,
                                          Array<IRenderEngine *> &enginesForPostProcess,
                                          Hash<HashPtr, IEffect *> &nextPostEffects) const
{
    enginesForPreProcess.clear();
    enginesForStandard.clear();
    enginesForPostProcess.clear();
    nextPostEffects.clear();
    const Array<PrivateContext::RenderEnginePtr *> &engines = m_context->engines;
    const int nengines = engines.count();
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i]->value;
        if (engine->effect(IEffect::kPreProcess)) {
            enginesForPreProcess.append(engine);
        }
        else if (engine->effect(IEffect::kPostProcess)) {
            enginesForPostProcess.append(engine);
        }
        else {
            enginesForStandard.append(engine);
        }
    }
    IEffect *nextPostEffectRef = 0;
    nextPostEffects.clear();
    for (int i = enginesForPostProcess.count() - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *effect = engine->effect(IEffect::kPostProcess);
        nextPostEffects.insert(engine, nextPostEffectRef);
        nextPostEffectRef = effect;
    }
}

void Scene::setPreferredFPS(const Scalar &value)
{
    m_context->preferredFPS = value;
}

bool Scene::isReachedTo(const IKeyframe::TimeIndex &timeIndex) const
{
    const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i]->value;
        if (!motion->isReachedTo(timeIndex)) {
            return false;
        }
    }
    return true;
}

IKeyframe::TimeIndex Scene::maxTimeIndex() const
{
    const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
    const int nmotions = motions.count();
    IKeyframe::TimeIndex maxTimeIndex = 0;
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i]->value;
        btSetMax(maxTimeIndex, motion->maxTimeIndex());
    }
    return maxTimeIndex;
}

void Scene::getModelRefs(Array<IModel *> &value) const
{
    m_context->getModels(value);
}

void Scene::getMotionRefs(Array<IMotion *> &value) const
{
    m_context->getMotions(value);
}

void Scene::getRenderEngineRefs(Array<IRenderEngine *> &value) const
{
    m_context->getRenderEngines(value);
}

IModel *Scene::findModel(const IString *name) const
{
    if (name) {
        IModel *const *model = m_context->name2modelRef.find(name->toHashString());
        return model ? *model : 0;
    }
    return 0;
}

IRenderEngine *Scene::findRenderEngine(const IModel *model) const
{
    IRenderEngine *const *engine = m_context->model2engineRef.find(model);
    return engine ? *engine : 0;
}

void Scene::sort()
{
    m_context->sort();
}

ILight *Scene::light() const
{
    return &m_context->light;
}

ICamera *Scene::camera() const
{
    return &m_context->camera;
}

IShadowMap *Scene::shadowMapRef() const
{
    return m_context->shadowMapRef;
}

void Scene::setShadowMapRef(IShadowMap *value)
{
    m_context->shadowMapRef = value;
}

Scalar Scene::preferredFPS() const
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

void Scene::setWorldRef(btDiscreteDynamicsWorld *worldRef)
{
    m_context->setWorldRef(worldRef);
}

}

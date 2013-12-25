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

#include "vpvl2/vpvl2.h"
#include "vpvl2/IApplicationContext.h"
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

#ifdef VPVL2_ENABLE_OPENGL
#ifdef VPVL2_OS_OSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#ifdef VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/fx/AssetRenderEngine.h"
#include "vpvl2/fx/PMXRenderEngine.h"
#endif /* VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT */

#ifdef VPVL2_ENABLE_NVIDIA_CG
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/EffectContext.h"
#endif /* VPVL2_ENABLE_NVIDIA_CG */
#ifdef VPVL2_LINK_NVFX
#include "vpvl2/nvfx/Effect.h"
#include "vpvl2/nvfx/EffectContext.h"
#endif

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/ConstraintSolver/btConstraintSolver.h>

#if defined(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
#include "vpvl2/cl/PMXAccelerator.h"
#else
namespace vpvl2 {
namespace cl {
class PMXAccelerator;
}
}
#endif /* VPVL2_ENABLE_OPENCL */

#if defined(VPVL2_OS_WINDOWS)
/* wglGetCurrentContext and wglGetCurrentDC */
#include <windows.h>
#elif defined(VPVL2_OS_OSX)
/* for CGLGetCurrentContext and CGLGetShareGroup */
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLDevice.h>
#elif defined(VPVL2_HAS_OPENGL_GLX)
/* for glXGetCurrentContext and glXGetCurrentDisplay */
#include <GL/glx.h>
#endif

#ifdef VPVL2_LINK_EGL
#include <EGL/egl.h>
#endif

#ifdef VPVL2_LINK_REGAL
#include <GL/Regal.h>
#else
#define RegalSetErrorCallback(callback)
#define RegalMakeCurrent(ctx)
#define RegalDestroyContext(ctx)
#ifndef VPVL2_ENABLE_OPENGL
typedef int GLenum;
#endif
#endif /* VPVL2_LINK_REGAL */

namespace
{

using namespace vpvl2;

VPVL2_DECL_TLS static bool g_initialized = false;

static void VPVL2SceneSetParentSceneRef(IModel *model, Scene *scene) VPVL2_DECL_NOEXCEPT
{
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

static void VPVL2SceneSetParentSceneRef(IMotion *motion, Scene *scene) VPVL2_DECL_NOEXCEPT
{
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

class Light VPVL2_DECL_FINAL : public ILight {
public:
    Light(Scene *sceneRef) :
        m_sceneRef(sceneRef),
        m_motion(0),
        m_color(kZeroV3),
        m_direction(kZeroV3),
        m_enableToon(false)
    {
        resetDefault();
    }
    ~Light() {
        internal::deleteObject(m_motion);
        m_enableToon = false;
        m_color.setZero();
        m_direction.setZero();
    }

    void addEventListenerRef(PropertyEventListener *value) {
        m_eventRefs.append(value);
    }
    void removeEventListenerRef(PropertyEventListener *value) {
        m_eventRefs.remove(value);
    }
    void getEventListenerRefs(Array<PropertyEventListener *> &value) {
        value.copy(m_eventRefs);
    }
    Vector3 color() const VPVL2_DECL_NOEXCEPT { return m_color; }
    Vector3 direction() const VPVL2_DECL_NOEXCEPT { return m_direction; }
    bool isToonEnabled() const VPVL2_DECL_NOEXCEPT { return m_enableToon; }
    IMotion *motion() const VPVL2_DECL_NOEXCEPT { return m_motion; }
    void setColor(const Vector3 &value) VPVL2_DECL_NOEXCEPT {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, colorWillChange(value, this));
        m_color = value;
    }
    void setDirection(const Vector3 &value) VPVL2_DECL_NOEXCEPT {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, directionWillChange(value, this));
        m_direction = value;
    }
    void setToonEnable(bool value) VPVL2_DECL_NOEXCEPT {
        m_enableToon = value;
    }
    void copyFrom(const ILight *value) VPVL2_DECL_NOEXCEPT {
        setColor(value->color());
        setDirection(value->direction());
        setToonEnable(value->isToonEnabled());
    }
    void resetDefault() VPVL2_DECL_NOEXCEPT {
        setColor(Vector3(0.6f, 0.6f, 0.6f));
        setDirection(Vector3(-0.5f, -1.0f, -0.5f));
        setToonEnable(false);
    }
    void setMotion(IMotion *value) VPVL2_DECL_NOEXCEPT {
        VPVL2SceneSetParentSceneRef(m_motion, 0);
        m_motion = value;
        VPVL2SceneSetParentSceneRef(value, m_sceneRef);
    }

private:
    Scene *m_sceneRef;
    IMotion *m_motion;
    Array<PropertyEventListener *> m_eventRefs;
    Vector3 m_color;
    Vector3 m_direction;
    bool m_enableToon;
};

class Camera VPVL2_DECL_FINAL : public ICamera {
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
          m_zfar(10000.0f)
    {
        resetDefault();
    }
    ~Camera() {
        internal::deleteObject(m_motion);
        m_transform.setIdentity();
        m_lookAt.setZero();
        m_position.setZero();
        m_angle.setZero();
        m_distance.setZero();
        m_fov = 0;
        m_znear = 0;
        m_zfar = 0;
    }

    void addEventListenerRef(PropertyEventListener *value) {
        m_eventRefs.append(value);
    }
    void removeEventListenerRef(PropertyEventListener *value) {
        m_eventRefs.remove(value);
    }
    void getEventListenerRefs(Array<PropertyEventListener *> &value) {
        value.copy(m_eventRefs);
    }
    Transform modelViewTransform() const VPVL2_DECL_NOEXCEPT { return m_transform; }
    Vector3 lookAt() const VPVL2_DECL_NOEXCEPT { return m_lookAt; }
    Vector3 position() const VPVL2_DECL_NOEXCEPT { return m_position; }
    Vector3 angle() const VPVL2_DECL_NOEXCEPT { return m_angle; }
    Scalar fov() const VPVL2_DECL_NOEXCEPT { return m_fov; }
    Scalar distance() const VPVL2_DECL_NOEXCEPT { return m_distance.z(); }
    Scalar znear() const VPVL2_DECL_NOEXCEPT { return m_znear; }
    Scalar zfar() const VPVL2_DECL_NOEXCEPT { return m_zfar; }
    IMotion *motion() const VPVL2_DECL_NOEXCEPT { return m_motion; }
    void setLookAt(const Vector3 &value) VPVL2_DECL_NOEXCEPT { m_lookAt = value; }
    void setAngle(const Vector3 &value) VPVL2_DECL_NOEXCEPT { m_angle = value; }
    void setFov(Scalar value) VPVL2_DECL_NOEXCEPT { m_fov = value; }
    void setZNear(Scalar value) VPVL2_DECL_NOEXCEPT { m_znear = value; }
    void setZFar(Scalar value) VPVL2_DECL_NOEXCEPT { m_zfar = value; }
    void setDistance(Scalar value) VPVL2_DECL_NOEXCEPT {
        m_distance.setZ(value);
        m_position = m_lookAt + m_distance;
    }
    void copyFrom(const ICamera *value) VPVL2_DECL_NOEXCEPT {
        setLookAt(value->lookAt());
        setAngle(value->angle());
        setFov(value->fov());
        setDistance(value->distance());
        setZNear(value->znear());
        setZFar(value->zfar());
    }
    void resetDefault() VPVL2_DECL_NOEXCEPT {
        setLookAt(Vector3(0, 10, 0));
        setAngle(kZeroV3);
        setFov(27);
        setDistance(50);
        updateTransform();
    }
    void setMotion(IMotion *value) VPVL2_DECL_NOEXCEPT {
        VPVL2SceneSetParentSceneRef(m_motion, 0);
        m_motion = value;
        VPVL2SceneSetParentSceneRef(value, m_sceneRef);
    }

    void updateTransform() VPVL2_DECL_NOEXCEPT {
        const Quaternion rotationX(kUnitX, btRadians(m_angle.x())),
                rotationY(kUnitY, btRadians(m_angle.y())),
                rotationZ(kUnitZ, btRadians(m_angle.z()));
        m_transform.setIdentity();
        m_transform.setRotation(rotationZ * rotationX * rotationY);
        m_transform.setOrigin((m_transform * -m_lookAt) - m_distance);
    }

private:
    Scene *m_sceneRef;
    IMotion *m_motion;
    Array<PropertyEventListener *> m_eventRefs;
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

} /* namespace anonymous */

namespace vpvl2
{

struct Scene::PrivateContext VPVL2_DECL_FINAL
{
    struct ModelPtr VPVL2_DECL_FINAL {
        ModelPtr(IModel *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~ModelPtr() {
            if (ownMemory) {
                internal::deleteObject(value);
            }
        }
        IModel *value;
        int priority;
        bool ownMemory;
    };
    struct MotionPtr VPVL2_DECL_FINAL {
        MotionPtr(IMotion *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~MotionPtr() {
            if (ownMemory) {
                internal::deleteObject(value);
            }
        }
        IMotion *value;
        int priority;
        bool ownMemory;
    };
    struct RenderEnginePtr VPVL2_DECL_FINAL {
        RenderEnginePtr(IRenderEngine *v, int p, bool o)
            : value(v),
              priority(p),
              ownMemory(o)
        {
        }
        ~RenderEnginePtr() {
            if (ownMemory) {
                internal::deleteObject(value);
            }
        }
        IRenderEngine *value;
        int priority;
        bool ownMemory;
    };
    template<typename T>
    struct Predication VPVL2_DECL_FINAL {
        bool operator()(const T *left, const T *right) const {
            return left->priority < right->priority;
        }
    };

    static void handleRegalErrorCallback(GLenum error) {
        (void) error;
        VPVL2_LOG(WARNING, "error=" << error);
    }

    PrivateContext(Scene *sceneRef, bool ownMemory)
        : shadowMapRef(0),
          worldRef(0),
          accelerationType(Scene::kSoftwareFallback),
          defaultEffect(0),
          light(sceneRef),
          camera(sceneRef),
          currentTimeIndex(0),
          preferredFPS(Scene::defaultFPS()),
          ownMemory(ownMemory)
    {
    }
    ~PrivateContext() {
        destroyWorld();
        releaseAllRenderEngines();
        motions.releaseAll();
        engines.releaseAll();
        models.releaseAll();
        internal::deleteObject(defaultEffect);
        shadowMapRef = 0;
        worldRef = 0;
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
    IEffect *createEffectFromFile(const IString *path, IApplicationContext *applicationContextRef) {
        (void) path;
        (void) applicationContextRef;
        IEffect *effectRef = 0;
#ifdef VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT
#ifdef VPVL2_LINK_NVFX
        effectRef = effectContextNvFX.compileFromFile(path, applicationContextRef);
#endif /* VPVL2_LINK_NVFX */
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (!effectRef) {
            effectRef = effectContextCgFX.compileFromFile(path, applicationContextRef);
        }
#endif /* VPVL2_ENABLE_NVIDIA_CG */
#endif /* VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT */
        return effectRef;
    }
    IEffect *createEffectFromSource(const IString *source, IApplicationContext *applicationContextRef) {
        (void) source;
        (void) applicationContextRef;
        IEffect *effectRef = 0;
#ifdef VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT
#ifdef VPVL2_LINK_NVFX
        effectRef = effectContextNvFX.compileFromSource(source, applicationContextRef);
#endif /* VPVL2_LINK_NVFX */
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (!effectRef) {
            effectRef = effectContextCgFX.compileFromSource(source, applicationContextRef);
        }
#endif /* VPVL2_ENABLE_NVIDIA_CG */
#endif /* VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT */
        return effectRef;
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
    void releaseAllRenderEngines() {
        const int nengines = engines.count();
        for (int i = 0; i < nengines; i++) {
            RenderEnginePtr *v = engines[i];
            IRenderEngine *e = v->value;
            e->release();
        }
    }

    void resetMotionState() {
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i]->value;
            model->resetMotionState(worldRef);
        }
        if (worldRef) {
            worldRef->getBroadphase()->resetPool(worldRef->getDispatcher());
            worldRef->getConstraintSolver()->reset();
        }
    }
    void updateModels() {
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i]->value;
            model->performUpdate();
        }
    }
    void markAllMorphsDirty() {
        Array<IMorph *> morphs;
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i]->value;
            morphs.clear();
            model->getMorphRefs(morphs);
            const int nmorphs = morphs.count();
            for (int j = 0; j < nmorphs; j++) {
                IMorph *morph = morphs[j];
                morph->markDirty();
            }
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

    bool isOpenCLAcceleration() const VPVL2_DECL_NOEXCEPT {
        return accelerationType == kOpenCLAccelerationType1 || accelerationType == kOpenCLAccelerationType2;
    }
    cl::PMXAccelerator *createPMXAccelerator(const Scene *sceneRef, IApplicationContext *applicationContextRef, IModel *modelRef) {
        cl::PMXAccelerator *accelerator = 0;
#if defined(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
        if (isOpenCLAcceleration()) {
            accelerator = new cl::PMXAccelerator(sceneRef, applicationContextRef, modelRef, accelerationType);
        }
#else
        (void) sceneRef;
        (void) applicationContextRef;
        (void) modelRef;
#endif /* VPVL2_ENABLE_OPENCL */
        return accelerator;
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

    IShadowMap *shadowMapRef;
    btDiscreteDynamicsWorld *worldRef;
    Scene::AccelerationType accelerationType;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    cg::EffectContext effectContextCgFX;
#endif
#ifdef VPVL2_LINK_NVFX
    nvfx::EffectContext effectContextNvFX;
#endif
    Hash<HashPtr, IRenderEngine *> model2engineRef;
    Hash<HashString, IModel *> name2modelRef;
    Array<ModelPtr *> models;
    Array<MotionPtr *> motions;
    Array<RenderEnginePtr *> engines;
    IEffect *defaultEffect;
    Light light;
    Camera camera;
    IKeyframe::TimeIndex currentTimeIndex;
    Scalar preferredFPS;
    bool ownMemory;
};

bool Scene::initialize(void *opaque)
{
    bool ok = true;
    if (!g_initialized) {
        RegalMakeCurrent(Scene::opaqueCurrentPlatformOpenGLContext());
        RegalSetErrorCallback(&Scene::PrivateContext::handleRegalErrorCallback);
#ifdef VPVL2_LINK_NVFX
        const IApplicationContext::FunctionResolver *resolver = static_cast<const IApplicationContext::FunctionResolver *>(opaque);
        nvfx::EffectContext::initializeGLEW(resolver);
#else
        (void) opaque;
#endif
        ok = g_initialized = true;
        if (ok) {
            setRequiredOpenGLState();
        }
    }
    return ok;
}

bool Scene::isInitialized() VPVL2_DECL_NOEXCEPT
{
    return g_initialized;
}

void Scene::setRequiredOpenGLState() VPVL2_DECL_NOEXCEPT
{
#ifdef VPVL2_ENABLE_OPENGL
    /* register default OpenGL states */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
#endif /* VPVL2_ENABLE_OPENGL */
}

void Scene::terminate()
{
    if (g_initialized) {
        RegalDestroyContext(Scene::opaqueCurrentPlatformOpenGLContext());
        g_initialized = false;
    }
}

void *Scene::opaqueCurrentPlatformOpenGLContext() VPVL2_DECL_NOEXCEPT
{
#if !defined(VPVL2_ENABLE_OPENGL)
    return 0;
#elif defined(VPVL2_LINK_EGL)
    return ::eglGetCurrentContext();
#elif defined(VPVL2_OS_WINDOWS)
    return ::wglGetCurrentContext();
#elif defined(VPVL2_OS_OSX)
    return ::CGLGetCurrentContext();
#elif defined(VPVL2_HAS_OPENGL_GLX)
    return ::glXGetCurrentContext();
#else
    return 0;
#endif
}

void *Scene::opaqueCurrentPlatformOpenGLDevice() VPVL2_DECL_NOEXCEPT
{
#if !defined(VPVL2_ENABLE_OPENGL)
    return 0;
#elif defined(VPVL2_LINK_EGL)
    return ::eglGetCurrentDisplay();
#elif defined(VPVL2_OS_WINDOWS)
    return ::wglGetCurrentDC();
#elif defined(VPVL2_OS_OSX)
    return ::CGLGetShareGroup(::CGLGetCurrentContext());
#elif defined(VPVL2_HAS_OPENGL_GLX)
    return ::glXGetCurrentDisplay();
#else
    return 0;
#endif
}

bool Scene::isAcceleratorSupported() VPVL2_DECL_NOEXCEPT
{
#if defined(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT) && defined(VPVL2_ENABLE_OPENCL)
    return true;
#else
    return false;
#endif
}

Scalar Scene::defaultFPS() VPVL2_DECL_NOEXCEPT
{
    static const Scalar kDefaultFPS = 30;
    return kDefaultFPS;
}

void Scene::deleteModelUnlessReferred(IModel *model)
{
    if (model && !model->parentSceneRef()) {
        internal::deleteObject(model);
    }
}

void Scene::deleteMotionUnlessReferred(IMotion *motion)
{
    if (motion && !motion->parentSceneRef()) {
        internal::deleteObject(motion);
    }
}

void Scene::deleteRenderEngineUnlessReferred(IRenderEngine *engine)
{
    if (engine && !engine->parentModelRef()) {
        internal::deleteObject(engine);
    }
}

Scene::Scene(bool ownMemory)
    : m_context(new PrivateContext(this, ownMemory))
{
}

Scene::~Scene()
{
    internal::deleteObject(m_context);
}

IRenderEngine *Scene::createRenderEngine(IApplicationContext *applicationContextRef, IModel *model, int flags)
{
    VPVL2_CHECK(applicationContextRef);
    IRenderEngine *engine = 0;
#ifdef VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT
    if (model) {
        switch (model->type()) {
        case IModel::kAssetModel: {
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
            asset::Model *m = static_cast<asset::Model *>(model);
#if defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX)
            if (internal::hasFlagBits(flags, kEffectCapable)) {
                engine = new fx::AssetRenderEngine(applicationContextRef, this, m);
            }
            else
#endif /* defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX) */
                engine = new gl2::AssetRenderEngine(applicationContextRef, this, m);
#endif /* VPVL2_LINK_ASSIMP */
            break;
        }
        case IModel::kPMDModel:
        case IModel::kPMXModel: {
            cl::PMXAccelerator *accelerator = m_context->createPMXAccelerator(this, applicationContextRef, model);
#if defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX)
            if (internal::hasFlagBits(flags, kEffectCapable)) {
                engine = new fx::PMXRenderEngine(applicationContextRef, this, accelerator, model);
            }
            else
#endif /* defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX) */
                engine = new gl2::PMXRenderEngine(applicationContextRef, this, accelerator, model);
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
    (void) applicationContextRef;
    (void) model;
    (void) flags;
#endif /* VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT */
    return engine;
}

void Scene::addModel(IModel *model, IRenderEngine *engine, int priority)
{
    if (model && engine) {
        m_context->addModelPtr(model, engine, priority);
        VPVL2SceneSetParentSceneRef(model, this);
        if (const IString *name = model->name(IEncoding::kDefaultLanguage)) {
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

IEffect *Scene::createEffectFromSource(const IString *source, IApplicationContext *applicationContextRef)
{
    VPVL2_CHECK(source);
    VPVL2_CHECK(applicationContextRef);
    return m_context->createEffectFromSource(source, applicationContextRef);
}

IEffect *Scene::createEffectFromFile(const IString *path, IApplicationContext *applicationContextRef)
{
    VPVL2_CHECK(path);
    VPVL2_CHECK(applicationContextRef);
    return m_context->createEffectFromFile(path, applicationContextRef);
}

IEffect *Scene::createDefaultStandardEffectRef(IApplicationContext *applicationContextRef)
{
    IEffect *effectRef = m_context->defaultEffect;
    if (!effectRef) {
        VPVL2_CHECK(applicationContextRef);
        IString *source = applicationContextRef->loadShaderSource(IApplicationContext::kModelEffectTechniques, 0);
        VPVL2_DCHECK(source && source->size() > 0);
        m_context->defaultEffect = effectRef = m_context->createEffectFromSource(source, applicationContextRef);
    }
    return effectRef;
}

IEffect *Scene::createEffectFromModel(const IModel *model, const IString *dir, IApplicationContext *applicationContextRef)
{
    VPVL2_CHECK(model);
    VPVL2_CHECK(dir);
    VPVL2_CHECK(applicationContextRef);
#ifdef VPVL2_ENABLE_NVIDIA_CG
    const IString *pathRef = applicationContextRef->effectFilePath(model, dir);
    return m_context->createEffectFromFile(pathRef, applicationContextRef);
#else
    (void) model;
    (void) dir;
    (void) applicationContextRef;
    return 0;
#endif
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
        internal::deleteObject(engine);
        internal::deleteObject(model);
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
        internal::deleteObject(motion);
    }
    motion = 0;
}

void Scene::advance(const IKeyframe::TimeIndex &delta, int flags)
{
    if (internal::hasFlagBits(flags, kUpdateCamera)) {
        Camera &camera = m_context->camera;
        IMotion *cameraMotion = camera.motion();
        if (cameraMotion) {
            cameraMotion->advanceScene(delta, this);
        }
    }
    if (internal::hasFlagBits(flags, kUpdateLight)) {
        Light &light = m_context->light;
        IMotion *lightMotion = light.motion();
        if (lightMotion) {
            lightMotion->advanceScene(delta, this);
        }
    }
    if (internal::hasFlagBits(flags, kResetMotionState)) {
        m_context->resetMotionState();
    }
    if (internal::hasFlagBits(flags, kForceUpdateAllMorphs)) {
        m_context->markAllMorphsDirty();
    }
    if (internal::hasFlagBits(flags, kUpdateModels)) {
        const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i]->value;
            motion->advance(delta);
        }
    }
    m_context->currentTimeIndex += delta;
}

void Scene::seek(const IKeyframe::TimeIndex &timeIndex, int flags)
{
    if (internal::hasFlagBits(flags, kUpdateCamera)) {
        Camera &camera = m_context->camera;
        IMotion *cameraMotion = camera.motion();
        if (cameraMotion) {
            cameraMotion->seekScene(timeIndex, this);
        }
    }
    if (internal::hasFlagBits(flags, kUpdateLight)) {
        Light &light = m_context->light;
        IMotion *lightMotion = light.motion();
        if (lightMotion) {
            lightMotion->seekScene(timeIndex, this);
        }
    }
    if (internal::hasFlagBits(flags, kUpdateModels)) {
        const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i]->value;
            motion->seek(timeIndex);
        }
    }
    m_context->currentTimeIndex = timeIndex;
}

void Scene::updateModel(IModel *model) const
{
    if (model) {
        model->performUpdate();
        if (IRenderEngine *engine = findRenderEngine(model)) {
            engine->update();
        }
    }
}

void Scene::update(int flags)
{
    if (internal::hasFlagBits(flags, kUpdateCamera)) {
        m_context->updateCamera();
    }
    if (internal::hasFlagBits(flags, kForceUpdateAllMorphs)) {
        m_context->markAllMorphsDirty();
    }
    if (internal::hasFlagBits(flags, kUpdateModels)) {
        m_context->updateModels();
    }
    /*
     * Call updateMotionAfter after #updateModels() to resolve dependency
     * (get position from motion state) of Bone's world transform.
     */
    if (internal::hasFlagBits(flags, kResetMotionState)) {
        m_context->resetMotionState();
    }
    /*
     * Call updateRenderEngines after #update(Models|MotionState) to get skinned position.
     * #updateModels() performs transforming position to skinned position by the model's bones.
     */
    if (internal::hasFlagBits(flags, kUpdateRenderEngines)) {
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
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (engine->effectRef(IEffect::kPreProcess)) {
            enginesForPreProcess.append(engine);
        }
        else if (engine->effectRef(IEffect::kPostProcess)) {
            enginesForPostProcess.append(engine);
        }
        else {
            enginesForStandard.append(engine);
        }
#else
        enginesForPreProcess.append(engine);
        enginesForPostProcess.append(engine);
        enginesForStandard.append(engine);
#endif
    }
    IEffect *nextPostEffectRef = 0;
    nextPostEffects.clear();
    for (int i = enginesForPostProcess.count() - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *effect = engine->effectRef(IEffect::kPostProcess);
        nextPostEffects.insert(engine, nextPostEffectRef);
        nextPostEffectRef = effect;
    }
}

void Scene::reset()
{
    bool ownMemory = m_context->ownMemory;
    internal::deleteObject(m_context);
    m_context = new PrivateContext(this, ownMemory);
}

void Scene::setPreferredFPS(const Scalar &value) VPVL2_DECL_NOEXCEPT
{
    m_context->preferredFPS = value;
}

bool Scene::isReachedTo(const IKeyframe::TimeIndex &timeIndex) const VPVL2_DECL_NOEXCEPT
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

IKeyframe::TimeIndex Scene::duration() const VPVL2_DECL_NOEXCEPT
{
    const Array<PrivateContext::MotionPtr *> &motions = m_context->motions;
    const int nmotions = motions.count();
    IKeyframe::TimeIndex maxTimeIndex = 0;
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i]->value;
        btSetMax(maxTimeIndex, motion->duration());
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

IModel *Scene::findModel(const IString *name) const VPVL2_DECL_NOEXCEPT
{
    if (name) {
        IModel *const *model = m_context->name2modelRef.find(name->toHashString());
        return model ? *model : 0;
    }
    return 0;
}

IRenderEngine *Scene::findRenderEngine(const IModel *model) const VPVL2_DECL_NOEXCEPT
{
    IRenderEngine *const *engine = m_context->model2engineRef.find(model);
    return engine ? *engine : 0;
}

void Scene::sort()
{
    m_context->sort();
}

IKeyframe::TimeIndex Scene::currentTimeIndex() const VPVL2_DECL_NOEXCEPT
{
    return m_context->currentTimeIndex;
}

ILight *Scene::lightRef() const VPVL2_DECL_NOEXCEPT
{
    return &m_context->light;
}

ICamera *Scene::cameraRef() const VPVL2_DECL_NOEXCEPT
{
    return &m_context->camera;
}

IShadowMap *Scene::shadowMapRef() const VPVL2_DECL_NOEXCEPT
{
    return m_context->shadowMapRef;
}

void Scene::setShadowMapRef(IShadowMap *value) VPVL2_DECL_NOEXCEPT
{
    m_context->shadowMapRef = value;
}

Scalar Scene::preferredFPS() const VPVL2_DECL_NOEXCEPT
{
    return m_context->preferredFPS;
}

Scene::AccelerationType Scene::accelerationType() const VPVL2_DECL_NOEXCEPT
{
    return m_context->accelerationType;
}

void Scene::setAccelerationType(AccelerationType value) VPVL2_DECL_NOEXCEPT
{
    m_context->accelerationType = value;
}

void Scene::setWorldRef(btDiscreteDynamicsWorld *worldRef) VPVL2_DECL_NOEXCEPT
{
    m_context->setWorldRef(worldRef);
}

} /* namespace vpvl2 */

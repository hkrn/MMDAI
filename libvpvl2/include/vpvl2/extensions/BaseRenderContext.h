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

#pragma once
#ifndef VPVL2_EXTENSIONS_BASERENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_BASERENDERCONTEXT_H_

/* libvpvl2 */
#include "vpvl2/vpvl2.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/extensions/gl/FrameBufferObject.h"
#include "vpvl2/extensions/icu/Encoding.h"

/* Bullet Physics */
#ifndef VPVL2_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif /* VPVL2_NO_BULLET */

/* STL */
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>

/* Open Asset Import Library */
#ifdef VPVL2_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
BT_DECLARE_HANDLE(aiScene);
#endif /* VPVL2_LINK_ASSIMP */

/* GLM */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* NVTT */
#ifdef VPVL2_LINK_NVTT
#include <nvcore/Stream.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
#endif

namespace vpvl2
{
namespace extensions
{
using namespace icu;

static const uint8_t *UICastData(const std::string &data)
{
    return reinterpret_cast<const uint8_t *>(data.c_str());
}

typedef std::map<UnicodeString, UnicodeString> UIStringMap;

class World {
public:
    World()
        : m_dispatcher(0),
          m_broadphase(0),
          m_solver(0),
          m_world(0),
          m_motionFPS(0),
          m_fixedTimeStep(0),
          m_maxSubSteps(0)
    {
        m_dispatcher = new btCollisionDispatcher(&m_config);
        m_broadphase = new btDbvtBroadphase();
        m_solver = new btSequentialImpulseConstraintSolver();
        m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, &m_config);
        setGravity(vpvl2::Vector3(0.0f, -9.8f, 0.0f));
        setPreferredFPS(vpvl2::Scene::defaultFPS());
    }
    ~World() {
        const int nmodels = m_modelRefs.count();
        for (int i = 0; i < nmodels; i++) {
            removeModel(m_modelRefs[i]);
        }
        delete m_dispatcher;
        m_dispatcher = 0;
        delete m_broadphase;
        m_broadphase = 0;
        delete m_solver;
        m_solver = 0;
        delete m_world;
        m_world = 0;
        m_motionFPS = 0;
        m_maxSubSteps = 0;
        m_fixedTimeStep = 0;
    }
    const vpvl2::Vector3 gravity() const { return m_world->getGravity(); }
    void setGravity(const vpvl2::Vector3 &value) { m_world->setGravity(value); }
    unsigned long randSeed() const { return m_solver->getRandSeed(); }
    void setRandSeed(unsigned long value) { m_solver->setRandSeed(value); }
    void setPreferredFPS(const vpvl2::Scalar &value) {
        m_motionFPS = value;
        m_maxSubSteps = btMax(int(60 / m_motionFPS), 1);
        m_fixedTimeStep = 1.0f / value;
    }
    void addModel(vpvl2::IModel *value) {
        value->joinWorld(m_world);
        m_modelRefs.add(value);
    }
    void removeModel(vpvl2::IModel *value) {
        value->leaveWorld(m_world);
        m_modelRefs.remove(value);
    }
    void addRigidBody(btRigidBody *value) {
        m_world->addRigidBody(value);
    }
    void removeRigidBody(btRigidBody *value) {
        m_world->removeRigidBody(value);
    }
    void stepSimulation(const vpvl2::Scalar &delta) {
        m_world->stepSimulation(delta, m_maxSubSteps, m_fixedTimeStep);
    }

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher *m_dispatcher;
    btDbvtBroadphase *m_broadphase;
    btSequentialImpulseConstraintSolver *m_solver;
    btDiscreteDynamicsWorld *m_world;
    vpvl2::Scalar m_motionFPS;
    Array<IModel *> m_modelRefs;
    Scalar m_fixedTimeStep;
    int m_maxSubSteps;

    VPVL2_DISABLE_COPY_AND_ASSIGN(World)
};

class BaseRenderContext : public IRenderContext {
public:
    struct TextureCache {
        TextureCache() {}
        TextureCache(int w, int h, GLuint i)
            : width(w),
              height(h),
              id(i)
        {
        }
        int width;
        int height;
        GLuint id;
    };
    typedef std::map<UnicodeString, TextureCache> TextureCacheMap;
    struct InternalTexture {
        InternalTexture(IRenderContext::Texture *r, bool m, bool t)
            : ref(r),
              isToon(t),
              isSystem(false),
              mipmap(m),
              ok(false)
        {
        }
        void assign(const TextureCache &cache) {
            ref->width = cache.width;
            ref->height = cache.height;
            *static_cast<GLuint *>(ref->object) = cache.id;
        }
        IRenderContext::Texture *ref;
        bool isToon;
        bool isSystem;
        bool mipmap;
        bool ok;
    };
    struct InternalContext {
        TextureCacheMap textureCache;
        void addTextureCache(const UnicodeString &path, const TextureCache &cache) {
            textureCache.insert(std::make_pair(path, cache));
        }
        bool findTextureCache(const UnicodeString &path, InternalTexture &texture) {
            if (textureCache.find(path) != textureCache.end()) {
                texture.assign(textureCache[path]);
                texture.ok = true;
                return true;
            }
            return false;
        }
    };

    BaseRenderContext(Scene *sceneRef, UIStringMap *configRef)
        : m_sceneRef(sceneRef),
          m_configRef(configRef),
          m_lightWorldMatrix(1),
          m_lightViewMatrix(1),
          m_lightProjectionMatrix(1),
          m_cameraWorldMatrix(1),
          m_cameraViewMatrix(1),
          m_cameraProjectionMatrix(1)
    {
        std::istringstream in(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
        std::string extension;
        while (std::getline(in, extension, ' ')) {
            m_extensions.insert(extension);
        }
    }
    ~BaseRenderContext()
    {
        m_sceneRef = 0;
        m_configRef = 0;
    }

    void allocateUserData(const IModel * /* model */, void *&context) {
        InternalContext *ctx = new InternalContext();
        context = ctx;
    }
    void releaseUserData(const IModel * /* model */, void *&context) {
        delete static_cast<InternalContext *>(context);
        context = 0;
    }
    bool uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context) {
        bool mipmap = (flags & IRenderContext::kGenerateTextureMipmap) == kGenerateTextureMipmap;
        bool isToon = (flags & IRenderContext::kToonTexture) == kToonTexture;
        bool ret = false;
        InternalTexture t(&texture, mipmap, isToon);
        if (flags & IRenderContext::kTexture2D) {
            const UnicodeString &path = createPath(dir, name);
            std::cerr << "texture: " << String::toStdString(path) << std::endl;
            ret = uploadTextureInternal(path, t, context);
        }
        else if (isToon) {
            if (dir) {
                const UnicodeString &path = createPath(dir, name);
                std::cerr << "toon: " << String::toStdString(path) << std::endl;
                ret = uploadTextureInternal(path, t, context);
            }
            if (!t.ok) {
                String s((*m_configRef)["dir.system.toon"]);
                const UnicodeString &path = createPath(&s, name);
                std::cerr << "system: " << String::toStdString(path) << std::endl;
                t.isSystem = true;
                ret = uploadTextureInternal(path, t, context);
            }
        }
        return ret;
    }
    void getMatrix(float value[], const IModel *model, int flags) const {
        glm::mat4x4 m(1);
        if (flags & IRenderContext::kShadowMatrix) {
            if (flags & IRenderContext::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderContext::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderContext::kWorldMatrix) {
                static const Vector3 plane(0.0f, 1.0f, 0.0f);
                const ILight *light = m_sceneRef->light();
                const Vector3 &direction = light->direction();
                const Scalar dot = plane.dot(-direction);
                float matrix[16];
                for (int i = 0; i < 4; i++) {
                    int offset = i * 4;
                    for (int j = 0; j < 4; j++) {
                        int index = offset + j;
                        matrix[index] = plane[i] * direction[j];
                        if (i == j)
                            matrix[index] += dot;
                    }
                }
                m *= glm::make_mat4x4(matrix);
                m *= m_cameraWorldMatrix;
                m = glm::scale(m, glm::vec3(model->scaleFactor()));
            }
        }
        else if (flags & IRenderContext::kCameraMatrix) {
            if (flags & IRenderContext::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderContext::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderContext::kWorldMatrix) {
                const IBone *bone = model->parentBoneRef();
                Transform transform;
                transform.setOrigin(model->worldPosition());
                transform.setRotation(model->worldRotation());
                Scalar matrix[16];
                transform.getOpenGLMatrix(matrix);
                m *= glm::make_mat4x4(matrix);
                if (bone) {
                    transform = bone->worldTransform();
                    transform.getOpenGLMatrix(matrix);
                    m *= glm::make_mat4x4(matrix);
                }
                m *= m_cameraWorldMatrix;
                m = glm::scale(m, glm::vec3(model->scaleFactor()));
            }
        }
        else if (flags & IRenderContext::kLightMatrix) {
            if (flags & IRenderContext::kWorldMatrix) {
                m *= m_lightWorldMatrix;
                m = glm::scale(m, glm::vec3(model->scaleFactor()));
            }
            if (flags & IRenderContext::kProjectionMatrix)
                m *= m_lightProjectionMatrix;
            if (flags & IRenderContext::kViewMatrix)
                m *= m_lightViewMatrix;
        }
        if (flags & IRenderContext::kInverseMatrix)
            m = glm::inverse(m);
        if (flags & IRenderContext::kTransposeMatrix)
            m = glm::transpose(m);
        memcpy(value, glm::value_ptr(m), sizeof(float) * 16);
    }
    void log(void * /* context */, LogLevel /* level */, const char *format, va_list ap) {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), format, ap);
        std::cerr << buf << std::endl;
    }
    IString *loadShaderSource(ShaderType type, const IModel *model, const IString * /* dir */, void * /* context */) {
        std::string file;
        switch (model->type()) {
        case IModel::kAsset:
            file += "asset/";
            break;
        case IModel::kPMD:
            file += "pmd/";
            break;
        case IModel::kPMX:
            file += "pmx/";
            break;
        default:
            break;
        }
        switch (type) {
        case kEdgeVertexShader:
            file += "edge.vsh";
            break;
        case kEdgeFragmentShader:
            file += "edge.fsh";
            break;
        case kModelVertexShader:
            file += "model.vsh";
            break;
        case kModelFragmentShader:
            file += "model.fsh";
            break;
        case kShadowVertexShader:
            file += "shadow.vsh";
            break;
        case kShadowFragmentShader:
            file += "shadow.fsh";
            break;
        case kZPlotVertexShader:
            file += "zplot.vsh";
            break;
        case kZPlotFragmentShader:
            file += "zplot.fsh";
            break;
        case kEdgeWithSkinningVertexShader:
            file += "skinning/edge.vsh";
            break;
        case kModelWithSkinningVertexShader:
            file += "skinning/model.vsh";
            break;
        case kShadowWithSkinningVertexShader:
            file += "skinning/shadow.vsh";
            break;
        case kZPlotWithSkinningVertexShader:
            file += "skinning/zplot.vsh";
            break;
        case kModelEffectTechniques:
        case kMaxShaderType:
        default:
            break;
        }
        UnicodeString path = (*m_configRef)["dir.system.shaders"];
        path.append("/");
        path.append(UnicodeString::fromUTF8(file));
        std::string bytes;
        if (loadFile(path, bytes)) {
            return new(std::nothrow) String(UnicodeString::fromUTF8("#version 120\n" + bytes));
        }
        else {
            return 0;
        }
    }
    IString *loadKernelSource(KernelType type, void * /* context */) {
        std::string file;
        switch (type) {
        case kModelSkinningKernel:
            file += "skinning.cl";
            break;
        default:
            break;
        }
        UnicodeString path = (*m_configRef)["dir.system.kernels"];
        path.append("/");
        path.append(UnicodeString::fromUTF8(file));
        std::string bytes;
        if (loadFile(path, bytes)) {
            return new(std::nothrow) String(UnicodeString::fromUTF8(bytes));
        }
        else {
            return 0;
        }
    }
    IString *toUnicode(const uint8_t *str) const {
        const char *s = reinterpret_cast<const char *>(str);
        return new(std::nothrow) String(UnicodeString(s, strlen(s), "shift_jis"));
    }
    bool hasExtension(const void *namePtr) const {
        return m_extensions.find(static_cast<const char *>(namePtr)) != m_extensions.end();
    }
    void startProfileSession(ProfileType /* type */, const void * /* arg */) {
        // TODO: implement here
    }
    void stopProfileSession(ProfileType /* type */, const void * /* arg */) {
        // TODO: implement here
    }

    IString *loadShaderSource(ShaderType /* type */, const IString * /* path */) {
        return 0;
    }
    void getToonColor(const IString * /* name */, const IString * /* dir */, Color & /* value */, void * /* context */) {
    }
    void getViewport(Vector3 &value) const {
        value.setValue(m_viewport.x, m_viewport.y, 0);
    }
    void getMousePosition(Vector4 &value, MousePositionType type) const {
        switch (type) {
        case kMouseLeftPressPosition:
            value.setValue(m_mouseLeftPressPosition.x,
                           m_mouseLeftPressPosition.y,
                           m_mouseLeftPressPosition.z,
                           m_mouseLeftPressPosition.w);
            break;
        case kMouseMiddlePressPosition:
            value.setValue(m_mouseMiddlePressPosition.x,
                           m_mouseMiddlePressPosition.y,
                           m_mouseMiddlePressPosition.z,
                           m_mouseMiddlePressPosition.w);
            break;
        case kMouseRightPressPosition:
            value.setValue(m_mouseRightPressPosition.x,
                           m_mouseRightPressPosition.y,
                           m_mouseRightPressPosition.z,
                           m_mouseRightPressPosition.w);
            break;
        case kMouseCursorPosition:
            value.setValue(m_mouseCursorPosition.x,
                           m_mouseCursorPosition.y,
                           m_mouseCursorPosition.z,
                           m_mouseCursorPosition.w);
            break;
        default:
            break;
        }
    }
    void getTime(float & /* value */, bool /* sync */) const {
    }
    void getElapsed(float & /* value */, bool /* sync */) const {
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
    }
    IModel *findModel(const IString * /* name */) const {
        return 0;
    }
    IModel *effectOwner(const IEffect * /* effect */) const {
        return 0;
    }
    void setRenderColorTargets(const void *targets, const int ntargets) {
    }
    FrameBufferObject *createFrameBufferObject() {
        return new FrameBufferObject(m_viewport.x, m_viewport.y, 4);
    }
    bool hasFrameBufferObjectBound() const {
        return false;
    }
    void getEffectCompilerArguments(Array<IString *> & /* arguments */) const {
    }
    const IString *effectFilePath(const IModel * /* model */, const IString * /* dir */) const {
        return 0;
    }

    void setCameraMatrix(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection) {
        m_cameraWorldMatrix = world;
        m_cameraViewMatrix = view;
        m_cameraProjectionMatrix = projection;
    }
    void setViewport(const glm::vec2 &value) {
        m_viewport = value;
    }
    void setMousePosition(const glm::vec2 &value, bool pressed, MousePositionType type) {
        switch (type) {
        case kMouseLeftPressPosition:
            m_mouseLeftPressPosition = glm::vec4(value.x, value.y, pressed, 0);
            break;
        case kMouseMiddlePressPosition:
            m_mouseMiddlePressPosition = glm::vec4(value.x, value.y, pressed, 0);
            break;
        case kMouseRightPressPosition:
            m_mouseRightPressPosition = glm::vec4(value.x, value.y, pressed, 0);
            break;
        case kMouseCursorPosition:
            m_mouseCursorPosition = glm::vec4(value.x, value.y, 0, 0);
            break;
        default:
            break;
        }
    }
    void updateCameraMatrix(const glm::vec2 &size) {
        const ICamera *camera = m_sceneRef->camera();
        Scalar matrix[16];
        camera->modelViewTransform().getOpenGLMatrix(matrix);
        const glm::mediump_float &aspect = size.x / size.y;
        const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
                &projection = glm::infinitePerspective(camera->fov(), aspect, camera->znear());
        setCameraMatrix(world, view, projection);
        setViewport(size);
    }

    virtual bool loadFile(const UnicodeString &path, std::string &bytes) = 0;

protected:
    static const UnicodeString createPath(const IString *dir, const UnicodeString &name) {
        UnicodeString n = name;
        return static_cast<const String *>(dir)->value() + "/" + n.findAndReplace('\\', '/');
    }
    static const UnicodeString createPath(const IString *dir, const IString *name) {
        const UnicodeString &d = static_cast<const String *>(dir)->value();
        UnicodeString n = static_cast<const String *>(name)->value();
        return d + "/" + n.findAndReplace('\\', '/');
    }
    void generateMipmap() const {
#ifdef VPVL2_LINK_GLEW
        if (glewIsSupported("glGenerateMipmap"))
            glGenerateMipmap(GL_TEXTURE_2D);
#else
        const void *procs[] = { "glGenerateMipmap", "glGenerateMipmapEXT", 0 };
        typedef void (*glGenerateMipmapProcPtr)(GLuint);
        if (glGenerateMipmapProcPtr glGenerateMipmapProcPtrRef = reinterpret_cast<glGenerateMipmapProcPtr>(findProcedureAddress(procs)))
            glGenerateMipmapProcPtrRef(GL_TEXTURE_2D);
#endif
    }
    GLuint createTexture(const void *ptr, size_t width, size_t height, GLenum format, GLenum type, bool mipmap, bool canOptimize) const {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(GL_APPLE_client_storage) && defined(GL_APPLE_texture_range)
        if (canOptimize) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
            glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        }
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, ptr);
        if (mipmap)
            generateMipmap();
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }
    virtual bool uploadTextureInternal(const UnicodeString &path, InternalTexture &texture, void *context) = 0;

    Scene *m_sceneRef;
    UIStringMap *m_configRef;
    glm::mat4x4 m_lightWorldMatrix;
    glm::mat4x4 m_lightViewMatrix;
    glm::mat4x4 m_lightProjectionMatrix;
    glm::mat4x4 m_cameraWorldMatrix;
    glm::mat4x4 m_cameraViewMatrix;
    glm::mat4x4 m_cameraProjectionMatrix;
    glm::vec4 m_mouseCursorPosition;
    glm::vec4 m_mouseLeftPressPosition;
    glm::vec4 m_mouseMiddlePressPosition;
    glm::vec4 m_mouseRightPressPosition;
    glm::vec2 m_viewport;
    std::set<std::string> m_extensions;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_BASERENDERCONTEXT_H_ */

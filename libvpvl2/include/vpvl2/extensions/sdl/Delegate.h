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

/* libvpvl2 */
#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>
#include <vpvl2/extensions/icu/Encoding.h>

/* SDL */
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

/* Bullet Physics */
#ifndef VPVL2_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

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
#endif

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

namespace {

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(SDL_RWops *buffer)
        : m_buffer(buffer),
          m_size(0)
    {
        size_t pos = tell();
        SDL_RWseek(m_buffer, 0, RW_SEEK_END);
        m_size = tell();
        seek(pos);
    }
    ~ReadonlyMemoryStream() {
        m_buffer = 0;
        m_size = 0;
    }

    bool isSaving() const { return false; }
    bool isError() const { return false; }
    void seek(uint pos) { SDL_RWseek(m_buffer, pos, RW_SEEK_SET); }
    uint tell() const { return SDL_RWtell(m_buffer); }
    uint size() const { return m_size; }
    void clearError() {}
    bool isAtEnd() const { return tell() == m_size; }
    bool isSeekable() const { return true; }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return SDL_RWread(m_buffer, data, len, 1); }

private:
    SDL_RWops *m_buffer;
    size_t m_size;
};

}

#else

namespace nv {
class Stream {
public:
    Stream() {}
    virtual ~Stream() {}
};
}

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const QString &/*path*/) {}
    ~ReadonlyFileStream() {}
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(QByteArray &/*bytes*/) {}
    ~ReadonlyMemoryStream() {}
};

#endif

namespace vpvl2
{
namespace extensions
{
namespace sdl
{
using namespace icu;

static bool UILoadFile(const UnicodeString &path, std::string &bytes)
{
    bytes.clear();
    FILE *fp = ::fopen(String::toStdString(path).c_str(), "rb");
    bool ret = false;
    if (fp) {
        ::fseek(fp, 0, SEEK_END);
        size_t size = ::ftell(fp);
        ::fseek(fp, 0, SEEK_SET);
        std::vector<char> data(size);
        ::fread(&data[0], size, 1, fp);
        bytes.assign(data.begin(), data.end());
        ret = true;
    }
    ::fclose(fp);
    return ret;
}

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
        m_fixedTimeStep = 1.0 / value;
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

class Delegate : public IRenderDelegate {
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
    struct InternalContext {
        TextureCacheMap textureCache;
    };
    struct InternalTexture {
        InternalTexture(Delegate::Texture *r, bool m, bool t)
            : ref(r),
              isToon(t),
              isSystem(false),
              mipmap(m),
              ok(false)
        {
        }
        Delegate::Texture *ref;
        bool isToon;
        bool isSystem;
        bool mipmap;
        bool ok;
    };

    Delegate(Scene *sceneRef, UIStringMap *configRef)
        : m_sceneRef(sceneRef),
          m_configRef(configRef),
          m_lightWorldMatrix(1),
          m_lightViewMatrix(1),
          m_lightProjectionMatrix(1),
          m_cameraWorldMatrix(1),
          m_cameraViewMatrix(1),
          m_cameraProjectionMatrix(1),
          m_colorSwapSurface(0)
    {
        m_colorSwapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32,
                                                  0x00ff0000,
                                                  0x0000ff00,
                                                  0x000000ff,
                                                  0xff000000);
        std::istringstream in(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
        std::string extension;
        while (std::getline(in, extension, ' ')) {
            m_extensions.insert(extension);
        }
    }
    ~Delegate()
    {
        m_sceneRef = 0;
        m_configRef = 0;
        SDL_FreeSurface(m_colorSwapSurface);
    }

    void allocateContext(const IModel * /* model */, void *&context) {
        InternalContext *ctx = new InternalContext();
        context = ctx;
    }
    void releaseContext(const IModel * /* model */, void *&context) {
        delete static_cast<InternalContext *>(context);
        context = 0;
    }
    bool uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context) {
        bool mipmap = flags & IRenderDelegate::kGenerateTextureMipmap;
        bool isToon = flags & IRenderDelegate::kToonTexture;
        bool ret = false;
        InternalTexture t(&texture, mipmap, isToon);
        if (flags & IRenderDelegate::kTexture2D) {
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
        if (flags & IRenderDelegate::kShadowMatrix) {
            if (flags & IRenderDelegate::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderDelegate::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderDelegate::kWorldMatrix) {
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
        else if (flags & IRenderDelegate::kCameraMatrix) {
            if (flags & IRenderDelegate::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderDelegate::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderDelegate::kWorldMatrix) {
                const IBone *bone = model->parentBone();
                Transform transform;
                transform.setOrigin(model->position());
                transform.setRotation(model->rotation());
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
        else if (flags & IRenderDelegate::kLightMatrix) {
            if (flags & IRenderDelegate::kWorldMatrix) {
                m *= m_lightWorldMatrix;
                m = glm::scale(m, glm::vec3(model->scaleFactor()));
            }
            if (flags & IRenderDelegate::kProjectionMatrix)
                m *= m_lightProjectionMatrix;
            if (flags & IRenderDelegate::kViewMatrix)
                m *= m_lightViewMatrix;
        }
        if (flags & IRenderDelegate::kInverseMatrix)
            m = glm::inverse(m);
        if (flags & IRenderDelegate::kTransposeMatrix)
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
        if (UILoadFile(path, bytes)) {
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
        if (UILoadFile(path, bytes)) {
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
    void *findProcedureAddress(const void **candidatesPtr) const {
        const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
        const char *candidate = candidates[0];
        int i = 0;
        while (candidate) {
            void *address = SDL_GL_GetProcAddress(candidate);
            if (address) {
                return address;
            }
            candidate = candidates[++i];
        }
        return 0;
    }

    IString *loadShaderSource(ShaderType /* type */, const IString * /* path */) {
        return 0;
    }
    void getToonColor(const IString * /* name */, const IString * /* dir */, Color & /* value */, void * /* context */) {
    }
    void getViewport(Vector3 & /* value */) const {
    }
    void getMousePosition(Vector4 & /* value */, MousePositionType /* type */) const {
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
    void setRenderColorTargets(const void * /* targets */, const int /* ntargets */) {
    }
    void bindRenderColorTarget(void * /* texture */, size_t /* width */, size_t /* height */, int /* index */, bool /* enableAA */) {
    }
    void releaseRenderColorTarget(void * /* texture */, size_t /* width */, size_t /* height */, int /* index */, bool /* enableAA */) {
    }
    void bindRenderDepthStencilTarget(void * /* texture */, void * /* depth */, void * /* stencil */, size_t /* width */, size_t /* height */, bool /* enableAA */) {
    }
    void releaseRenderDepthStencilTarget(void * /* texture */, void * /* depth */, void * /* stencil */, size_t /* width */, size_t /* height */, bool /* enableAA */) {
    }

    void setCameraMatrix(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection) {
        m_cameraWorldMatrix = world;
        m_cameraViewMatrix = view;
        m_cameraProjectionMatrix = projection;
    }

private:
    static const UnicodeString createPath(const IString *dir, const UnicodeString &name) {
        UnicodeString n = name;
        return static_cast<const String *>(dir)->value() + "/" + n.findAndReplace('\\', '/');
    }
    static const UnicodeString createPath(const IString *dir, const IString *name) {
        const UnicodeString &d = static_cast<const String *>(dir)->value();
        UnicodeString n = static_cast<const String *>(name)->value();
        return d + "/" + n.findAndReplace('\\', '/');
    }
    bool uploadTextureInternal(const UnicodeString &path, InternalTexture &texture, void *context) {
        InternalContext *privateContext = static_cast<InternalContext *>(context);
        /* テクスチャのキャッシュを検索する */
        if (privateContext) {
            TextureCacheMap &tc = privateContext->textureCache;
            if (tc.find(path) != tc.end()) {
                setTextureID(tc[path], texture);
                texture.ok = true;
                return true;
            }
        }
        std::string bytes;
        if (!UILoadFile(path, bytes)) {
            texture.ok = false;
            return true;
        }
        SDL_Surface *surface = 0;
        SDL_RWops *source = SDL_RWFromConstMem(bytes.data(), bytes.length());
        const UnicodeString &lowerPath = path.tempSubString().toLower();
        char extension[4] = { 0 };
        if (lowerPath.endsWith(".sph") || lowerPath.endsWith(".spa")) {
            memcpy(extension, "BMP" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".tga")) {
            memcpy(extension, "TGA" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".png") && IMG_isPNG(source)) {
            memcpy(extension, "PNG" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".bmp") && IMG_isBMP(source)) {
            memcpy(extension, "BMP" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".jpg") && IMG_isJPG(source)) {
            memcpy(extension, "JPG" ,sizeof(extension));
        }
        if (*extension) {
            surface = IMG_LoadTyped_RW(source, 0, extension);
            surface = SDL_ConvertSurface(surface, m_colorSwapSurface->format, SDL_SWSURFACE);
        }
#ifdef VPVL2_LINK_NVTT
        else if (lowerPath.endsWith(".dds")) {
            nv::DirectDrawSurface dds;
            if (dds.load(new ReadonlyMemoryStream(source))) {
                nv::Image *nvimage = new nv::Image();
                dds.mipmap(nvimage, 0, 0);
                surface = SDL_CreateRGBSurfaceFrom(nvimage->pixels(),
                                                   nvimage->width(),
                                                   nvimage->height(),
                                                   32,
                                                   nvimage->width() * 4,
                                                   0x00ff0000,
                                                   0x0000ff00,
                                                   0x000000ff,
                                                   0xff000000);
                surface = SDL_ConvertSurface(surface, m_colorSwapSurface->format, SDL_SWSURFACE);
            }
        }
#endif
        SDL_FreeRW(source);
        if (!surface) {
            texture.ok = false;
            return true;
        }
        GLuint textureID = 0;
        size_t width = surface->w, height = surface->h;
        glGenTextures(1, &textureID);
#if defined(GL_APPLE_client_storage) && defined(GL_APPLE_texture_range)
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA,
                     GL_UNSIGNED_INT_8_8_8_8_REV, surface->pixels);
        if (texture.mipmap) {
            const void *procs[] = { "glGenerateMipmap", "glGenerateMipmapEXT", 0 };
            typedef void (*glGenerateMipmapProcPtr)(GLuint);
            if (glGenerateMipmapProcPtr glGenerateMipmapProcPtrRef = reinterpret_cast<glGenerateMipmapProcPtr>(findProcedureAddress(procs)))
                glGenerateMipmapProcPtrRef(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
#else
        GLuint internal = GL_RGBA, format = GL_BGRA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, format,
                     GL_UNSIGNED_INT_8_8_8_8_REV, surface->pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
        SDL_FreeSurface(surface);
        TextureCache cache(width, height, textureID);
        setTextureID(cache, texture);
        addTextureCache(path, cache, privateContext);
        texture.ok = textureID != 0;
        return texture.ok;
    }
    static void setTextureID(const TextureCache &cache, InternalTexture &texture) {
        Delegate::Texture *ref = texture.ref;
        ref->width = cache.width;
        ref->height = cache.height;
        *const_cast<GLuint *>(static_cast<const GLuint *>(ref->object)) = cache.id;
        if (!texture.isToon) {
            GLuint textureID = *static_cast<const GLuint *>(ref->object);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    static void addTextureCache(const UnicodeString &path, const TextureCache &texture, InternalContext *context) {
        if (context)
            context->textureCache.insert(std::make_pair(path, texture));
    }

    Scene *m_sceneRef;
    UIStringMap *m_configRef;
    glm::mat4x4 m_lightWorldMatrix;
    glm::mat4x4 m_lightViewMatrix;
    glm::mat4x4 m_lightProjectionMatrix;
    glm::mat4x4 m_cameraWorldMatrix;
    glm::mat4x4 m_cameraViewMatrix;
    glm::mat4x4 m_cameraProjectionMatrix;
    SDL_Surface *m_colorSwapSurface;
    std::set<std::string> m_extensions;
};

} /* namespace sdl */
} /* namespace extensions */
} /* namespace vpvl2 */

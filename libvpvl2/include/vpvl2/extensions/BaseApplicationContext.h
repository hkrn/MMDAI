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

#pragma once
#ifndef VPVL2_EXTENSIONS_BASEAPPLICATIONCONTEXT_H_
#define VPVL2_EXTENSIONS_BASEAPPLICATIONCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/IApplicationContext.h>
#include <vpvl2/IEffect.h>
#include <vpvl2/Scene.h>
#include <vpvl2/extensions/gl/FrameBufferObject.h>
#include <vpvl2/extensions/icu4c/Encoding.h>

/* STL */
#include <memory>
#include <map>
#include <string>
#include <vector>

/* GLM */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-op-parentheses"
#include <glm/glm.hpp>
#pragma clang diagnostic pop

/* Cg and ICU (i18n) */
#ifdef VPVL2_ENABLE_NVIDIA_CG
#ifdef VPVL2_OS_OSX
#include <cg.h>
#else /* VPVL2_OS_OSX */
#include <Cg/cg.h>
#endif /* VPVL2_OS_OSX */
#endif /* VPVL2_ENABLE_NVIDIA_CG */

#if defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX)
#include <unicode/regex.h>
#endif

#if !defined(VPVL2_MAKE_SMARTPTR) && !defined(VPVL2_MAKE_SMARTPTR2)
#if __cplusplus > 199907L
#define VPVL2_MAKE_SMARTPTR(kClass) typedef std::unique_ptr<kClass> kClass ## SmartPtr
#define VPVL2_MAKE_SMARTPTR2(kClass, kDestructor) typedef std::unique_ptr<kClass, kDestructor> kClass ## SmartPtr
#elif defined(VPVL2_ENABLE_BOOST)
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/checked_delete.hpp>
#define VPVL2_MAKE_SMARTPTR(kClass) typedef boost::interprocess::unique_ptr<kClass, boost::checked_deleter<kClass> > kClass ## SmartPtr
#define VPVL2_MAKE_SMARTPTR2(kClass, kDestructor) typedef boost::interprocess::unique_ptr<kClass, kDestructor> kClass ## SmartPtr
#else
/* std::auto_ptr is deprecated and cannot use move semantics correctly */
#define VPVL2_MAKE_SMARTPTR(kClass) typedef std::auto_ptr<kClass> kClass ## SmartPtr
#define VPVL2_MAKE_SMARTPTR2(kClass, kDestructor) typedef std::auto_ptr<kClass> kClass ## SmartPtr
#endif
#endif /* VPVL2_MAKE_SMART_PTR */

#if defined(VPVL2_LINK_INTEL_TBB) && !defined(VPVL2_OS_OSX)
#include <tbb/task_scheduler_init.h>
#else
namespace tbb {
struct task_scheduler_init {
task_scheduler_init() {}
~task_scheduler_init() {}
};
} /* namespace tbb */
#endif

namespace nv {
class Timer;
}

namespace vpvl2 {
class Factory;
class IModel;
class IMotion;
class IRenderEngine;
class ITexture;

namespace extensions {
class Archive;
class World;
namespace gl {
class SimpleShadowMap;
VPVL2_MAKE_SMARTPTR(FrameBufferObject);
VPVL2_MAKE_SMARTPTR(SimpleShadowMap);
}
namespace icu4c {
class Encoding;
class StringMap;
VPVL2_MAKE_SMARTPTR(Encoding);
}

VPVL2_MAKE_SMARTPTR(Archive);
VPVL2_MAKE_SMARTPTR(Factory);
VPVL2_MAKE_SMARTPTR2(IModel, Scene::Deleter);
VPVL2_MAKE_SMARTPTR2(IMotion, Scene::Deleter);
VPVL2_MAKE_SMARTPTR2(IRenderEngine, Scene::Deleter);
VPVL2_MAKE_SMARTPTR(IString);
VPVL2_MAKE_SMARTPTR(Scene);
VPVL2_MAKE_SMARTPTR(World);

#if defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX)
VPVL2_MAKE_SMARTPTR(IEffect);
VPVL2_MAKE_SMARTPTR(RegexMatcher);
#endif /* VPVL2_ENABLE_NVIDIA_CG */

class VPVL2_API BaseApplicationContext : public IApplicationContext {
public:
    struct MapBuffer {
        MapBuffer(const BaseApplicationContext *baseApplicationContext)
            : baseRenderContextRef(baseApplicationContext),
              address(0),
              size(0),
              opaque(0)
        {
        }
        ~MapBuffer() {
            baseRenderContextRef->unmapFile(this);
            address = 0;
            size = 0;
            opaque = 0;
        }
        const BaseApplicationContext *baseRenderContextRef;
        uint8 *address;
        vsize size;
        intptr_t opaque;
    };
    class ModelContext {
    public:
        ModelContext(BaseApplicationContext *applicationContextRef, Archive *archiveRef, const IString *directory);
        ~ModelContext();
        void addTextureCache(const std::string &path, ITexture *textureRef);
        bool findTextureCache(const std::string &path, TextureDataBridge &bridge) const;
        bool uploadTexture(const std::string &path, TextureDataBridge &bridge);
        bool uploadTexture(const uint8 *data, vsize size, const std::string &key, TextureDataBridge &bridge);
        bool cacheTexture(const std::string &key, ITexture *textureRef, TextureDataBridge &bridge);
        void optimizeTexture(ITexture *texture);
        int countCachedTextures() const;
        ITexture *createTexture(const void *ptr, const extensions::gl::BaseSurface::Format &format, const Vector3 &size, bool mipmap) const;
        ITexture *createTexture(const uint8 *data, vsize size, bool mipmap);
        Archive *archiveRef() const;
        const IString *directoryRef() const;
    private:
        static const extensions::gl::GLenum kGL_UNPACK_CLIENT_STORAGE_APPLE = 0x85B2;
        static const extensions::gl::GLenum kGL_TEXTURE_STORAGE_HINT_APPLE = 0x85BC;
        static const extensions::gl::GLenum kGL_STORAGE_CACHED_APPLE = 0x85BE;
        typedef void (GLAPIENTRY * PFNGLPIXELSTOREIPROC) (extensions::gl:: GLenum pname, extensions::gl::GLint param);
        PFNGLPIXELSTOREIPROC pixelStorei;
        typedef std::map<std::string, ITexture *> TextureCacheMap;
        const IString *m_directoryRef;
        Archive *m_archiveRef;
        BaseApplicationContext *m_applicationContextRef;
        TextureCacheMap m_textureRefCache;
        float m_maxAnisotropyValue;
    };

    static bool initializeOnce(const char *argv0, const char *logdir, int vlog);
    static void terminate();

    BaseApplicationContext(Scene *sceneRef, IEncoding *encodingRef, const icu4c::StringMap *configRef);
    ~BaseApplicationContext();

    void initialize(bool enableDebug);
    void release();

    bool uploadTexture(const IString *name, TextureDataBridge &bridge, void *userData);
    void getMatrix(float32 value[], const IModel *model, int flags) const;
    IString *loadShaderSource(ShaderType type, const IModel *model, void *userData);
    IString *loadShaderSource(ShaderType type, const IString *path);
    IString *loadKernelSource(KernelType type, void *userData);
    IString *toUnicode(const uint8 *str) const;

    typedef std::pair<IEffect *, bool> EffectAttachmentValue;
    typedef std::pair<RegexMatcher *, EffectAttachmentValue> EffectAttachmentRule;
    typedef std::vector<EffectAttachmentRule> EffectAttachmentRuleList;
    class OffscreenTexture {
    public:
        OffscreenTexture(const IEffect::OffscreenRenderTarget &r,
                         const EffectAttachmentRuleList &a,
                         const Vector3 &size,
                         FunctionResolver *resolver)
            : renderTarget(r),
              attachmentRules(a),
              colorTextureRef(r.textureRef),
              depthStencilBuffer(resolver, createDepthFormat(r.textureRef, resolver), size)
        {
            depthStencilBuffer.create();
        }
        ~OffscreenTexture() {
            EffectAttachmentRuleList::const_iterator it = attachmentRules.begin();
            while (it != attachmentRules.end()) {
                delete it->first;
                ++it;
            }
        }
        const IEffect::OffscreenRenderTarget renderTarget;
        const EffectAttachmentRuleList attachmentRules;
        ITexture *colorTextureRef;
        extensions::gl::FrameBufferObject::StandardRenderBuffer depthStencilBuffer;
    private:
        static extensions::gl::BaseSurface::Format createDepthFormat(const ITexture *texture, FunctionResolver *resolver) {
            const extensions::gl::BaseSurface::Format *formatPtr = reinterpret_cast<extensions::gl::BaseSurface::Format *>(texture->format());
            return extensions::gl::BaseSurface::Format(0, extensions::gl::FrameBufferObject::detectDepthFormat(resolver, formatPtr->internal), 0, 0);
        }

        VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenTexture)
    };

    void getViewport(Vector3 &value) const;
    void getMousePosition(Vector4 &value, MousePositionType type) const;
    IModel *findModel(const IString *name) const;
    IModel *effectOwner(const IEffect *effect) const;
    void setEffectOwner(const IEffect *effectRef, IModel *model);
    void addModelPath(IModel *model, const std::string &path);
    std::string effectOwnerName(const IEffect *effect) const;
    extensions::gl::FrameBufferObject *createFrameBufferObject();
    void getEffectCompilerArguments(Array<IString *> &arguments) const;
    const IString *effectFilePath(const IModel *model, const IString *dir) const;
    void addSharedTextureParameter(const char *name, const SharedTextureParameter &parameter);
    bool tryGetSharedTextureParameter(const char *name, SharedTextureParameter &parameter) const;
    void setMousePosition(const glm::vec4 &value, MousePositionType type, bool &handled);
    void handleKeyPress(int value, int modifiers, bool &handled);
    std::string findModelPath(const IModel *modelRef) const;
    std::string findModelBasename(const IModel *modelRef) const;
    extensions::gl::FrameBufferObject *findFrameBufferObjectByRenderTarget(const IEffect::OffscreenRenderTarget &rt, bool enableAA);
    void bindOffscreenRenderTarget(OffscreenTexture *textureRef, bool enableAA);
    void releaseOffscreenRenderTarget(const OffscreenTexture *textureRef, bool enableAA);
    void parseOffscreenSemantic(IEffect *effectRef, const IString *directoryRef);
    void renderOffscreen();
    void createEffectParameterUIWidgets(IEffect *effectRef);
    void renderEffectParameterUIWidgets();
    IEffect *createEffectRef(const IString *path);
    IEffect *createEffectRef(IModel *modelRef, const IString *directoryRef);
#if 0
    void addModelPath(IModel * /* model */, const std::string & /* path */) {}
    void parseOffscreenSemantic(IEffect * /* effect */, const IString * /* dir */) {}
    void renderOffscreen() {}
    IEffect *createEffectRef(IModel * /* model */, const IString * /* dir */) { return 0; }
#endif /* VPVL2_ENABLE_NVIDIA_CG */

    IModel *currentModelRef() const;
    void setCurrentModelRef(IModel *value);
    int samplesMSAA() const;
    void setSamplesMSAA(int value);
    Scene *sceneRef() const;
    void getCameraMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection) const;
    void setCameraMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection);
    void getLightMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection) const;
    void setLightMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection);
    void updateCameraMatrices();
    glm::ivec4 viewportRegion() const;
    void setViewportRegion(const glm::ivec4 &value);
    void createShadowMap(const Vector3 &size);
    void releaseShadowMap();
    void renderShadowMap();

    virtual bool mapFile(const std::string &path, MapBuffer *bufferRef) const = 0;
    virtual bool unmapFile(MapBuffer *bufferRef) const = 0;
    virtual bool existsFile(const std::string &path) const = 0;

protected:
    typedef void (GLAPIENTRY * PFNGLGETINTEGERVPROC) (extensions::gl::GLenum pname, extensions::gl::GLint *params);
    typedef void (GLAPIENTRY * PFNGLVIEWPORTPROC) (extensions::gl::GLint x, extensions::gl::GLint y, extensions::gl::GLsizei width, extensions::gl::GLsizei height);
    typedef void (GLAPIENTRY * PFNGLCLEARPROC) (extensions::gl::GLbitfield mask);
    typedef void (GLAPIENTRY * PFNGLCLEARCOLORPROC) (extensions::gl::GLclampf red, extensions::gl::GLclampf green, extensions::gl::GLclampf blue, extensions::gl::GLclampf alpha);
    typedef void (GLAPIENTRY * PFNGLCLEARDEPTHPROC) (extensions::gl::GLclampd depth);
    PFNGLGETINTEGERVPROC getIntegerv;
    PFNGLVIEWPORTPROC viewport;
    PFNGLCLEARPROC clear;
    PFNGLCLEARCOLORPROC clearColor;
    PFNGLCLEARDEPTHPROC clearDepth;

    bool uploadSystemToonTexture(const std::string &name, TextureDataBridge &bridge, ModelContext *context);
    bool internalUploadTexture(const std::string &name, const std::string &path, TextureDataBridge &bridge, ModelContext *context);
    void validateEffectResources();
    std::string toonDirectory() const;
    std::string shaderDirectory() const;
    std::string effectDirectory() const;
    std::string kernelDirectory() const;

    virtual bool uploadTextureOpaque(const uint8 *data, vsize size, const std::string &key, ModelContext *context, TextureDataBridge &bridge);
    virtual bool uploadTextureOpaque(const std::string &path, ModelContext *context, TextureDataBridge &bridge);
    virtual gl::BaseSurface::Format defaultTextureFormat() const;

    const icu4c::StringMap *m_configRef;
    Scene *m_sceneRef;
    IEncoding *m_encodingRef;
    IModel *m_currentModelRef;
    extensions::gl::BaseSurface::Format m_renderColorFormat;
    extensions::gl::SimpleShadowMapSmartPtr m_shadowMap;
    glm::mat4x4 m_lightWorldMatrix;
    glm::mat4x4 m_lightViewMatrix;
    glm::mat4x4 m_lightProjectionMatrix;
    glm::mat4x4 m_cameraWorldMatrix;
    glm::mat4x4 m_cameraViewMatrix;
    glm::mat4x4 m_cameraProjectionMatrix;
    glm::ivec4 m_viewportRegion;
    glm::mediump_float m_aspectRatio;

    typedef PointerHash<HashPtr, extensions::gl::FrameBufferObject> RenderTargetMap;
    typedef PointerHash<HashString, IEffect> Path2EffectMap;
    typedef Hash<HashPtr, std::string> ModelRef2PathMap;
    typedef Hash<HashPtr, std::string> ModelRef2BasenameMap;
    typedef Hash<HashPtr, IModel *> EffectRef2ModelRefMap;
    typedef Hash<HashPtr, std::string> EffectRef2OwnerNameMap;
    typedef Hash<HashString, IModel *> Name2ModelRefMap;
    typedef PointerArray<OffscreenTexture> OffscreenTextureList;
    typedef std::pair<const IEffect::Parameter *, const char *> SharedTextureParameterKey;
    typedef std::map<SharedTextureParameterKey, SharedTextureParameter> SharedTextureParameterMap;
    glm::vec4 m_mouseCursorPosition;
    glm::vec4 m_mouseLeftPressPosition;
    glm::vec4 m_mouseMiddlePressPosition;
    glm::vec4 m_mouseRightPressPosition;
    Path2EffectMap m_effectCaches;
    Name2ModelRefMap m_basename2modelRefs;
    ModelRef2PathMap m_modelRef2Paths;
    ModelRef2BasenameMap m_modelRef2Basenames;
    EffectRef2ModelRefMap m_effectRef2modelRefs;
    EffectRef2OwnerNameMap m_effectRef2owners;
    RenderTargetMap m_renderTargets;
    OffscreenTextureList m_offscreenTextures;
    SharedTextureParameterMap m_sharedParameters;
    Array<vpvl2::IEffect::Technique *> m_offscreenTechniques;
    mutable IStringSmartPtr m_effectPathPtr;
    int m_samplesMSAA;
    bool m_viewportRegionInvalidated;

private:
    static void debugMessageCallback(gl::GLenum source, gl::GLenum type, gl::GLuint id, gl::GLenum severity,
                                     gl::GLsizei length, const gl::GLchar *message, gl::GLvoid *userData);

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseApplicationContext)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_BASEAPPLICATIONCONTEXT_H_ */

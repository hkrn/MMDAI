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
#include <vpvl2/IEffect.h>
#include <vpvl2/IRenderContext.h>
#include <vpvl2/Scene.h>

/* STL */
#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>
/*
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
*/

/* GLM */
#include <glm/glm.hpp>

/* NVTT */

/* Cg and ICU */
#ifdef VPVL2_ENABLE_NVIDIA_CG
#include <Cg/cg.h>
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

namespace nv {
class Timer;
}

namespace vpvl2 {
class Factory;
class IModel;
class IMotion;
class IRenderEngine;

namespace extensions {
class Archive;
class World;
namespace gl {
class SimpleShadowMap;
}
namespace icu4c {
class Encoding;
class String;
class StringMap;
}
using namespace icu4c;

VPVL2_MAKE_SMARTPTR(Archive);
VPVL2_MAKE_SMARTPTR(Encoding);
VPVL2_MAKE_SMARTPTR(Factory);
VPVL2_MAKE_SMARTPTR(FrameBufferObject);
VPVL2_MAKE_SMARTPTR2(IModel, Scene::Deleter);
VPVL2_MAKE_SMARTPTR2(IMotion, Scene::Deleter);
VPVL2_MAKE_SMARTPTR2(IRenderEngine, Scene::Deleter);
VPVL2_MAKE_SMARTPTR(Scene);
VPVL2_MAKE_SMARTPTR(SimpleShadowMap);
VPVL2_MAKE_SMARTPTR(String);
VPVL2_MAKE_SMARTPTR(World);

#ifdef VPVL2_ENABLE_NVIDIA_CG
VPVL2_MAKE_SMARTPTR(IEffect);
VPVL2_MAKE_SMARTPTR(RegexMatcher);
#endif /* VPVL2_ENABLE_NVIDIA_CG */

class VPVL2_API BaseRenderContext : public IRenderContext {
public:
    struct TextureCache {
        TextureCache() {}
        TextureCache(const Texture &t)
            : size(t.size),
              opaque(t.opaque)
        {
        }
        const Vector3 size;
        intptr_t opaque;
    };
    typedef std::map<UnicodeString, TextureCache> TextureCacheMap;
    struct ModelContext {
        TextureCacheMap textureCache;
        void addTextureCache(const UnicodeString &path, const TextureCache &cache) {
            textureCache.insert(std::make_pair(path, cache));
        }
        bool findTextureCache(const UnicodeString &path, Texture &texture) {
            if (textureCache.find(path) != textureCache.end()) {
                const TextureCache &tc = textureCache[path];
                texture.size = tc.size;
                texture.opaque = tc.opaque;
                texture.ok = true;
                return true;
            }
            return false;
        }
    };
    struct MapBuffer {
        MapBuffer(const BaseRenderContext *baseRenderContext)
            : baseRenderContextRef(baseRenderContext),
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
        const BaseRenderContext *baseRenderContextRef;
        uint8_t *address;
        size_t size;
        void *opaque;
    };

    BaseRenderContext(Scene *sceneRef, const StringMap *configRef);
    ~BaseRenderContext();

    void allocateUserData(const IModel *model, void *&context);
    void releaseUserData(const IModel *model, void *&context);
    bool uploadTexture(const IString *name, const IString *dir, Texture &texture, void *context);
    void getMatrix(float value[], const IModel *model, int flags) const;
    void log(void *context, LogLevel level, const char *format, va_list ap);
    IString *loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void *context);
    IString *loadShaderSource(ShaderType type, const IString *path);
    IString *loadKernelSource(KernelType type, void *context);
    IString *toUnicode(const uint8_t *str) const;
    bool hasExtension(const void *namePtr) const;
    void startProfileSession(ProfileType type, const void *arg);
    void stopProfileSession(ProfileType type, const void *arg);

#ifdef VPVL2_ENABLE_NVIDIA_CG
    typedef std::pair<RegexMatcher *, IEffect *> EffectAttachmentRule;
    typedef std::vector<EffectAttachmentRule> EffectAttachmentRuleList;
    class OffscreenTexture {
    public:
        OffscreenTexture(const IEffect::OffscreenRenderTarget &r,
                         const EffectAttachmentRuleList &a,
                         const Vector3 &size,
                         GLenum colorFormat,
                         GLenum colorInternalFormat,
                         GLenum colorType)
            : renderTarget(r),
              attachmentRules(a),
              colorTexture(size, colorFormat, colorInternalFormat, colorType),
              depthStencilBuffer(size, FrameBufferObject::detectDepthFormat(colorFormat))
        {
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
        FrameBufferObject::Texture2D colorTexture;
        FrameBufferObject::StandardRenderBuffer depthStencilBuffer;
    private:
        VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenTexture)
    };

    void getViewport(Vector3 &value) const;
    void getMousePosition(Vector4 &value, MousePositionType type) const;
    IModel *findModel(const IString *name) const;
    IModel *effectOwner(const IEffect *effect) const;
    void setEffectOwner(const IEffect *effectRef, IModel *model);
    void addModelPath(IModel *model, const UnicodeString &path);
    UnicodeString effectOwnerName(const IEffect *effect) const;
    FrameBufferObject *createFrameBufferObject();
    void getEffectCompilerArguments(Array<IString *> &arguments) const;
    const IString *effectFilePath(const IModel *model, const IString *dir) const;
    void addSharedTextureParameter(const char *name, const SharedTextureParameter &parameter);
    bool tryGetSharedTextureParameter(const char *name, SharedTextureParameter &parameter) const;
    void setMousePosition(const glm::vec2 &value, bool pressed, MousePositionType type);
    UnicodeString findModelPath(const IModel *model) const;
    FrameBufferObject *findFrameBufferObjectByRenderTarget(const IEffect::OffscreenRenderTarget &rt, bool enableAA);
    void bindOffscreenRenderTarget(const OffscreenTexture *texture, bool enableAA);
    void releaseOffscreenRenderTarget(const OffscreenTexture *texture, bool enableAA);
    void parseOffscreenSemantic(IEffect *effect, const IString *dir);
    void renderOffscreen();
    IEffect *createEffectRef(const IString *path);
    IEffect *createEffectRef(IModel *model, const IString *dir);
#else
    void addModelPath(IModel * /* model */, const UnicodeString & /* path */) {}
    void parseOffscreenSemantic(IEffect * /* effect */, const IString * /* dir */) {}
    void renderOffscreen() {}
    IEffect *createEffectRef(IModel * /* model */, const IString * /* dir */) { return 0; }
#endif /* VPVL2_ENABLE_NVIDIA_CG */

    void setArchive(Archive *value);
    void setSceneRef(Scene *value);
    void getCameraMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection);
    void setCameraMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection);
    void setViewport(const glm::vec2 &value);
    void updateCameraMatrices(const glm::vec2 &size);
    void createShadowMap(const Vector3 &size);
    void renderShadowMap();

    virtual bool mapFile(const UnicodeString &path, MapBuffer *buffer) const = 0;
    virtual bool unmapFile(MapBuffer *buffer) const = 0;
    virtual bool existsFile(const UnicodeString &path) const = 0;

protected:
    static const UnicodeString createPath(const IString *dir, const UnicodeString &name);
    static const UnicodeString createPath(const IString *dir, const IString *name);
    UnicodeString toonDirectory() const;
    UnicodeString shaderDirectory() const;
    UnicodeString effectDirectory() const;
    UnicodeString kernelDirectory() const;
    void generateMipmap(GLenum target) const;
    GLuint createTexture(const void *ptr, const Vector3 &size, GLenum format, GLenum type, bool mipmap, bool canOptimize) const;
    virtual bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context) = 0;

    const StringMap *m_configRef;
    Scene *m_sceneRef;
    Archive *m_archive;
    SimpleShadowMapSmartPtr m_shadowMap;
    glm::mat4x4 m_lightWorldMatrix;
    glm::mat4x4 m_lightViewMatrix;
    glm::mat4x4 m_lightProjectionMatrix;
    glm::mat4x4 m_cameraWorldMatrix;
    glm::mat4x4 m_cameraViewMatrix;
    glm::mat4x4 m_cameraProjectionMatrix;
    glm::vec2 m_viewport;
    std::set<std::string> m_extensions;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    typedef PointerHash<HashPtr, FrameBufferObject> RenderTargetMap;
    typedef PointerHash<HashString, IEffect> Path2EffectMap;
    typedef Hash<HashPtr, UnicodeString> ModelRef2PathMap;
    typedef Hash<HashPtr, IModel *> EffectRef2ModelRefMap;
    typedef Hash<HashPtr, UnicodeString> EffectRef2OwnerNameMap;
    typedef Hash<HashString, IModel *> Name2ModelRefMap;
    typedef PointerArray<OffscreenTexture> OffscreenTextureList;
    typedef std::pair<const CGcontext, const char *> SharedTextureParameterKey;
    typedef std::map<SharedTextureParameterKey, SharedTextureParameter> SharedTextureParameterMap;
    glm::vec4 m_mouseCursorPosition;
    glm::vec4 m_mouseLeftPressPosition;
    glm::vec4 m_mouseMiddlePressPosition;
    glm::vec4 m_mouseRightPressPosition;
    Path2EffectMap m_effectCaches;
    Name2ModelRefMap m_basename2modelRefs;
    ModelRef2PathMap m_modelRef2Paths;
    EffectRef2ModelRefMap m_effectRef2modelRefs;
    EffectRef2OwnerNameMap m_effectRef2owners;
    RenderTargetMap m_renderTargets;
    OffscreenTextureList m_offscreenTextures;
    SharedTextureParameterMap m_sharedParameters;
    mutable StringSmartPtr m_effectPathPtr;
    int m_msaaSamples;
#endif

private:
    void release();

#ifdef VPVL2_LINK_NVTT
    nv::Timer *getProfileTimer(ProfileType type) const;
    mutable PointerHash<HashInt, nv::Timer> m_profileTimers;
#endif /* VPVL2_LINK_NVTT */

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseRenderContext)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_BASERENDERCONTEXT_H_ */

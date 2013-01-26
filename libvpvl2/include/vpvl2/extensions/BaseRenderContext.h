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
#include <vpvl2/IBone.h>
#include <vpvl2/ICamera.h>
#include <vpvl2/IEffect.h>
#include <vpvl2/ILight.h>
#include <vpvl2/IModel.h>
#include <vpvl2/IRenderContext.h>
#include <vpvl2/IRenderEngine.h>
#include <vpvl2/Scene.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/gl/FrameBufferObject.h>
#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include <vpvl2/extensions/icu/StringMap.h>

/* STL */
#include <memory>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>

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

/* Cg and ICU */
#ifdef VPVL2_ENABLE_NVIDIA_CG
#include <vpvl2/extensions/cg/Util.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
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

namespace vpvl2
{
class Factory;
class IMotion;
class IRenderEngine;

namespace extensions
{
class World;
namespace icu {
class Encoding;
}
using namespace icu;

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
#endif

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
    struct ModelContext {
        TextureCacheMap textureCache;
        void addTextureCache(const UnicodeString &path, const TextureCache &cache) {
            textureCache.insert(std::make_pair(path, cache));
        }
        bool findTextureCache(const UnicodeString &path, Texture &texture) {
            if (textureCache.find(path) != textureCache.end()) {
                const TextureCache &tc = textureCache[path];
                texture.width = tc.width;
                texture.height = tc.height;
                texture.opaque = tc.id;
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

    BaseRenderContext(Scene *sceneRef, const StringMap *configRef)
        : m_configRef(configRef),
          m_sceneRef(sceneRef),
          m_lightWorldMatrix(1),
          m_lightViewMatrix(1),
          m_lightProjectionMatrix(1),
          m_cameraWorldMatrix(1),
          m_cameraViewMatrix(1),
          m_cameraProjectionMatrix(1)
    #ifdef VPVL2_ENABLE_NVIDIA_CG
        ,
          m_effectPathPtr(0),
          m_msaaSamples(0)
    #endif
    {
        std::istringstream in(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
        std::string extension;
        while (std::getline(in, extension, ' ')) {
            m_extensions.insert(extension);
        }
        // const GLubyte *shaderVersionString = glGetString(GL_SHADING_LANGUAGE_VERSION);
#ifdef VPVL2_ENABLE_NVIDIA_CG
        glGetIntegerv(GL_MAX_SAMPLES, &m_msaaSamples);
#endif
    }
    ~BaseRenderContext() {
        release();
    }

    void allocateUserData(const IModel * /* model */, void *&context) {
        ModelContext *ctx = new ModelContext();
        context = ctx;
    }
    void releaseUserData(const IModel * /* model */, void *&context) {
        delete static_cast<ModelContext *>(context);
        context = 0;
    }
    bool uploadTexture(const IString *name, const IString *dir, Texture &texture, void *context) {
        bool ret = false;
        if (texture.toon) {
            if (dir) {
                const UnicodeString &path = createPath(dir, name);
                std::cerr << "toon: " << String::toStdString(path) << std::endl;
                ret = uploadTextureInternal(path, texture, context);
            }
            if (!texture.ok) {
                String s(toonDirectory());
                const UnicodeString &path = createPath(&s, name);
                std::cerr << "system: " << String::toStdString(path) << std::endl;
                texture.system = true;
                ret = uploadTextureInternal(path, texture, context);
            }
        }
        else {
            const UnicodeString &path = createPath(dir, name);
            std::cerr << "texture: " << String::toStdString(path) << std::endl;
            ret = uploadTextureInternal(path, texture, context);
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
    IString *loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void * /* context */) {
        std::string file;
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (type == kModelEffectTechniques) {
            const IString *path = effectFilePath(model, dir);
            return loadShaderSource(type, path);
        }
#endif
        switch (model->type()) {
        case IModel::kAssetModel:
            file += "asset/";
            break;
        case IModel::kPMDModel:
        case IModel::kPMXModel:
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
        UnicodeString path = shaderDirectory();
        path.append("/");
        path.append(UnicodeString::fromUTF8(file));
        MapBuffer buffer(this);
        if (mapFile(path, &buffer)) {
            std::string bytes(buffer.address, buffer.address + buffer.size);
            return new(std::nothrow) String(UnicodeString::fromUTF8("#version 120\n" + bytes));
        }
        else {
            return 0;
        }
    }
    IString *loadShaderSource(ShaderType type, const IString *path) {
#ifdef VPVL2_ENABLE_NVIDIA_CG
        if (type == kModelEffectTechniques) {
            std::string bytes;
            MapBuffer buffer(this);
            if (path && mapFile(static_cast<const String *>(path)->value(), &buffer)) {
                uint8_t *address = buffer.address;
                bytes.assign(address, address + buffer.size);
            }
            else {
                UnicodeString defaultEffectPath = effectDirectory();
                defaultEffectPath.append("/");
                defaultEffectPath.append(UnicodeString::fromUTF8("base.cgfx"));
                if (mapFile(defaultEffectPath, &buffer)) {
                    uint8_t *address = buffer.address;
                    bytes.assign(address, address + buffer.size);
                }
            }
            return bytes.empty() ? 0 : new (std::nothrow) String(UnicodeString::fromUTF8(bytes));
        }
#endif
        return 0;
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
        UnicodeString path = kernelDirectory();
        path.append("/");
        path.append(UnicodeString::fromUTF8(file));
        MapBuffer buffer(this);
        if (mapFile(path, &buffer)) {
            std::string bytes(buffer.address, buffer.address + buffer.size);
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
    IModel *findModel(const IString *name) const {
        IModel *model = m_sceneRef->findModel(name);
        if (!model) {
            if (IModel *const *value = m_basename2modelRefs.find(name->toHashString())) {
                model = *value;
            }
        }
        return model;
    }
    IModel *effectOwner(const IEffect *effect) const {
        if (IModel *const *value = m_effectRef2modelRefs.find(effect))
            return *value;
        return 0;
    }
    void setEffectOwner(const IEffect *effectRef, IModel *model) {
        const IString *name = model->name();
        m_effectRef2owners.insert(effectRef, static_cast<const String *>(name)->value());
        m_effectRef2modelRefs.insert(effectRef, model);
    }
    void addModelPath(IModel *model, const UnicodeString &path) {
        if (model) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher filenameMatcher(".+/((.+)\\.\\w+)$", 0, status);
            filenameMatcher.reset(path);
            if (filenameMatcher.find()) {
                const UnicodeString &basename = filenameMatcher.group(1, status);
                if (!model->name()) {
                    String s(filenameMatcher.group(2, status));
                    model->setName(&s);
                }
                m_basename2modelRefs.insert(String::toStdString(basename).c_str(), model);
            }
            else {
                if (!model->name()) {
                    String s(path);
                    model->setName(&s);
                }
                m_basename2modelRefs.insert(String::toStdString(path).c_str(), model);
            }
            m_modelRef2Paths.insert(model, path);
        }
    }
    UnicodeString effectOwnerName(const IEffect *effect) const {
        if (const UnicodeString *value = m_effectRef2owners.find(effect)) {
            return *value;
        }
        return UnicodeString();
    }
    FrameBufferObject *createFrameBufferObject() {
        FrameBufferObjectSmartPtr fbo(new FrameBufferObject(m_msaaSamples));
        fbo->create();
        return fbo.release();
    }
    void getEffectCompilerArguments(Array<IString *> &arguments) const {
        arguments.clear();
    }
    const IString *effectFilePath(const IModel *model, const IString *dir) const {
        const UnicodeString &path = findModelPath(model);
        if (!path.isEmpty()) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher filenameMatcher("^.+/(.+)\\.\\w+$", 0, status);
            filenameMatcher.reset(path);
            const UnicodeString &s = filenameMatcher.find() ? filenameMatcher.group(1, status) : path;
            RegexMatcher extractMatcher("^.+\\[(.+)(?:\\.(?:cg)?fx)?\\]$", 0, status);
            extractMatcher.reset(s);
            const UnicodeString &cgfx = extractMatcher.find()
                    ? extractMatcher.replaceAll("$1.cgfx", status) : s + ".cgfx";
            const UnicodeString &newEffectPath = createPath(dir, cgfx);
            m_effectPathPtr.reset(existsFile(newEffectPath) ? new String(newEffectPath) : 0);
        }
        if (!m_effectPathPtr.get()) {
            m_effectPathPtr.reset(new String(createPath(dir, UnicodeString::fromUTF8("default.cgfx"))));
        }
        return m_effectPathPtr.get();
    }
    void addSharedTextureParameter(const char *name, const SharedTextureParameter &parameter) {
        CGcontext contextRef = static_cast<CGcontext>(parameter.context);
        SharedTextureParameterKey key(contextRef, name);
        m_sharedParameters.insert(std::make_pair(key, parameter));
    }
    bool tryGetSharedTextureParameter(const char *name, SharedTextureParameter &parameter) const {
        CGcontext contextRef = static_cast<CGcontext>(parameter.context);
        SharedTextureParameterKey key(contextRef, name);
        SharedTextureParameterMap::const_iterator it = m_sharedParameters.find(key);
        if (it != m_sharedParameters.end()) {
            parameter = it->second;
            return true;
        }
        return false;
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
    UnicodeString findModelPath(const IModel *model) const {
        if (const UnicodeString *value = m_modelRef2Paths.find(model)) {
            return *value;
        }
        return UnicodeString();
    }
    FrameBufferObject *findFrameBufferObjectByRenderTarget(const IEffect::OffscreenRenderTarget &rt, bool enableAA) {
        FrameBufferObjectSmartPtr buffer;
        if (const FrameBufferObject::AbstractTexture *textureRef = rt.textureRef) {
            if (FrameBufferObject *const *value = m_renderTargets.find(textureRef)) {
                buffer.reset(*value);
            }
            else {
                buffer.reset(new FrameBufferObject(enableAA ? m_msaaSamples : 0));
                buffer->create();
                m_renderTargets.insert(textureRef, buffer.get());
            }
        }
        return buffer.release();
    }
    void bindOffscreenRenderTarget(const OffscreenTexture *texture, bool enableAA) {
        static const GLuint buffers[] = { GL_COLOR_ATTACHMENT0 };
        static const int nbuffers = sizeof(buffers) / sizeof(buffers[0]);
        cg::Util::setRenderColorTargets(buffers, nbuffers);
        const IEffect::OffscreenRenderTarget &rt = texture->renderTarget;
        if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
            buffer->bindTexture(&texture->colorTexture, 0);
            buffer->bindDepthStencilBuffer(&texture->depthStencilBuffer);
        }
    }
    void releaseOffscreenRenderTarget(const OffscreenTexture *texture, bool enableAA) {
        const IEffect::OffscreenRenderTarget &rt = texture->renderTarget;
        if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
            buffer->transferMSAABuffer(0);
            buffer->unbindTexture(0);
            buffer->unbindDepthStencilBuffer();
            buffer->unbind();
            cg::Util::setRenderColorTargets(0, 0);
        }
    }
    void parseOffscreenSemantic(IEffect *effect, const IString *dir) {
        if (effect) {
            IEffectSmartPtr offscreenEffect;
            EffectAttachmentRuleList attachmentRules;
            std::string line;
            UErrorCode status = U_ZERO_ERROR;
            Vector3 size;
            RegexMatcher extensionMatcher("\\.(cg)?fx(sub)?$", 0, status),
                    pairMatcher("\\s*=\\s*", 0, status),
                    wildcardAllMatcher("\\*", 0, status),
                    wildcardCharacterMatcher("\\?", 0, status),
                    trimEmptyMatcher("\\\\Q\\\\E", 0, status);
            Array<IEffect::OffscreenRenderTarget> offscreenRenderTargets;
            effect->getOffscreenRenderTargets(offscreenRenderTargets);
            const int nOffscreenRenderTargets = offscreenRenderTargets.count();
            /* オフスクリーンレンダーターゲットの設定 */
            for (int i = 0; i < nOffscreenRenderTargets; i++) {
                const IEffect::OffscreenRenderTarget &renderTarget = offscreenRenderTargets[i];
                const CGparameter parameter = static_cast<const CGparameter>(renderTarget.textureParameter);
                const CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "DefaultEffect");
                std::istringstream stream(cgGetStringAnnotationValue(annotation));
                std::vector<UnicodeString> tokens(2);
                attachmentRules.clear();
                /* スクリプトを解析 */
                while (std::getline(stream, line, ';')) {
                    if (pairMatcher.split(UnicodeString::fromUTF8(line), &tokens[0], tokens.size(), status) == tokens.size()) {
                        const UnicodeString &value = tokens[1].trim();
                        UnicodeString key = "\\A\\Q" + tokens[0].trim() + "\\E\\z";
                        wildcardCharacterMatcher.reset(key);
                        wildcardAllMatcher.reset(wildcardCharacterMatcher.replaceAll("\\\\E.\\\\Q", status));
                        trimEmptyMatcher.reset(wildcardAllMatcher.replaceAll("\\\\E.*\\\\Q", status));
                        key = trimEmptyMatcher.replaceAll("", status);
                        RegexMatcherSmartPtr regexp(new RegexMatcher(key, 0, status));
                        /* self が指定されている場合は自身のエフェクトのファイル名を設定する */
                        if (key == "self") {
                            const UnicodeString &name = effectOwnerName(effect);
                            regexp->reset(name);
                        }
                        /* hide/none でなければオフスクリーン専用のモデルのエフェクト（オフスクリーン側が指定）を読み込む */
                        offscreenEffect.reset();
                        if (value != "hide" && value != "none") {
                            const UnicodeString &path = createPath(dir, value);
                            extensionMatcher.reset(path);
                            const String s2(extensionMatcher.replaceAll(".cgfx", status));
                            offscreenEffect.reset(createEffectRef(&s2));
                            offscreenEffect->setParentEffectRef(effect);
                        }
                        attachmentRules.push_back(EffectAttachmentRule(regexp.release(), offscreenEffect.release()));
                    }
                }
                if (!cg::Util::getSize2(parameter, size)) {
                    Vector3 viewport;
                    getViewport(viewport);
                    size.setX(btMax(size_t(1), size_t(viewport.x() * size.x())));
                    size.setY(btMax(size_t(1), size_t(viewport.y() * size.y())));
                }
                GLenum internal, format, type;
                cg::Util::getTextureFormat(parameter, internal, format, type);
                /* RenderContext 特有の OffscreenTexture に変換して格納 */
                m_offscreenTextures.add(new OffscreenTexture(renderTarget, attachmentRules, size, format, internal, type));
            }
        }
    }
    void renderOffscreen() {
        Array<IRenderEngine *> engines;
        m_sceneRef->getRenderEngineRefs(engines);
        const int nengines = engines.count();
        /* オフスクリーンレンダリングを行う前に元のエフェクトを保存する */
        Hash<HashPtr, IEffect *> effects;
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            if (IEffect *starndardEffect = engine->effect(IEffect::kStandard)) {
                effects.insert(engine, starndardEffect);
            }
            else if (IEffect *postEffect = engine->effect(IEffect::kPostProcess)) {
                effects.insert(engine, postEffect);
            }
            else if (IEffect *preEffect = engine->effect(IEffect::kPreProcess)) {
                effects.insert(engine, preEffect);
            }
            else {
                effects.insert(engine, 0);
            }
        }
        /* オフスクリーンレンダーターゲット毎にエフェクトを実行する */
        const int ntextures = m_offscreenTextures.count();
        for (int i = 0; i < ntextures; i++) {
            const OffscreenTexture *offscreenTexture = m_offscreenTextures[i];
            const IEffect::OffscreenRenderTarget &renderTarget = offscreenTexture->renderTarget;
            const CGparameter parameter = static_cast<CGparameter>(renderTarget.textureParameter);
            const CGannotation antiAlias = cgGetNamedParameterAnnotation(parameter, "AntiAlias");
            bool enableAA = false;
            /* セマンティクスから各種パラメータを設定 */
            if (cgIsAnnotation(antiAlias)) {
                int nvalues;
                const CGbool *values = cgGetBoolAnnotationValues(antiAlias, &nvalues);
                enableAA = nvalues > 0 ? values[0] == CG_TRUE : false;
            }
            /* オフスクリーンレンダリングターゲットを割り当ててレンダリング先をそちらに変更する */
            bindOffscreenRenderTarget(offscreenTexture, enableAA);
            const FrameBufferObject::AbstractTexture *texture = renderTarget.textureRef;
            const Vector3 &size = texture->size();
            updateCameraMatrices(glm::vec2(size.x(), size.y()));
            glViewport(0, 0, size.x(), size.y());
            const CGannotation clearColor = cgGetNamedParameterAnnotation(parameter, "ClearColor");
            if (cgIsAnnotation(clearColor)) {
                int nvalues;
                const float *color = cgGetFloatAnnotationValues(clearColor, &nvalues);
                if (nvalues == 4) {
                    glClearColor(color[0], color[1], color[2], color[3]);
                }
            }
            else {
                glClearDepth(1);
            }
            /* オフスクリーンレンダリングターゲットに向けてレンダリングを実行する */
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            for (int j = 0; j < nengines; j++) {
                IRenderEngine *engine = engines[j];
                if (engine->effect(IEffect::kPreProcess) || engine->effect(IEffect::kPostProcess))
                    continue;
                const IModel *model = engine->parentModelRef();
                const IString *name = model->name();
                const UnicodeString &n = name ? static_cast<const String *>(name)->value() : findModelPath(model);
                const EffectAttachmentRuleList &rules = offscreenTexture->attachmentRules;
                EffectAttachmentRuleList::const_iterator it2 = rules.begin();
                while (it2 != rules.end()) {
                    const EffectAttachmentRule &rule = *it2;
                    RegexMatcher *matcherRef = rule.first;
                    matcherRef->reset(n);
                    if (matcherRef->find()) {
                        IEffect *effectRef = rule.second;
                        engine->setEffect(IEffect::kStandardOffscreen, effectRef, 0);
                        break;
                    }
                    ++it2;
                }
                engine->update();
                engine->renderModel();
                engine->renderEdge();
            }
            /* オフスクリーンレンダリングターゲットの割り当てを解除 */
            releaseOffscreenRenderTarget(offscreenTexture, enableAA);
        }
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            IEffect *const *effect = effects.find(engine);
            engine->setEffect(IEffect::kAutoDetection, *effect, 0);
        }
    }
    IEffect *createEffectRef(const IString *path) {
        IEffect *effectRef = 0;
        if (IEffect *const *value = m_effectCaches.find(path->toHashString())) {
            effectRef = *value;
        }
        else if (existsFile(static_cast<const String *>(path)->value())) {
            IEffectSmartPtr effectPtr(m_sceneRef->createEffectFromFile(path, this));
            if (!effectPtr.get() || !effectPtr->internalPointer()) {
                std::cerr << path->toByteArray() << " cannot be compiled" << std::endl;
                std::cerr << cgGetLastListing(static_cast<CGcontext>(effectPtr->internalContext())) << std::endl;
            }
            else {
                effectRef = effectPtr.get();
                m_effectCaches.insert(path->toHashString(), effectPtr.release());
            }
        }
        return effectRef;
    }
    IEffect *createEffectRef(IModel *model, const IString *dir) {
        const UnicodeString &pathForKey = static_cast<const String *>(effectFilePath(model, dir))->value();
        const String s(pathForKey);
        IEffect *effectRef = createEffectRef(&s);
        if (effectRef) {
            setEffectOwner(effectRef, model);
            // const IString *name = model->name();
            // std::cout << "Loaded an model effect for " << (name ? name->toByteArray() : "") << std::endl;
        }
        return effectRef;
    }
#else
    void addModelPath(IModel * /* model */, const UnicodeString & /* path */) {}
    void parseOffscreenSemantic(IEffect * /* effect */, const IString * /* dir */) {}
    void renderOffscreen() {}
    IEffect *createEffectRef(IModel * /* model */, const IString * /* dir */) { return 0; }
#endif

    void setArchive(ArchiveSmartPtr value) {
        m_archive = value;
    }
    void setSceneRef(Scene *value) {
        release();
        m_sceneRef = value;
    }
    void getCameraMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection) {
        world = m_cameraWorldMatrix;
        view = m_cameraViewMatrix;
        projection = m_cameraProjectionMatrix;
    }
    void setCameraMatrces(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection) {
        m_cameraWorldMatrix = world;
        m_cameraViewMatrix = view;
        m_cameraProjectionMatrix = projection;
    }
    void setViewport(const glm::vec2 &value) {
        m_viewport = value;
    }
    void updateCameraMatrices(const glm::vec2 &size) {
        const ICamera *camera = m_sceneRef->camera();
        Scalar matrix[16];
        camera->modelViewTransform().getOpenGLMatrix(matrix);
        const glm::mediump_float &aspect = size.x / size.y;
        const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
                &projection = glm::infinitePerspective(camera->fov(), aspect, camera->znear());
        setCameraMatrces(world, view, projection);
        setViewport(size);
    }
    void createShadowMap(const Vector3 &size) {
        if (Scene::isSelfShadowSupported()) {
            m_shadowMap.reset(new SimpleShadowMap(size.x(), size.y()));
            m_shadowMap->create();
            m_sceneRef->setShadowMapRef(m_shadowMap.get());
        }
    }
    void renderShadowMap() {
        if (m_shadowMap.get()) {
            m_shadowMap->bind();
            const Vector3 &size = m_shadowMap->size();
            glViewport(0, 0, size.x(), size.y());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Array<IRenderEngine *> engines;
            m_sceneRef->getRenderEngineRefs(engines);
            const int nengines = engines.count();
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->renderZPlot();
            }
            m_shadowMap->unbind();
        }
    }

    virtual bool mapFile(const UnicodeString &path, MapBuffer *buffer) const = 0;
    virtual bool unmapFile(MapBuffer *buffer) const = 0;
    virtual bool existsFile(const UnicodeString &path) const = 0;

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
    UnicodeString toonDirectory() const {
        return m_configRef->value("dir.system.toon", UnicodeString("../../VPVM/resources/images"));
    }
    UnicodeString shaderDirectory() const {
        return m_configRef->value("dir.system.shaders", UnicodeString("../../VPVM/resources/shaders"));
    }
    UnicodeString effectDirectory() const {
        return m_configRef->value("dir.system.effects", UnicodeString("../../VPVM/resources/effects"));
    }
    UnicodeString kernelDirectory() const {
        return m_configRef->value("dir.system.kernels", UnicodeString("../../VPVM/resources/kernels"));
    }

    void generateMipmap(GLenum target) const {
#ifdef VPVL2_LINK_GLEW
        if (GLEW_ARB_framebuffer_object)
            glGenerateMipmap(target);
#else
        const void *procs[] = { "glGenerateMipmap", "glGenerateMipmapEXT", 0 };
        typedef void (*glGenerateMipmapProcPtr)(GLuint);
        if (glGenerateMipmapProcPtr glGenerateMipmapProcPtrRef = reinterpret_cast<glGenerateMipmapProcPtr>(findProcedureAddress(procs)))
            glGenerateMipmapProcPtrRef(target);
#endif
    }
    GLuint createTexture(const void *ptr, const Vector3 &size, GLenum format, GLenum type, bool mipmap, bool canOptimize) const {
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x(), size.y(), 0, format, type, ptr);
        if (mipmap)
            generateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }

    virtual bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context) = 0;

    const StringMap *m_configRef;
    Scene *m_sceneRef;
    ArchiveSmartPtr m_archive;
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
    typedef Hash<HashPtr, UnicodeString> ModelRef2PathMap;
    typedef Hash<HashPtr, IModel *> EffectRef2ModelRefMap;
    typedef Hash<HashPtr, UnicodeString> EffectRef2OwnerNameMap;
    typedef Hash<HashPtr, FrameBufferObject *> RenderTargetMap;
    typedef Hash<HashString, IEffect *> Path2EffectMap;
    typedef Hash<HashString, IModel *> Name2ModelRefMap;
    typedef Array<OffscreenTexture *> OffscreenTextureList;
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
    void release() {
        m_sceneRef = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
        m_effectCaches.releaseAll();
        m_offscreenTextures.releaseAll();
        m_renderTargets.releaseAll();
        m_effectCaches.clear();
        m_renderTargets.clear();
        m_basename2modelRefs.clear();
        m_modelRef2Paths.clear();
        m_effectRef2modelRefs.clear();
        m_effectRef2owners.clear();
        m_sharedParameters.clear();
        m_effectPathPtr.reset();
#endif
    }

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseRenderContext)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_BASERENDERCONTEXT_H_ */
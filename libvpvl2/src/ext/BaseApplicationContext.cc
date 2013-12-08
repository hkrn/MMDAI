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

#include <vpvl2/extensions/BaseApplicationContext.h>

/* libvpvl2 */
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/fx/Util.h>
#include <vpvl2/extensions/gl/FrameBufferObject.h>
#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include <vpvl2/extensions/gl/Texture2D.h>
#include <vpvl2/extensions/icu4c/StringMap.h>

#ifdef VPVL2_ENABLE_EXTENSION_ARCHIVE
#include <vpvl2/extensions/Archive.h>
#endif
//#ifdef VPVL2_LINK_ATB
#if 1
#include <AntTweakBar.h>
#else
#define TwInit(graphAPI, device)
#define TwDraw()
#define TwTerminate()
#define TwWindowSize(width, height)
#define TwNewBar(barName) 0
#define TwDeleteAllBars()
#define TwSetParam(bar, varName, paramName, paramValueType, inValueCount, inValues)
#define TwAddVarCB(bar, name, type, setCallback, getCallback, clientData, def)
#define TwMouseButton(action, button) 0
#define TwMouseMotion(mouseX, mouseY) 0
#define TwMouseWheel(pos) 0
#define TwKeyPressed(key, modifiers) 0
static const int TW_OPENGL = 0;
static const int TW_OPENGL_CORE = 1;
static const int TW_MOUSE_PRESSED = 1;
static const int TW_MOUSE_RELEASED = 2;
static const int TW_TYPE_BOOLCPP = 1;
static const int TW_TYPE_INT32 = 2;
static const int TW_TYPE_FLOAT = 3;
static const int TW_TYPE_CSTRING = 4;
typedef void TwBar;
typedef unsigned int ETwMouseAction;
#endif

/* STL */
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

/* Simple OpenGL Image Library */
#include "stb_image_aug.h"

/* FreeImage */
#ifdef VPVL2_LINK_FREEIMAGE
#include <FreeImage.h>
#else
#define FreeImage_Initialise()
#define FreeImage_DeInitialise()
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/* GLM */
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/string_cast.hpp>

/* Cg and ICU */
#include <unicode/udata.h>
#if defined(VPVL2_LINK_NVFX)
#include <FxLib.h>
#elif defined(VPVL2_ENABLE_NVIDIA_CG)
#include <vpvl2/extensions/fx/Util.h>
#include <unicode/regex.h>
#endif

#if defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS)
#include <sys/fcntl.h>
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::gl;

namespace {

#include "ICUCommonData.inl"

static const vpvl2::extensions::gl::GLenum kGL_DONT_CARE = 0x1100;
static const vpvl2::extensions::gl::GLenum kGL_MAX_SAMPLES = 0x8D57;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_API_ARB = 0x8246;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB = 0x8247;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_SHADER_COMPILER_ARB = 0x8248;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_THIRD_PARTY_ARB = 0x8249;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_APPLICATION_ARB = 0x824A;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SOURCE_OTHER_ARB = 0x824B;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_ERROR_ARB = 0x824C;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB = 0x824D;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB = 0x824E;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_PORTABILITY_ARB = 0x824F;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_PERFORMANCE_ARB = 0x8250;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_TYPE_OTHER_ARB = 0x8251;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SEVERITY_HIGH_ARB = 0x9146;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SEVERITY_MEDIUM_ARB = 0x9147;
static const vpvl2::extensions::gl::GLenum kGL_DEBUG_SEVERITY_LOW_ARB = 0x9148;

class EffectParameterUIBuilder {
public:
    static TwBar *createBar(IEffect *effectRef) {
        const char *effectName = reinterpret_cast<const char *>(effectRef->name()->toByteArray());
        TwBar *bar = TwNewBar(effectName);
        TwSetParam(bar, 0, "valueswidth", TW_PARAM_CSTRING, 1, "fit");
        TwAddVarCB(bar, "Enabled", TW_TYPE_BOOLCPP, setEnableEffect, getEnableEffect, effectRef, (std::string("help='Toggle Enable/Disable Effect of ") + effectName + "'").c_str());
        return bar;
    }
    static void createBoolean(TwBar *bar, IEffect::Parameter *parameterRef, std::ostringstream &stream) {
        TwAddVarCB(bar, parameterRef->annotationRef("UIName")->stringValue(), TW_TYPE_BOOLCPP, setBooleanParameter, getBooleanParameter, parameterRef, stream.str().c_str());
    }
    static void createInteger(TwBar *bar, IEffect::Parameter *parameterRef, std::ostringstream &stream) {
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIMin")) {
            stream << "min=" << annotationRef->integerValue() << " ";
        }
        else {
            stream << "min=0 ";
        }
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIMax")) {
            stream << "max=" << annotationRef->integerValue() << " ";
        }
        TwAddVarCB(bar, parameterRef->annotationRef("UIName")->stringValue(), TW_TYPE_INT32, setIntegerParameter, getIntegerParameter, parameterRef, stream.str().c_str());
    }
    static void createFloat(TwBar *bar, IEffect::Parameter *parameterRef, std::ostringstream &stream) {
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIMin")) {
            stream << "min=" << annotationRef->floatValue() << " ";
        }
        else {
            stream << "min=0.0 ";
        }
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIMax")) {
            stream << "max=" << annotationRef->floatValue() << " ";
        }
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIStep")) {
            stream << "step=" << annotationRef->floatValue() << " ";
        }
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIPrecision")) {
            stream << "precision=" << annotationRef->integerValue() << " ";
        }
        TwAddVarCB(bar, parameterRef->annotationRef("UIName")->stringValue(), TW_TYPE_FLOAT, setFloatParameter, getFloatParameter, parameterRef, stream.str().c_str());
    }
    static void setCommonDefinition(const IEffect::Parameter *parameterRef, std::ostringstream &stream) {
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIHelp")) {
            stream << "help='" << annotationRef->stringValue() << "' ";
        }
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("UIVisible")) {
            stream << "visible='" << annotationRef->booleanValue() << "' ";
        }
    }

private:
    static void getEnableEffect(void *value, void *userData) {
        const IEffect *effectRef = static_cast<const IEffect *>(userData);
        *static_cast<bool *>(value) = effectRef->isEnabled();
    }
    static void setEnableEffect(const void *value, void *userData) {
        IEffect *parameterRef = static_cast<IEffect *>(userData);
        parameterRef->setEnabled(*static_cast<const bool *>(value));
    }
    static void getBooleanParameter(void *value, void *userData) {
        const IEffect::Parameter *parameterRef = static_cast<const IEffect::Parameter *>(userData);
        int bv = 0;
        parameterRef->getValue(bv);
        *static_cast<bool *>(value) = bv != 0;
    }
    static void setBooleanParameter(const void *value, void *userData) {
        IEffect::Parameter *parameterRef = static_cast<IEffect::Parameter *>(userData);
        bool bv = *static_cast<const bool *>(value);
        parameterRef->setValue(bv);
    }
    static void getIntegerParameter(void *value, void *userData) {
        const IEffect::Parameter *parameterRef = static_cast<const IEffect::Parameter *>(userData);
        int iv = 0;
        parameterRef->getValue(iv);
        *static_cast<int *>(value) = iv;
    }
    static void setIntegerParameter(const void *value, void *userData) {
        IEffect::Parameter *parameterRef = static_cast<IEffect::Parameter *>(userData);
        int iv = *static_cast<const int *>(value);
        parameterRef->setValue(iv);
    }
    static void getFloatParameter(void *value, void *userData) {
        const IEffect::Parameter *parameterRef = static_cast<const IEffect::Parameter *>(userData);
        float fv = 0;
        parameterRef->getValue(fv);
        *static_cast<float *>(value) = fv;
    }
    static void setFloatParameter(const void *value, void *userData) {
        IEffect::Parameter *parameterRef = static_cast<IEffect::Parameter *>(userData);
        float fv = *static_cast<const float *>(value);
        parameterRef->setValue(fv);
    }

    VPVL2_MAKE_STATIC_CLASS(EffectParameterUIBuilder)
};

static inline const char *DebugMessageSourceToString(vpvl2::extensions::gl::GLenum value)
{
    switch (value) {
    case kGL_DEBUG_SOURCE_API_ARB:
        return "OpenGL";
    case kGL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        return "Window";
    case kGL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        return "ShaderCompiler";
    case kGL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        return "ThirdParty";
    case kGL_DEBUG_SOURCE_APPLICATION_ARB:
        return "Application";
    case kGL_DEBUG_SOURCE_OTHER_ARB:
        return "Other";
    default:
        return "Unknown";
    }
}

static inline const char *DebugMessageTypeToString(vpvl2::extensions::gl::GLenum value)
{
    switch (value) {
    case kGL_DEBUG_TYPE_ERROR_ARB:
        return "Error";
    case kGL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        return "DeprecatedBehavior";
    case kGL_DEBUG_TYPE_PORTABILITY_ARB:
        return "Portability";
    case kGL_DEBUG_TYPE_PERFORMANCE_ARB:
        return "Performance";
    case kGL_DEBUG_TYPE_OTHER_ARB:
        return "Other";
    default:
        return "Unknown";
    }
}

static inline IString *toIStringFromUtf8(const std::string &bytes)
{
#ifdef _MSC_VER /* workaround for unexpected bogus string from UnicodeString::fromUTF8 on MSVC build */
    UnicodeString s;
    int32_t length = int32_t(bytes.length()), length16;
    UChar *utf16 = s.getBuffer(length + 1);
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strFromUTF8WithSub(utf16, s.getCapacity(), &length16, bytes.data(), length, 0xfffd, 0, &errorCode);
    s.releaseBuffer(length16);
    return bytes.empty() ? 0 : new (std::nothrow) String(s);
#else
    return bytes.empty() ? 0 : new (std::nothrow) String(UnicodeString::fromUTF8(bytes));
#endif
}

#if defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS)

static char g_crashHandlePath[PATH_MAX];

static void HandleFailure(const char *data, int size)
{
    int fd = ::open(g_crashHandlePath, O_WRONLY | O_APPEND | O_CREAT);
    if (fd != -1) {
        ::write(fd, data, size);
        ::close(fd);
    }
}

static void InstallFailureHandler(const char *logdir)
{
    google::InstallFailureSignalHandler();
    google::InstallFailureWriter(HandleFailure);
    static const char kFailureLogFilename[] = "/failure.log";
    internal::snprintf(g_crashHandlePath, sizeof(g_crashHandlePath), "%s/%s", logdir, kFailureLogFilename);
}

#else
#define InstallFailureHandler(logdir)
#endif

} /* namespace anonymous */

namespace vpvl2
{
namespace extensions
{
using namespace gl;
using namespace icu4c;

BaseApplicationContext::ModelContext::ModelContext(BaseApplicationContext *applicationContextRef, vpvl2::extensions::Archive *archiveRef, const IString *directory)
    : pixelStorei(reinterpret_cast<PFNGLPIXELSTOREIPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glPixelStorei"))),
      m_directoryRef(directory),
      m_archiveRef(archiveRef),
      m_applicationContextRef(applicationContextRef),
      m_maxAnisotropyValue(0)
{
    IApplicationContext::FunctionResolver *resolver = applicationContextRef->sharedFunctionResolverInstance();
    if (resolver->hasExtension("EXT_texture_filter_anisotropic")) {
        typedef void (GLAPIENTRY * PFNGLGETFLOATVPROC)(GLenum pname, GLfloat *values);
        reinterpret_cast<PFNGLGETFLOATVPROC>(resolver->resolveSymbol("glGetFloatv"))(BaseTexture::kGL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropyValue);
    }
}

BaseApplicationContext::ModelContext::~ModelContext()
{
    m_archiveRef = 0;
    m_applicationContextRef = 0;
    m_directoryRef = 0;
}

void BaseApplicationContext::ModelContext::addTextureCache(const std::string &path, ITexture *textureRef)
{
    if (textureRef) {
        m_textureRefCache.insert(std::make_pair(path, textureRef));
    }
}

bool BaseApplicationContext::ModelContext::findTexture(const std::string &path, TextureDataBridge &bridge) const
{
    VPVL2_DCHECK(!path.empty());
    TextureCacheMap::const_iterator it = m_textureRefCache.find(path);
    if (it != m_textureRefCache.end()) {
        bridge.dataRef = it->second;
        return true;
    }
    return false;
}

bool BaseApplicationContext::ModelContext::storeTexture(const std::string &key, ITexture *textureRef, TextureDataBridge &bridge)
{
    VPVL2_DCHECK(!key.empty());
    bool ok = textureRef != 0;
    if (textureRef) {
        pushAnnotationGroup("BaseApplicationContext::ModelContext#cacheTexture", m_applicationContextRef->sharedFunctionResolverInstance());
        textureRef->bind();
        textureRef->setParameter(BaseTexture::kGL_TEXTURE_MAG_FILTER, int(BaseTexture::kGL_LINEAR));
        textureRef->setParameter(BaseTexture::kGL_TEXTURE_MIN_FILTER, int(BaseTexture::kGL_LINEAR));
        if (internal::hasFlagBits(bridge.flags, IApplicationContext::kToonTexture)) {
            textureRef->setParameter(BaseTexture::kGL_TEXTURE_WRAP_S, int(BaseTexture::kGL_CLAMP_TO_EDGE));
            textureRef->setParameter(BaseTexture::kGL_TEXTURE_WRAP_T, int(BaseTexture::kGL_CLAMP_TO_EDGE));
        }
        if (m_maxAnisotropyValue > 0) {
            textureRef->setParameter(BaseTexture::kGL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropyValue);
        }
        textureRef->unbind();
        bridge.dataRef = textureRef;
        annotateObject(BaseTexture::kGL_TEXTURE, textureRef->data(), ("key=" + key).c_str(), m_applicationContextRef->sharedFunctionResolverInstance());
        addTextureCache(key, textureRef);
        popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    }
    return ok;
}

int BaseApplicationContext::ModelContext::countTextures() const
{
    return int(m_textureRefCache.size());
}

ITexture *BaseApplicationContext::ModelContext::createTexture(const void *ptr, const BaseSurface::Format &format, const Vector3 &size, bool /* mipmap */) const
{
    VPVL2_DCHECK(ptr);
    FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    pushAnnotationGroup("BaseApplicationContext::ModelContext#createTexture", resolver);
    Texture2D *texture = new (std::nothrow) Texture2D(resolver, format, size, 0);
    if (texture) {
        texture->create();
        texture->bind();
        texture->fillPixels(ptr);
        texture->generateMipmaps();
        texture->unbind();
    }
    popAnnotationGroup(resolver);
    return texture;
}

ITexture *BaseApplicationContext::ModelContext::createTexture(const uint8 *data, vsize size, bool mipmap)
{
    VPVL2_DCHECK(data && size > 0);
    Vector3 textureSize;
    ITexture *texturePtr = 0;
    int x = 0, y = 0, ncomponents = 0;
#ifdef VPVL2_LINK_FREEIMAGE
    FIMEMORY *memory = FreeImage_OpenMemory(const_cast<uint8_t *>(data), size);
    FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memory);
    if (format != FIF_UNKNOWN) {
        if (FIBITMAP *bitmap = FreeImage_LoadFromMemory(format, memory)) {
            if (FIBITMAP *bitmap32 = FreeImage_ConvertTo32Bits(bitmap)) {
                FreeImage_FlipVertical(bitmap32);
                textureSize.setValue(FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32), 1);
                texturePtr = uploadTexture(FreeImage_GetBits(bitmap32), m_applicationContextRef->textureFormat(), textureSize, mipmap, false);
                FreeImage_Unload(bitmap);
                FreeImage_Unload(bitmap32);
                return texturePtr;
            }
            else {
                VPVL2_LOG(WARNING, "Cannot convert loaded image to 32bits image");
            }
            FreeImage_Unload(bitmap);
        }
        else {
            VPVL2_LOG(WARNING, "Cannot decode the image");
        }
    }
    else {
        VPVL2_LOG(WARNING, "Cannot detect image format");
    }
    FreeImage_CloseMemory(memory);
#endif
    /* Loading major image format (BMP/JPG/PNG/TGA/DDS) texture with stb_image.c */
    if (stbi_uc *ptr = stbi_load_from_memory(data, int(size), &x, &y, &ncomponents, 4)) {
        textureSize.setValue(Scalar(x), Scalar(y), 1);
        texturePtr = createTexture(ptr, m_applicationContextRef->defaultTextureFormat(), textureSize, mipmap);
        stbi_image_free(ptr);
    }
    return texturePtr;
}

void BaseApplicationContext::ModelContext::optimizeTexture(ITexture *texture)
{
    FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    texture->bind();
    if (resolver->hasExtension("APPLE_texture_range")) {
        texture->setParameter(kGL_TEXTURE_STORAGE_HINT_APPLE, int(kGL_STORAGE_CACHED_APPLE));
    }
    if (resolver->hasExtension("APPLE_client_storage")) {
        pixelStorei(kGL_UNPACK_CLIENT_STORAGE_APPLE, kGL_TRUE);
    }
    texture->unbind();
}

Archive *BaseApplicationContext::ModelContext::archiveRef() const
{
    return m_archiveRef;
}

const IString *BaseApplicationContext::ModelContext::directoryRef() const
{
    return m_directoryRef;
}

bool BaseApplicationContext::ModelContext::uploadTexture(const std::string &path, TextureDataBridge &bridge)
{
    VPVL2_DCHECK(!path.empty());
    if (path[path.length() - 1] == '/') {
        VPVL2_VLOG(2, path << " is the directory, skipped.");
        return true;
    }
    else if (findTexture(path, bridge)) {
        VPVL2_VLOG(2, path << " is already cached, skipped.");
        return true;
    }
    ITexture *texturePtr = 0;
    MapBuffer buffer(m_applicationContextRef);
    /* Loading major image format (BMP/JPG/PNG/TGA/DDS) texture with stb_image.c */
    if (m_applicationContextRef->mapFile(path, &buffer)) {
        texturePtr = createTexture(buffer.address, buffer.size, internal::hasFlagBits(bridge.flags, IApplicationContext::kGenerateTextureMipmap));
        if (!texturePtr) {
            VPVL2_LOG(WARNING, "Cannot load texture from " << path << ": " << stbi_failure_reason());
            return false;
        }
    }
    return storeTexture(path, texturePtr, bridge);
}

bool BaseApplicationContext::ModelContext::uploadTexture(const uint8 *data, vsize size, const std::string &key, TextureDataBridge &bridge)
{
    VPVL2_DCHECK(data && size > 0);
    if (findTexture(key, bridge)) {
        VPVL2_VLOG(2, key << " is already cached, skipped.");
        return true;
    }
    ITexture *texturePtr = createTexture(data, size, internal::hasFlagBits(bridge.flags, IApplicationContext::kGenerateTextureMipmap));
    if (!texturePtr) {
        VPVL2_LOG(WARNING, "Cannot load texture with key " << key << ": " << stbi_failure_reason());
        return false;
    }
    return storeTexture(key, texturePtr, bridge);
}

bool BaseApplicationContext::initializeOnce(const char *argv0, const char *logdir, int vlog)
{
    VPVL2_CHECK(argv0);
    google::InitGoogleLogging(argv0);
    InstallFailureHandler(logdir);
    FreeImage_Initialise();
    FLAGS_v = vlog;
    if (logdir) {
        FLAGS_stop_logging_if_full_disk = true;
        FLAGS_log_dir = logdir;
#ifndef NDEBUG
        FLAGS_alsologtostderr = true;
#endif
    }
    else {
        google::LogToStderr();
        FLAGS_logtostderr = true;
        FLAGS_colorlogtostderr = true;
    }
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(g_icudt52l_dat, &err);
    return err == U_ZERO_ERROR;
}

void BaseApplicationContext::terminate()
{
    FreeImage_DeInitialise();
    TwTerminate();
    Scene::terminate();
    google::ShutdownGoogleLogging();
}

BaseApplicationContext::BaseApplicationContext(Scene *sceneRef, IEncoding *encodingRef, const StringMap *configRef)
    : getIntegerv(0),
      viewport(0),
      clear(0),
      clearColor(0),
      clearDepth(0),
      m_configRef(configRef),
      m_sceneRef(sceneRef),
      m_encodingRef(encodingRef),
      m_currentModelRef(0),
      m_renderColorFormat(kGL_RGBA, kGL_RGBA8, kGL_UNSIGNED_BYTE, Texture2D::kGL_TEXTURE_2D),
      m_lightWorldMatrix(1),
      m_lightViewMatrix(1),
      m_lightProjectionMatrix(1),
      m_cameraWorldMatrix(1),
      m_cameraViewMatrix(1),
      m_cameraProjectionMatrix(1),
      m_aspectRatio(1),
      m_samplesMSAA(0),
      m_viewportRegionInvalidated(false)
{
}

void BaseApplicationContext::initialize(bool enableDebug)
{
    FunctionResolver *resolver = sharedFunctionResolverInstance();
    pushAnnotationGroup("BaseApplicationContext#initialize", resolver);
    getIntegerv = reinterpret_cast<PFNGLGETINTEGERVPROC>(resolver->resolveSymbol("glGetIntegerv"));
    viewport = reinterpret_cast<PFNGLVIEWPORTPROC>(resolver->resolveSymbol("glViewport"));
    clear = reinterpret_cast<PFNGLCLEARPROC>(resolver->resolveSymbol("glClear"));
    clearColor = reinterpret_cast<PFNGLCLEARCOLORPROC>(resolver->resolveSymbol("glClearColor"));
    clearDepth = reinterpret_cast<PFNGLCLEARDEPTHPROC>(resolver->resolveSymbol("glClearDepth"));
    if (enableDebug && resolver->hasExtension("ARB_debug_output")) {
        typedef void (GLAPIENTRY * GLDEBUGPROCARB) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
        typedef void (GLAPIENTRY * PFNGLENABLEPROC) (GLenum cap);
        typedef void (GLAPIENTRY * PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
        typedef void (GLAPIENTRY * PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB callback, void* userParam);
        reinterpret_cast<PFNGLENABLEPROC>(resolver->resolveSymbol("glEnable"))(kGL_DEBUG_OUTPUT_SYNCHRONOUS);
        reinterpret_cast<PFNGLDEBUGMESSAGECONTROLARBPROC>(resolver->resolveSymbol("glDebugMessageControlARB"))(kGL_DONT_CARE, kGL_DONT_CARE, kGL_DONT_CARE, 0, 0, kGL_TRUE);
        reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKARBPROC>(resolver->resolveSymbol("glDebugMessageCallbackARB"))(reinterpret_cast<GLDEBUGPROCARB>(&BaseApplicationContext::debugMessageCallback), this);
    }
    getIntegerv(kGL_MAX_SAMPLES, &m_samplesMSAA);
    TwInit(resolver->query(FunctionResolver::kQueryCoreProfile) != 0 ? TW_OPENGL_CORE : TW_OPENGL, 0);
    popAnnotationGroup(resolver);
}

BaseApplicationContext::~BaseApplicationContext()
{
    m_encodingRef = 0;
    /* m_samplesMSAA must not set zero at #release(), it causes multiple post effect will be lost */
    m_samplesMSAA = 0;
}

void BaseApplicationContext::release()
{
    pushAnnotationGroup("BaseApplicationContext#release", sharedFunctionResolverInstance());
    m_shadowMap.reset();
    m_currentModelRef = 0;
    m_offscreenTextures.releaseAll();
    m_renderTargets.releaseAll();
    m_basename2ModelRefs.clear();
    m_modelRef2Paths.clear();
    m_sharedParameters.clear();
    m_offscreenTechniques.clear();
    m_effectRef2ModelRefs.clear();
    m_effectRef2Owners.clear();
    m_effectRef2Paths.clear();
    m_effectRef2ParameterUIs.clear();
    TwDeleteAllBars();
    m_effectCaches.releaseAll();
    popAnnotationGroup(sharedFunctionResolverInstance());
}

bool BaseApplicationContext::uploadTexture(const IString *name, TextureDataBridge &bridge, void *userData)
{
    bool ret = false;
    bridge.dataRef = 0;
    ModelContext *context = static_cast<ModelContext *>(userData);
    std::string newName = static_cast<const String *>(name)->toStdString();
    std::string::size_type pos(newName.find('\\'));
    while (pos != std::string::npos) {
        newName.replace(pos, 1, "/");
        pos = newName.find('\\', pos + 1);
    }
    if (internal::hasFlagBits(bridge.flags, IApplicationContext::kToonTexture)) {
        if (!internal::hasFlagBits(bridge.flags, IApplicationContext::kSystemToonTexture)) {
            /* it's directory if name2.empty() is true */
            if (newName.empty()) {
                const std::string &newToonPath = toonDirectory() + "/toon0.bmp";
                if (!context->findTexture(newToonPath, bridge)) {
                    /* uses default system texture loader */
                    VPVL2_VLOG(2, "Try loading a system default toon texture from archive: " << newToonPath);
                    ret = context->uploadTexture(newToonPath, bridge);
                }
            }
            else if (context->archiveRef()) {
                VPVL2_VLOG(2, "Try loading a model toon texture from archive: " << newName);
                ret = internalUploadTexture(newName, std::string(), bridge, context);
            }
            else if (const IString *directoryRef = context->directoryRef()) {
                const std::string &path = static_cast<const String *>(directoryRef)->toStdString()
                        + "/" + static_cast<const String *>(name)->toStdString();
                VPVL2_VLOG(2, "Try loading a model toon texture: " << path);
                ret = internalUploadTexture(newName, path, bridge, context);
            }
        }
        if (!ret) {
            bridge.flags |= IApplicationContext::kSystemToonTexture;
            VPVL2_VLOG(2, "Loading a system default toon texture: " << newName);
            ret = uploadSystemToonTexture(newName, bridge, context);
        }
    }
    else if (const IString *directoryRef = context->directoryRef()) {
        const std::string &path = static_cast<const String *>(directoryRef)->toStdString() + "/" + newName;
        VPVL2_VLOG(2, "Loading a model texture: " << path);
        ret = internalUploadTexture(newName, path, bridge, context);
    }
    return ret;
}

bool BaseApplicationContext::uploadSystemToonTexture(const std::string &name, TextureDataBridge &bridge, ModelContext *context)
{
    MapBuffer buffer(this);
    const std::string &path = toonDirectory() + "/" + name;
    /* open a (system) toon texture from library resource */
    return mapFile(path, &buffer) ? context->uploadTexture(buffer.address, buffer.size, path, bridge) : false;
}

bool BaseApplicationContext::internalUploadTexture(const std::string &name, const std::string &path, TextureDataBridge &bridge, ModelContext *context)
{
    if (!internal::hasFlagBits(bridge.flags, IApplicationContext::kSystemToonTexture)) {
        if (Archive *archiveRef = context->archiveRef()) {
            archiveRef->uncompressEntry(name);
            VPVL2_LOG(INFO, name);
            if (const std::string *bytesRef = archiveRef->dataRef(name)) {
                const uint8 *ptr = reinterpret_cast<const uint8 *>(bytesRef->data());
                vsize size = bytesRef->size();
                return uploadTextureOpaque(ptr, size, name, context, bridge);
            }
            VPVL2_LOG(WARNING, "Cannot load a bridge from archive: " << name);
            /* force true to continue loading texture if path is directory */
            return false;
        }
        else if (!existsFile(path)) {
            VPVL2_LOG(WARNING, "Cannot load inexist " << path);
            return true; /* skip */
        }
    }
    return uploadTextureOpaque(path, context, bridge);
}

void BaseApplicationContext::validateEffectResources()
{
    nvFX::IResourceRepository *resourceRepository = nvFX::getResourceRepositorySingleton();
    resourceRepository->setParams(m_viewportRegion.x, m_viewportRegion.y, m_viewportRegion.z, m_viewportRegion.w, m_samplesMSAA, 0, static_cast<BufferHandle>(0));
    resourceRepository->validateAll();
}

bool BaseApplicationContext::uploadTextureOpaque(const uint8 *data, vsize size, const std::string &key, ModelContext *context, TextureDataBridge &bridge)
{
    /* fallback to default texture loader */
    VPVL2_VLOG(2, "Using default texture loader (stbi_image) instead of inherited class texture loader.");
    return context->uploadTexture(data, size, key, bridge);
}

bool BaseApplicationContext::uploadTextureOpaque(const std::string &path, ModelContext *context, TextureDataBridge &bridge)
{
    /* fallback to default texture loader */
    VPVL2_VLOG(2, "Using default texture loader (stbi_image) instead of inherited class texture loader.");
    return context->uploadTexture(path, bridge);
}

BaseSurface::Format BaseApplicationContext::defaultTextureFormat() const
{
    return BaseSurface::Format(kGL_RGBA, kGL_RGBA8, kGL_UNSIGNED_INT_8_8_8_8_REV, Texture2D::kGL_TEXTURE_2D);
}

void BaseApplicationContext::getMatrix(float32 value[], const IModel *model, int flags) const
{
    glm::mat4x4 m(1);
    if (internal::hasFlagBits(flags, IApplicationContext::kShadowMatrix)) {
        if (internal::hasFlagBits(flags, IApplicationContext::kProjectionMatrix)) {
            m *= m_cameraProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IApplicationContext::kViewMatrix)) {
            m *= m_cameraViewMatrix;
        }
        if (model && internal::hasFlagBits(flags, IApplicationContext::kWorldMatrix)) {
            static const Vector3 plane(0.0f, 1.0f, 0.0f);
            const ILight *light = m_sceneRef->lightRef();
            const Vector3 &direction = light->direction();
            const Scalar dot = plane.dot(-direction);
            float matrix[16];
            for (int i = 0; i < 4; i++) {
                int offset = i * 4;
                for (int j = 0; j < 4; j++) {
                    int index = offset + j;
                    matrix[index] = plane[i] * direction[j];
                    if (i == j) {
                        matrix[index] += dot;
                    }
                }
            }
            m *= glm::make_mat4x4(matrix);
            m *= m_cameraWorldMatrix;
            m = glm::scale(m, glm::vec3(model->scaleFactor()));
        }
    }
    else if (internal::hasFlagBits(flags, IApplicationContext::kCameraMatrix)) {
        if (internal::hasFlagBits(flags, IApplicationContext::kProjectionMatrix)) {
            m *= m_cameraProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IApplicationContext::kViewMatrix)) {
            m *= m_cameraViewMatrix;
        }
        if (model && internal::hasFlagBits(flags, IApplicationContext::kWorldMatrix)) {
            const IBone *bone = model->parentBoneRef();
            Transform transform;
            transform.setOrigin(model->worldTranslation());
            transform.setRotation(model->worldOrientation());
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
    else if (internal::hasFlagBits(flags, IApplicationContext::kLightMatrix)) {
        if (internal::hasFlagBits(flags, IApplicationContext::kProjectionMatrix)) {
            m *= m_lightProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IApplicationContext::kViewMatrix)) {
            m *= m_lightViewMatrix;
        }
        if (model && internal::hasFlagBits(flags, IApplicationContext::kWorldMatrix)) {
            m *= m_lightWorldMatrix;
            m = glm::scale(m, glm::vec3(model->scaleFactor()));
        }
    }
    if (internal::hasFlagBits(flags, IApplicationContext::kInverseMatrix)) {
        m = glm::inverse(m);
    }
    if (internal::hasFlagBits(flags, IApplicationContext::kTransposeMatrix)) {
        m = glm::transpose(m);
    }
    std::memcpy(value, glm::value_ptr(m), sizeof(float) * 16);
}

IString *BaseApplicationContext::loadShaderSource(ShaderType type, const IModel *model, void *userData)
{
    std::string file;
    if (type == kModelEffectTechniques) {
        IStringSmartPtr path(String::create(resolveEffectFilePath(model, static_cast<const IString *>(userData))));
        return loadShaderSource(type, path.get());
    }
    switch (model->type()) {
    case IModel::kAssetModel:
        file += "/asset/";
        break;
    case IModel::kPMDModel:
    case IModel::kPMXModel:
        file += "/pmx/";
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
    case kTransformFeedbackVertexShader:
        file += "transform.vsh";
        break;
    case kModelEffectTechniques:
    case kMaxShaderType:
    default:
        break;
    }
    const std::string &path = shaderDirectory() + "/" + file;
    MapBuffer buffer(this);
    if (mapFile(path, &buffer)) {
        std::string bytes(buffer.address, buffer.address + buffer.size);
        FunctionResolver *resolver = sharedFunctionResolverInstance();
        if (resolver->query(FunctionResolver::kQueryCoreProfile)) {
            static const char kFormat[] = "#version %d core\n";
            char appendingHeader[256];
            internal::snprintf(appendingHeader, sizeof(appendingHeader), kFormat, resolver->query(IApplicationContext::FunctionResolver::kQueryShaderVersion));
            return toIStringFromUtf8(appendingHeader + bytes);
        }
        else {
            return toIStringFromUtf8(bytes);
        }
    }
    else {
        return 0;
    }
}

IString *BaseApplicationContext::loadShaderSource(ShaderType type, const IString *path)
{
    if (type == kModelEffectTechniques) {
        std::string bytes;
        MapBuffer buffer(this);
        if (path && mapFile(static_cast<const String *>(path)->toStdString(), &buffer)) {
            uint8 *address = buffer.address;
            bytes.assign(address, address + buffer.size);
        }
        else {
            std::string defaultEffectPath = effectDirectory();
#if defined(VPVL2_LINK_NVFX)
            defaultEffectPath.append("/base.glslfx");
#elif defined(VPVL2_ENABLE_NVIDIA_CG)
            defaultEffectPath.append("/base.cgfx");
#endif
            if (mapFile(defaultEffectPath, &buffer)) {
                uint8 *address = buffer.address;
#if defined(VPVL2_LINK_NVFX)
                if (sharedFunctionResolverInstance()->hasExtension("ARB_separate_shader_objects")) {
                    bytes.append("#extension GL_ARB_separate_shader_objects : enable\n");
                }
                bytes.append(address, address + buffer.size);
#else
                bytes.assign(address, address + buffer.size);
#endif
            }
        }
        return toIStringFromUtf8(bytes);
    }
    return 0;
}

IString *BaseApplicationContext::loadKernelSource(KernelType type, void * /* userData */)
{
    std::string file;
    switch (type) {
    case kModelSkinningKernel:
        file += "skinning.cl";
        break;
    default:
        break;
    }
    MapBuffer buffer(this);
    const std::string &path = kernelDirectory() + "/" + file;
    if (mapFile(path, &buffer)) {
        std::string bytes(buffer.address, buffer.address + buffer.size);
        return String::create(bytes);
    }
    else {
        return 0;
    }
}

IString *BaseApplicationContext::toUnicode(const uint8 *str) const
{
    if (const char *s = reinterpret_cast<const char *>(str)) {
        return m_encodingRef->toString(str, std::strlen(s), IString::kShiftJIS);
    }
    return 0;
}

void BaseApplicationContext::getViewport(Vector3 &value) const
{
    value.setValue(m_viewportRegion.z, m_viewportRegion.w, 1);
}

void BaseApplicationContext::getMousePosition(Vector4 &value, MousePositionType type) const {
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

IModel *BaseApplicationContext::findEffectModelRef(const IString *name) const
{
    IModel *model = m_sceneRef->findModel(name);
    if (!model) {
        if (IModel *const *value = m_basename2ModelRefs.find(name->toHashString())) {
            model = *value;
        }
    }
    return model;
}

IModel *BaseApplicationContext::findEffectModelRef(const IEffect *effect) const
{
    if (IModel *const *value = m_effectRef2ModelRefs.find(effect)) {
        return *value;
    }
    return 0;
}

void BaseApplicationContext::setEffectModelRef(const IEffect *effectRef, IModel *model)
{
    const IString *name = model->name(IEncoding::kDefaultLanguage);
    m_effectRef2Owners.insert(effectRef, static_cast<const String *>(name)->toStdString());
    m_effectRef2ModelRefs.insert(effectRef, model);
}

void BaseApplicationContext::addModelFilePath(IModel *model, const std::string &path)
{
    if (model) {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher filenameMatcher(".+/((.+)\\.\\w+)$", 0, status);
        filenameMatcher.reset(UnicodeString::fromUTF8(path));
        if (filenameMatcher.find()) {
            const std::string &basename = String::toStdString(filenameMatcher.group(1, status));
            if (!model->name(IEncoding::kDefaultLanguage)) {
                String s(filenameMatcher.group(2, status));
                model->setName(&s, IEncoding::kDefaultLanguage);
            }
            m_basename2ModelRefs.insert(basename.c_str(), model);
            m_modelRef2Basenames.insert(model, basename);
        }
        else {
            if (!model->name(IEncoding::kDefaultLanguage)) {
                IStringSmartPtr s(String::create(path));
                model->setName(s.get(), IEncoding::kDefaultLanguage);
            }
            m_basename2ModelRefs.insert(path.c_str(), model);
        }
        m_modelRef2Paths.insert(model, path);
    }
}

std::string BaseApplicationContext::findEffectOwnerName(const IEffect *effect) const
{
    if (const std::string *value = m_effectRef2Owners.find(effect)) {
        return *value;
    }
    return std::string();
}

FrameBufferObject *BaseApplicationContext::createFrameBufferObject()
{
    return new FrameBufferObject(sharedFunctionResolverInstance(), m_renderColorFormat, m_samplesMSAA);
}

void BaseApplicationContext::getEffectCompilerArguments(Array<IString *> &arguments) const
{
    arguments.clear();
}

void BaseApplicationContext::addSharedTextureParameter(const char *name, const SharedTextureParameter &parameter)
{
    SharedTextureParameterKey key(parameter.parameterRef, name);
    m_sharedParameters.insert(std::make_pair(key, parameter));
}

bool BaseApplicationContext::tryGetSharedTextureParameter(const char *name, SharedTextureParameter &parameter) const
{
    SharedTextureParameterKey key(parameter.parameterRef, name);
    SharedTextureParameterMap::const_iterator it = m_sharedParameters.find(key);
    if (it != m_sharedParameters.end()) {
        parameter = it->second;
        return true;
    }
    return false;
}

void BaseApplicationContext::setMousePosition(const glm::vec4 &value, MousePositionType type, bool &handled)
{
    ETwMouseAction mouseAction = !btFuzzyZero(value.z) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
    switch (type) {
    case kMouseLeftPressPosition:
        m_mouseLeftPressPosition = value;
        handled = TwMouseButton(mouseAction, TW_MOUSE_LEFT) != 0;
        break;
    case kMouseMiddlePressPosition:
        m_mouseMiddlePressPosition = value;
        handled = TwMouseButton(mouseAction, TW_MOUSE_MIDDLE) != 0;
        break;
    case kMouseRightPressPosition:
        m_mouseRightPressPosition = value;
        handled = TwMouseButton(mouseAction, TW_MOUSE_RIGHT) != 0;
        break;
    case kMouseWheelPosition:
        handled = TwMouseWheel(value.y) != 0;
        break;
    case kMouseCursorPosition:
        m_mouseCursorPosition = value;
        handled = TwMouseMotion(value.x, value.y) != 0;
        break;
    default:
        break;
    }
}

void BaseApplicationContext::handleKeyPress(int value, int modifiers, bool &handled)
{
    handled = TwKeyPressed(value, modifiers) != 0;
}

std::string BaseApplicationContext::findModelFilePath(const IModel *modelRef) const
{
    if (const std::string *value = m_modelRef2Paths.find(modelRef)) {
        return *value;
    }
    return std::string();
}

std::string BaseApplicationContext::findModelFileBasename(const IModel *modelRef) const
{
    if (const std::string *value = m_modelRef2Basenames.find(modelRef)) {
        return *value;
    }
    return std::string();
}

FrameBufferObject *BaseApplicationContext::findFrameBufferObjectByRenderTarget(const IEffect::OffscreenRenderTarget &rt, bool enableAA)
{
    FrameBufferObject *buffer = 0;
    if (const ITexture *textureRef = rt.textureRef) {
        if (FrameBufferObject *const *value = m_renderTargets.find(textureRef)) {
            buffer = *value;
        }
        else {
            int nsamples = enableAA ? m_samplesMSAA : 0;
            buffer = m_renderTargets.insert(textureRef, new FrameBufferObject(sharedFunctionResolverInstance(), m_renderColorFormat, nsamples));
        }
    }
    return buffer;
}

void BaseApplicationContext::bindOffscreenRenderTarget(OffscreenTexture *textureRef, bool enableAA)
{
    const IEffect::OffscreenRenderTarget &rt = textureRef->renderTarget;
    FunctionResolver *resolver = sharedFunctionResolverInstance();
    if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
        buffer->bindTexture(textureRef->colorTextureRef, 0);
        buffer->bindDepthStencilBuffer(&textureRef->depthStencilBuffer);
    }
    static const GLuint buffers[] = { FrameBufferObject::kGL_COLOR_ATTACHMENT0 };
    static const int nbuffers = sizeof(buffers) / sizeof(buffers[0]);
    fx::Util::setRenderColorTargets(resolver, buffers, nbuffers);
}

void BaseApplicationContext::releaseOffscreenRenderTarget(const OffscreenTexture *textureRef, bool enableAA)
{
    const IEffect::OffscreenRenderTarget &rt = textureRef->renderTarget;
    if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
        buffer->readMSAABuffer(0);
        buffer->unbind();
    }
}

void BaseApplicationContext::parseOffscreenSemantic(IEffect *effectRef, const IString *directoryRef)
{
    pushAnnotationGroup("BaseApplicationContext#parseOffscreenSemantic", sharedFunctionResolverInstance());
#if defined(VPVL2_LINK_NVFX)
    if (effectRef) {
        Array<IEffect::Pass *> passes;
        Array<IEffect::Technique *> techniques;
        IEffect *defaultEffectRef = m_sceneRef->createDefaultStandardEffectRef(this);
        effectRef->getTechniqueRefs(techniques);
        const int ntechniques = techniques.count();
        for (int i = 0; i < ntechniques; i++) {
            IEffect::Technique *technique = techniques[i];
            if (fx::Util::isPassEquals(technique->annotationRef("MMDPass"), "vpvl2_nvfx_offscreen")) {
                technique->getPasses(passes);
                const int npasses = passes.count();
                for (int j = 0; j < npasses; j++) {
                    IEffect::Pass *pass = passes[j];
                    pass->setupOverrides(defaultEffectRef);
                    passes.append(pass);
                }
                m_offscreenTechniques.append(technique);
            }
        }
    }
#elif defined(VPVL2_ENABLE_NVIDIA_CG)
    if (effectRef) {
        EffectAttachmentRuleList attachmentRules;
        std::string line;
        UErrorCode status = U_ZERO_ERROR;
        Vector3 size;
        RegexMatcher extensionMatcher("\\.(cg)?fx(sub)?$", 0, status),
                pairMatcher("\\s*=\\s*", 0, status);
        Array<IEffect::OffscreenRenderTarget> offscreenRenderTargets;
        effectRef->getOffscreenRenderTargets(offscreenRenderTargets);
        const int nOffscreenRenderTargets = offscreenRenderTargets.count();
        /* オフスクリーンレンダーターゲットの設定 */
        for (int i = 0; i < nOffscreenRenderTargets; i++) {
            const IEffect::OffscreenRenderTarget &renderTarget = offscreenRenderTargets[i];
            const IEffect::Parameter *parameter = renderTarget.textureParameterRef;
            const IEffect::Annotation *annotation = parameter->annotationRef("DefaultEffect");
            std::istringstream stream(annotation ? annotation->stringValue() : std::string());
            std::vector<UnicodeString> tokens(2);
            attachmentRules.clear();
            /* スクリプトを解析 */
            while (std::getline(stream, line, ';')) {
                int32 size = static_cast<int32>(tokens.size());
                if (pairMatcher.split(UnicodeString::fromUTF8(line), &tokens[0], size, status) == size) {
                    const UnicodeString &key = tokens[0].trim();
                    const UnicodeString &value = tokens[1].trim();
                    RegexMatcherSmartPtr regexp;
                    status = U_ZERO_ERROR;
                    /* self が指定されている場合は自身のエフェクトのファイル名を設定する */
                    if (key == "self") {
                        const IModel *model = effectOwner(effectRef);
                        const UnicodeString &name = findModelBasename(model);
                        regexp.reset(new RegexMatcher("\\A\\Q" + name + "\\E\\z", 0, status));
                    }
                    else {
                        UnicodeString pattern = "\\A\\Q" + key + "\\E\\z";
                        pattern.findAndReplace("?", "\\E.\\Q");
                        pattern.findAndReplace("*", "\\E.*\\Q");
                        pattern.findAndReplace("\\Q\\E", "");
                        regexp.reset(new RegexMatcher(pattern, 0, status));
                    }
                    IEffect *offscreenEffectRef = 0;
                    /* hide/none でなければオフスクリーン専用のモデルのエフェクト（オフスクリーン側が指定）を読み込む */
                    bool hidden = (value == "hide" || value == "none");
                    if (!hidden) {
                        const UnicodeString &path = createPath(directoryRef, value);
                        extensionMatcher.reset(path);
                        status = U_ZERO_ERROR;
                        const String s2(extensionMatcher.replaceAll(".cgfx", status));
                        offscreenEffectRef = createEffectRef(&s2);
                        if (offscreenEffectRef) {
                            offscreenEffectRef->setParentEffectRef(effectRef);
                            VPVL2_VLOG(2, "Loaded an individual effect by offscreen: path=" << internal::cstr(&s2, "") << " pattern=" << String::toStdString(key));
                        }
                    }
                    attachmentRules.push_back(EffectAttachmentRule(regexp.release(), std::make_pair(offscreenEffectRef, hidden)));
                }
            }
            if (!fx::Util::getSize2(parameter, size)) {
                Vector3 viewport;
                getViewport(viewport);
                size.setX(btMax(1.0f, viewport.x() * size.x()));
                size.setY(btMax(1.0f, viewport.y() * size.y()));
            }
            FunctionResolver *resolver = sharedFunctionResolverInstance();
            BaseSurface::Format format; /* unused */
            fx::Util::getTextureFormat(parameter, resolver, format);
            /* RenderContext 特有の OffscreenTexture に変換して格納 */
            m_offscreenTextures.append(new OffscreenTexture(renderTarget, attachmentRules, size, resolver));
        }
    }
#endif
    popAnnotationGroup(sharedFunctionResolverInstance());
}

void BaseApplicationContext::renderOffscreen()
{
    pushAnnotationGroup("BaseApplicationContext#renderOffscreen", sharedFunctionResolverInstance());
#if defined(VPVL2_LINK_NVFX)
    Array<IEffect::Pass *> passes;
    Array<IRenderEngine *> engines;
    m_sceneRef->getRenderEngineRefs(engines);
    if (m_viewportRegionInvalidated) {
        validateEffectResources();
        m_viewportRegionInvalidated = false;
    }
    const int nengines = engines.count(), ntechniques = m_offscreenTechniques.count();
    int actualProceededTechniques = 0;
    for (int i = 0; i < ntechniques; i++) {
        IEffect::Technique *technique = m_offscreenTechniques[i];
        if (technique->parentEffectRef()->isEnabled()) {
            technique->getPasses(passes);
            const int npasses = passes.count();
            for (int j = 0; j < npasses; j++) {
                IEffect::Pass *pass = passes[j];
                pass->setState();
                if (pass->isRenderable()) {
                    for (int k = 0; k < nengines; k++) {
                        IRenderEngine *engine = engines[k];
                        engine->setOverridePass(pass);
                        engine->renderEdge();
                        engine->renderModel();
                        engine->setOverridePass(0);
                    }
                    pass->resetState();
                }
            }
            actualProceededTechniques++;
        }
    }
    if (actualProceededTechniques == 0) {
        viewport(m_viewportRegion.x, m_viewportRegion.y, m_viewportRegion.z, m_viewportRegion.w);
        clear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT | kGL_STENCIL_BUFFER_BIT);
    }
#elif defined(VPVL2_ENABLE_NVIDIA_CG)
    Array<IRenderEngine *> engines;
    m_sceneRef->getRenderEngineRefs(engines);
    const int nengines = engines.count();
    /* オフスクリーンレンダリングを行う前に元のエフェクトを保存する */
    Hash<HashPtr, IEffect *> effects;
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        if (IEffect *starndardEffect = engine->effectRef(IEffect::kStandard)) {
            effects.insert(engine, starndardEffect);
        }
        else if (IEffect *postEffect = engine->effectRef(IEffect::kPostProcess)) {
            effects.insert(engine, postEffect);
        }
        else if (IEffect *preEffect = engine->effectRef(IEffect::kPreProcess)) {
            effects.insert(engine, preEffect);
        }
        else {
            effects.insert(engine, 0);
        }
    }
    /* オフスクリーンレンダーターゲット毎にエフェクトを実行する */
    const int ntextures = m_offscreenTextures.count();
    for (int i = 0; i < ntextures; i++) {
        OffscreenTexture *offscreenTexture = m_offscreenTextures[i];
        const EffectAttachmentRuleList &rules = offscreenTexture->attachmentRules;
        const IEffect::OffscreenRenderTarget &renderTarget = offscreenTexture->renderTarget;
        const IEffect::Parameter *parameter = renderTarget.textureParameterRef;
        bool enableAA = false;
        /* セマンティクスから各種パラメータを設定 */
        if (const IEffect::Annotation *annotation = parameter->annotationRef("AntiAlias")) {
            enableAA = annotation->booleanValue();
        }
        /* オフスクリーンレンダリングターゲットを割り当ててレンダリング先をそちらに変更する */
        bindOffscreenRenderTarget(offscreenTexture, enableAA);
        const ITexture *texture = renderTarget.textureRef;
        const Vector3 &size = texture->size();
        updateCameraMatrices(glm::vec2(size.x(), size.y()));
        viewport(0, 0, GLsizei(size.x()), GLsizei(size.y()));
        if (const IEffect::Annotation *annotation = parameter->annotationRef("ClearColor")) {
            int nvalues;
            const float *color = annotation->floatValues(&nvalues);
            if (nvalues == 4) {
                clearColor(color[0], color[1], color[2], color[3]);
            }
        }
        else {
            clearDepth(1);
        }
        /* オフスクリーンレンダリングターゲットに向けてレンダリングを実行する */
        clear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT | kGL_STENCIL_BUFFER_BIT);
        for (int j = 0; j < nengines; j++) {
            IRenderEngine *engine = engines[j];
            const IModel *model = engine->parentModelRef();
            const UnicodeString &basename = findModelBasename(model);
            EffectAttachmentRuleList::const_iterator it2 = rules.begin();
            bool hidden = false;
            while (it2 != rules.end()) {
                const EffectAttachmentRule &rule = *it2;
                RegexMatcher *matcherRef = rule.first;
                matcherRef->reset(basename);
                if (matcherRef->find()) {
                    const EffectAttachmentValue &v = rule.second;
                    IEffect *effectRef = v.first;
                    engine->setEffect(effectRef, IEffect::kStandardOffscreen, 0);
                    hidden = v.second;
                    break;
                }
                ++it2;
            }
            if (!hidden) {
                engine->update();
                engine->renderModel();
                engine->renderEdge();
            }
        }
        /* オフスクリーンレンダリングターゲットの割り当てを解除 */
        releaseOffscreenRenderTarget(offscreenTexture, enableAA);
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *const *effect = effects.find(engine);
        engine->setEffect(*effect, IEffect::kAutoDetection, 0);
    }
#endif
    popAnnotationGroup(sharedFunctionResolverInstance());
}

void BaseApplicationContext::createEffectParameterUIWidgets(IEffect *effectRef)
{
    Array<IEffect::Parameter *> parameters;
    effectRef->getInteractiveParameters(parameters);
    TwBar *bar = EffectParameterUIBuilder::createBar(effectRef);
    const int nparameters = parameters.count();
    std::ostringstream stream;
    for (int i = 0; i < nparameters; i++) {
        IEffect::Parameter *parameterRef = parameters[i];
        stream.str(std::string());
        EffectParameterUIBuilder::setCommonDefinition(parameterRef, stream);
        switch (parameterRef->type()) {
        case IEffect::Parameter::kBoolean: {
            EffectParameterUIBuilder::createBoolean(bar, parameterRef, stream);
            break;
        }
        case IEffect::Parameter::kInteger: {
            EffectParameterUIBuilder::createInteger(bar, parameterRef, stream);
            break;
        }
        case IEffect::Parameter::kFloat: {
            EffectParameterUIBuilder::createFloat(bar, parameterRef, stream);
            break;
        }
        default:
            break;
        }
    }
    m_effectRef2ParameterUIs.insert(effectRef, bar);
}

void BaseApplicationContext::renderEffectParameterUIWidgets()
{
    TwDraw();
}

IEffect *BaseApplicationContext::createEffectRef(const std::string &path)
{
    pushAnnotationGroup("BaseApplicationContext#createEffectRef", sharedFunctionResolverInstance());
    IEffect *effectRef = 0;
    const HashString key(path.c_str());
    if (IEffect *const *value = m_effectCaches.find(key)) {
        effectRef = *value;
    }
    else if (existsFile(path)) {
        IStringSmartPtr pathPtr(String::create(path));
        IEffectSmartPtr effectPtr(m_sceneRef->createEffectFromFile(pathPtr.get(), this));
        if (!effectPtr.get() || !effectPtr->internalPointer()) {
            const char *message = effectPtr.get() ? effectPtr->errorString() : "(null)";
            VPVL2_LOG(WARNING, "Cannot compile an effect: " << path << " error=" << message);
        }
        else if (!m_effectCaches.find(key)) {
            const std::string &name = path.substr(path.rfind("/") + 1);
            IStringSmartPtr namePtr(String::create(name));
            effectRef = m_effectCaches.insert(key, effectPtr.release());
            effectRef->setName(namePtr.get());
            validateEffectResources();
            m_effectRef2Paths.insert(effectRef, path);
            VPVL2_LOG(INFO, "Created a effect: " << path);
        }
        else {
            VPVL2_LOG(INFO, "Duplicated effect was found and ignored it: " << path);
        }
    }
    else {
        effectRef = m_sceneRef->createDefaultStandardEffectRef(this);
        if (!effectRef) {
            VPVL2_LOG(WARNING, "Cannot compile an effect: " << path);
        }
    }
    popAnnotationGroup(sharedFunctionResolverInstance());
    return effectRef;
}

IEffect *BaseApplicationContext::createEffectRef(IModel *modelRef, const IString *directoryRef)
{
    const std::string &filePath = resolveEffectFilePath(modelRef, directoryRef);
    IEffect *effectRef = createEffectRef(filePath);
    if (effectRef) {
        setEffectModelRef(effectRef, modelRef);
        VPVL2_LOG(INFO, "Loaded an model effect: model=" << internal::cstr(modelRef->name(IEncoding::kDefaultLanguage), "(null)") << " path=" << filePath);
    }
    return effectRef;
}

std::string BaseApplicationContext::findEffectFilePath(const IEffect *effectRef) const
{
    if (const std::string *path = m_effectRef2Paths.find(effectRef)) {
        return *path;
    }
    return std::string();
}

std::string BaseApplicationContext::resolveEffectFilePath(const IModel *model, const IString *dir) const
{
    const std::string &path = findModelFilePath(model);
    if (!path.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher filenameMatcher("^.+/(.+)\\.\\w+$", 0, status);
        const UnicodeString &unicodePath = UnicodeString::fromUTF8(path);
        filenameMatcher.reset(unicodePath);
        const UnicodeString &s = filenameMatcher.find() ? filenameMatcher.group(1, status) : unicodePath;
        RegexMatcher extractMatcher("^.+\\[(.+)(?:\\.(?:cg)?fx)?\\]$", 0, status);
        extractMatcher.reset(s);
        const UnicodeString &cgfx = extractMatcher.find()
                ? extractMatcher.replaceAll("$1.cgfx", status) : s + ".cgfx";
        const std::string &newEffectPath = static_cast<const String *>(dir)->toStdString()
                + "/" + String::toStdString(cgfx);
        if (existsFile(newEffectPath)) {
            return newEffectPath;
        }
    }
    return static_cast<const String *>(dir)->toStdString() + "/default.cgfx";
}

void BaseApplicationContext::deleteEffectRef(const std::string &path)
{
    const HashString key(path.c_str());
    if (IEffect *const *value = m_effectCaches.find(key)) {
        IEffect *effect = *value;
        if (void *const *ptr = m_effectRef2ParameterUIs.find(effect)) {
            m_effectRef2ParameterUIs.remove(effect);
            TwBar *bar = static_cast<TwBar *>(*ptr);
            TwDeleteBar(bar);
        }
        m_effectRef2Paths.remove(effect);
        m_effectRef2ModelRefs.remove(effect);
        m_effectRef2Owners.remove(effect);
        internal::deleteObject(effect);
        m_effectCaches.remove(key);
    }
}

void BaseApplicationContext::deleteEffectRef(IEffect *&effectRef)
{
    const std::string &path = findEffectFilePath(effectRef);
    if (!path.empty()) {
        deleteEffectRef(path);
        effectRef = 0;
    }
}

void BaseApplicationContext::deleteEffectRef(IModel *modelRef, const IString *directoryRef)
{
    deleteEffectRef(resolveEffectFilePath(modelRef, directoryRef));
}

IModel *BaseApplicationContext::currentModelRef() const
{
    return m_currentModelRef;
}

void BaseApplicationContext::setCurrentModelRef(IModel *value)
{
    m_currentModelRef = value;
}

int BaseApplicationContext::samplesMSAA() const
{
    return m_samplesMSAA;
}

void BaseApplicationContext::setSamplesMSAA(int value)
{
    m_samplesMSAA = value;
}

Scene *BaseApplicationContext::sceneRef() const
{
    return m_sceneRef;
}

void BaseApplicationContext::getCameraMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection) const
{
    world = m_cameraWorldMatrix;
    view = m_cameraViewMatrix;
    projection = m_cameraProjectionMatrix;
}

void BaseApplicationContext::setCameraMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection)
{
    m_cameraWorldMatrix = world;
    m_cameraViewMatrix = view;
    m_cameraProjectionMatrix = projection;
}

void BaseApplicationContext::getLightMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection) const
{
    world = m_lightWorldMatrix;
    view = m_lightViewMatrix;
    projection = m_lightProjectionMatrix;
}

void BaseApplicationContext::setLightMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection)
{
    m_lightWorldMatrix = world;
    m_lightViewMatrix = view;
    m_lightProjectionMatrix = projection;
}

void BaseApplicationContext::updateCameraMatrices()
{
    const ICamera *cameraRef = m_sceneRef->cameraRef();
    Scalar matrix[16];
    cameraRef->modelViewTransform().getOpenGLMatrix(matrix);
    const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
            &projection = glm::infinitePerspective(cameraRef->fov(), m_aspectRatio, cameraRef->znear());
    setCameraMatrices(world, view, projection);
}

glm::ivec4 BaseApplicationContext::viewportRegion() const
{
    return m_viewportRegion;
}

void BaseApplicationContext::setViewportRegion(const glm::ivec4 &viewport)
{
    if (m_viewportRegion != viewport) {
        const glm::mediump_float &width = glm::mediump_float(viewport.z), &height = glm::mediump_float(viewport.w);
        m_aspectRatio = glm::max(width, height) / glm::min(width, height);
        m_viewportRegion = viewport;
        m_viewportRegionInvalidated = true;
        TwWindowSize(viewport.z, viewport.w);
        updateCameraMatrices();
    }
}

void BaseApplicationContext::createShadowMap(const Vector3 &size)
{
    const FunctionResolver *resolver = sharedFunctionResolverInstance();
    const char *kRequiredExtensions[] = {
        "ARB_texture_rg",
        "ARB_framebuffer_object",
        "ARB_depth_buffer_float",
        0
    };
    bool isSelfShadowSupported =
            resolver->query(IApplicationContext::FunctionResolver::kQueryVersion) >= gl::makeVersion(3, 2) ||
            hasAllExtensions(kRequiredExtensions, resolver);
    if (isSelfShadowSupported && !size.isZero() &&
            !(m_shadowMap.get() && (m_shadowMap->size() - size).fuzzyZero())) {
        pushAnnotationGroup("BaseApplicationContext#createShadowMap", sharedFunctionResolverInstance());
        m_shadowMap.reset(new SimpleShadowMap(resolver, vsize(size.x()), vsize(size.y())));
        m_shadowMap->create();
        popAnnotationGroup(sharedFunctionResolverInstance());
    }
    m_sceneRef->setShadowMapRef(m_shadowMap.get());
}

void BaseApplicationContext::releaseShadowMap()
{
    pushAnnotationGroup("BaseApplicationContext#releaseShadowMap", sharedFunctionResolverInstance());
    m_shadowMap.reset();
    m_sceneRef->setShadowMapRef(0);
    popAnnotationGroup(sharedFunctionResolverInstance());
}

void BaseApplicationContext::renderShadowMap()
{
    if (SimpleShadowMap *shadowMapRef = m_shadowMap.get()) {
        pushAnnotationGroup("BaseApplicationContext#renderShadowMap", sharedFunctionResolverInstance());
        shadowMapRef->bind();
        const Vector3 &size = shadowMapRef->size();
        viewport(0, 0, GLsizei(size.x()), GLsizei(size.y()));
        clear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT);
        Array<IRenderEngine *> engines;
        m_sceneRef->getRenderEngineRefs(engines);
        const int nengines = engines.count();
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            engine->renderZPlot();
        }
        shadowMapRef->unbind();
        popAnnotationGroup(sharedFunctionResolverInstance());
    }
}

std::string BaseApplicationContext::toonDirectory() const
{
    return m_configRef->value("dir.system.toon", std::string(":textures"));
}

std::string BaseApplicationContext::shaderDirectory() const
{
    return m_configRef->value("dir.system.shaders", std::string(":shaders"));
}

std::string BaseApplicationContext::effectDirectory() const
{
    return m_configRef->value("dir.system.effects", std::string(":effects"));
}

std::string BaseApplicationContext::kernelDirectory() const
{
    return m_configRef->value("dir.system.kernels", std::string(":kernels"));
}

void BaseApplicationContext::debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /* length */, const GLchar *message, GLvoid * /* userData */)
{
    switch (severity) {
    case kGL_DEBUG_SEVERITY_HIGH_ARB:
        VPVL2_LOG(ERROR, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    case kGL_DEBUG_SEVERITY_MEDIUM_ARB:
        VPVL2_LOG(WARNING, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    case kGL_DEBUG_SEVERITY_LOW_ARB:
        VPVL2_LOG(INFO, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    default:
        break;
    }
}

} /* namespace extensions */
} /* namespace vpvl2 */

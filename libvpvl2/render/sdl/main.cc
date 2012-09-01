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

/* internal headers for debug */
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/vmd/Motion.h"

/* ICU */
#include <unicode/unistr.h>

/* SDL */
#include <SDL.h>
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

/* DevIL */
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

namespace {

using namespace vpvl2;

class String : public IString {
public:
    String(const UnicodeString &value)
        : m_value(value)
    {
    }
    ~String() {
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->value());
    }
    bool contains(const IString *value) const {
        return m_value.indexOf(static_cast<const String *>(value)->value()) != -1;
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->value());
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(reinterpret_cast<const char *>(m_value.getTerminatedBuffer()));
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const UnicodeString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_value.getTerminatedBuffer());
    }
    size_t length() const {
        return m_value.length();
    }

private:
    mutable UnicodeString m_value;
};

class Encoding : public IEncoding {
public:
    Encoding()
    {
    }
    ~Encoding() {
    }

    const IString *stringConstant(ConstantType value) const {
        switch (value) {
        case kLeft: {
            static const String s("左");
            return &s;
        }
        case kRight: {
            static const String s("右");
            return &s;
        }
        case kFinger: {
            static const String s("指");
            return &s;
        }
        case kElbow: {
            static const String s("ひじ");
            return &s;
        }
        case kArm: {
            static const String s("腕");
            return &s;
        }
        case kWrist: {
            static const String s("手首");
            return &s;
        }
        case kCenter: {
            static const String s("センター");
            return &s;
        }
        default: {
            static const String s("");
            return &s;
        }
        }
    }
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(UnicodeString(str, size, "shift_jis"));
            break;
        case IString::kUTF8:
            s = new String(UnicodeString(str, size, "utf-8"));
            break;
        case IString::kUTF16:
            s = new String(UnicodeString(str, size, "utf-16"));
            break;
        default:
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = strlen(reinterpret_cast<const char *>(value));
        return toString(value, btMin(maxlen, size), codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        if (value) {
            const String *s = static_cast<const String *>(value);
            const UnicodeString &src = s->value();
            const char *codecTo = 0;
            switch (codec) {
            case IString::kShiftJIS:
                codecTo = "shift_jis";
                break;
            case IString::kUTF8:
                codecTo = "utf-8";
                break;
            case IString::kUTF16:
                codecTo = "utf-16";
                break;
            default:
                break;
            }
            size_t size = s->length(), newStringLength = src.extract(0, size, 0, codecTo);
            uint8_t *data = new uint8_t[newStringLength + 1];
            src.extract(0, size, reinterpret_cast<char *>(data), codecTo);
            memcpy(data, s->toByteArray(), size);
            data[size] = 0;
            return data;
        }
        return 0;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }
};

typedef std::map<std::string, std::string> UIStringMap;
struct UIContext
{
    UIContext(Scene *scene, UIStringMap *config)
        : sceneRef(scene),
          configRef(config),
          width(640),
          height(480),
          active(true)
    {
    }
    const Scene *sceneRef;
    const UIStringMap *configRef;
    size_t width;
    size_t height;
    bool active;
};

static void UIHandleKeyEvent(const SDL_KeyboardEvent &event, UIContext &context)
{
    const SDL_keysym &keysym = event.keysym;
    switch (keysym.sym) {
    case SDLK_ESCAPE:
        context.active = false;
        break;
    default:
        break;
    }
}

static void UIProceedEvents(UIContext &context)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            UIHandleKeyEvent(event.key, context);
            break;
        case SDL_VIDEORESIZE:
            context.width = event.resize.w;
            context.height = event.resize.h;
            break;
        case SDL_QUIT:
            context.active = false;
            break;
        default:
            break;
        }
    }
}

static void UIDrawScreen(const UIContext &context)
{
    glViewport(0, 0, context.width, context.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    const Array<IRenderEngine *> &engines = context.sceneRef->renderEngines();
    const int nengines = engines.count();
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        engine->preparePostProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        engine->performPreProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        engine->renderModel();
        engine->renderEdge();
        engine->renderShadow();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        engine->performPostProcess();
    }
    SDL_GL_SwapBuffers();
}

static std::string UITrimString(const std::string &value)
{
    const char delimiters[] = " \t\r\n";
    std::string::size_type first = value.find_first_not_of(delimiters);
    if (first == std::string::npos) {
        return std::string();
    }
    std::string::size_type last = value.find_last_not_of(delimiters);
    return value.substr(first, last - first + 1);
}

static bool UILoadFile(const std::string &path, std::string &bytes)
{
    bytes.clear();
    FILE *fp = ::fopen(path.c_str(), "rb");
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

class Delegate : public IRenderDelegate {
public:
    Delegate(Scene *sceneRef, UIStringMap *configRef)
        : m_sceneRef(sceneRef),
          m_configRef(configRef),
          m_lightWorldMatrix(1),
          m_lightViewMatrix(1),
          m_lightProjectionMatrix(0),
          m_cameraModelMatrix(1),
          m_cameraViewMatrix(1),
          m_cameraProjectionMatrix(1)
    {
    }
    ~Delegate()
    {
    }

    void allocateContext(const IModel * /* model */, void *& /* context */) {
    }
    void releaseContext(const IModel * /* model */, void *& /* context */) {
    }
    bool uploadTexture(const IString * /* name */, const IString * /* dir */, int /* flags */, Texture & /* texture */, void * /* context */) {
#if 0
        if (false) {
            ILuint imageID;
            ilGenImages(1, &imageID);
            ilBindImage(imageID);
            ILboolean loaded = IL_FALSE;
            loaded = ilLoadL(ilTypeFromExt(("." + suffix).constData()), bytes.constData(), bytes.size());
            if (loaded == IL_FALSE) {
                ILenum error = ilGetError();
                while (error != IL_NO_ERROR) {
                    qWarning("Cannot load a texture %s: %s", qPrintable(path), iluErrorString(error));
                    error = ilGetError();
                }
                ilDeleteImages(1, &imageID);
                ok = false;
                return true;
            }
            iluFlipImage();
            GLuint textureID = ilutGLBindTexImage();
            size_t width = ilGetInteger(IL_IMAGE_WIDTH);
            size_t height = ilGetInteger(IL_IMAGE_HEIGHT);
            ilBindImage(0);
            ilDeleteImages(1, &imageID);
        }
        else {
            ILuint imageID;
            ilGenImages(1, &imageID);
            ilBindImage(imageID);
            ILboolean loaded = IL_FALSE;
            if (!path.startsWith(":textures/")) {
                loaded = ilLoadImage(path.toLocal8Bit().constData());
            }
            else {
                QFile file(path);
                file.open(QFile::ReadOnly);
                const QByteArray &bytes = file.readAll();
                loaded = ilLoadL(ilTypeFromExt(path.toLocal8Bit().constData()), bytes.constData(), bytes.size());
            }
            if (loaded == IL_FALSE) {
                ILenum error = ilGetError();
                while (error != IL_NO_ERROR) {
                    qWarning("Cannot load a texture %s: %s", qPrintable(path), iluErrorString(error));
                    error = ilGetError();
                }
                ilDeleteImages(1, &imageID);
                ok = false;
                return true;
            }
            iluFlipImage();
            GLuint textureID = ilutGLBindTexImage();
            size_t width = ilGetInteger(IL_IMAGE_WIDTH);
            size_t height = ilGetInteger(IL_IMAGE_HEIGHT);
            ilBindImage(0);
            ilDeleteImages(1, &imageID);
            TextureCache cache(width, height, textureID);
            m_texture2Paths.insert(textureID, path);
            setTextureID(cache, isToon, texture);
            addTextureCache(privateContext, path, cache);
            qDebug("Loaded a texture (ID=%d, width=%ld, height=%ld): \"%s\"",
                   textureID, width, height, qPrintable(path));
            ok = textureID != 0;
            return ok;
        }
#endif
        return false;
    }
    void getMatrix(float value[], const IModel *model, int flags) const {
        glm::mat4x4 m(1);
        if (flags & IRenderDelegate::kShadowMatrix) {
            if (flags & IRenderDelegate::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderDelegate::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderDelegate::kWorldMatrix) {
                glm::mat4x4 shadowMatrix(1);
                /*
                static const Vector3 plane(0.0f, 1.0f, 0.0f);
                const ILight *light = m_sceneRef->light();
                const Vector3 &direction = light->direction();
                const Scalar dot = plane.dot(-direction);
                for (int i = 0; i < 4; i++) {
                    int offset = i * 4;
                    for (int j = 0; j < 4; j++) {
                        int index = offset + j;
                        shadowMatrix[index] = plane[i] * direction[j];
                        if (i == j)
                            shadowMatrix[index] += dot;
                    }
                }
                */
                m *= shadowMatrix;
                m *= m_cameraModelMatrix;
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
                m *= m_cameraModelMatrix;
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
        const std::string &base = (*m_configRef)["dir.system.shaders"];
        const std::string &path = base + "/" + file;
        std::string bytes;
        if (UILoadFile(path, bytes)) {
            UnicodeString s(reinterpret_cast<const UChar *>(bytes.c_str()), bytes.length());
            return new(std::nothrow) String(s);
        }
        else {
            return 0;
        }
    }
    IString *loadKernelSource(KernelType /* type */, void * /* context */) {
        return 0;
    }
    IString *toUnicode(const uint8_t * /* str */) const {
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

private:
    Scene *m_sceneRef;
    UIStringMap *m_configRef;
    glm::mat4x4 m_lightWorldMatrix;
    glm::mat4x4 m_lightViewMatrix;
    glm::mat4x4 m_lightProjectionMatrix;
    glm::mat4x4 m_cameraModelMatrix;
    glm::mat4x4 m_cameraViewMatrix;
    glm::mat4x4 m_cameraProjectionMatrix;
};

} /* namespace anonymous */

int main(int /* argc */, char ** /* argv[] */)
{
    atexit(SDL_Quit);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        std::cerr << "SDL_GetVideoInfo() failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    size_t width = 640, height = 480, bpp = info->vfmt->BitsPerPixel;
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Surface *surface = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL | SDL_RESIZABLE);
    if (!surface) {
        std::cerr << "SDL_SetVideoMode(width, height, bpp, SDL_OPENGL) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_WM_SetCaption("libvpvl2 with SDL", 0);
    std::cerr << "GL_VERSION:                " << glGetString(GL_VERSION) << std::endl;
    std::cerr << "GL_VENDOR:                 " << glGetString(GL_VENDOR) << std::endl;
    std::cerr << "GL_RENDERER:               " << glGetString(GL_RENDERER) << std::endl;

    int value = 0;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    std::cerr << "SDL_GL_RED_SIZE:           " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
    std::cerr << "SDL_GL_GREEN_SIZE:         " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    std::cerr << "SDL_GL_BLUE_SIZE:          " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    std::cerr << "SDL_GL_DEPTH_SIZE:         " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value);
    std::cerr << "SDL_GL_STENCIL_SIZE:       " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value);
    std::cerr << "SDL_GL_DOUBLEBUFFER:       " << value << std::endl;
    SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
    std::cerr << "SDL_GL_ACCELERATED_VISUAL: " << value << std::endl;

    std::ifstream stream("config.ini");
    std::string line;
    UIStringMap config;
    while (stream && std::getline(stream, line)) {
        if (line.empty() || line.find_first_of("#;") != std::string::npos)
            continue;
        std::istringstream ss(line);
        std::string key, value;
        std::getline(ss, key, '=');
        std::getline(ss, value);
        config[UITrimString(key)] = UITrimString(value);
    }

    Encoding encoding;
    Factory factory(&encoding);
    Scene scene;
    Delegate delegate(&scene, &config);
    glClearColor(0, 0, 1, 0);
    glClearDepth(0);
    bool ok = false;
    const std::string &motionPath = config["dir.motion"] + "/" + config["file.motion"];

    std::string data;
    int nmodels = int(strtol(config["models/size"].c_str(), 0, 10));
    for (int i = 0; i < nmodels; i++) {
        std::ostringstream stream;
        stream << "models/" << (i + 1);
        const std::string &prefix = stream.str(), &modelPath = config[prefix + "/path"];
        if (UILoadFile(modelPath, data)) {
            int flags = 0;
            IModel *model = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&delegate, model, flags);
            model->setEdgeWidth(strtof(config[prefix + "/edge.width"].c_str(), 0));
            if (engine->upload(0)) {
                scene.addModel(model, engine);
                if (UILoadFile(motionPath, data)) {
                    IMotion *motion = factory.createMotion(UICastData(data), data.size(), model, ok);
                    scene.addMotion(motion);
                }
            }
        }
    }
    int nassets = int(strtol(config["assets/size"].c_str(), 0, 10));
    for (int i = 0; i < nassets; i++) {
        std::ostringstream stream;
        stream << "assets/" << (i + 1);
        const std::string &prefix = stream.str(), &assetPath = config[prefix + "/path"];
        if (UILoadFile(assetPath, data)) {
            IModel *asset = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(0, asset, 0);
            scene.addModel(asset, engine);
        }
    }

    UIContext context(&scene, &config);
    while (context.active) {
        UIProceedEvents(context);
        UIDrawScreen(context);
    }

    return 0;
}

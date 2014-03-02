#include <iostream>

#include <GL/glew.h>
#include <FxParser.h>

/* prevent redefinition of GLhandleARB */
#define GL_ARB_shader_objects
#include <GLFW/glfw3.h>

#ifdef ENABLE_REGAL
#include <GL/Regal.h>
#else
#define RegalMakeCurrent(ctx)
#define RegalSetErrorCallback(callback)
#endif

#ifdef ENABLE_GLSL_OPTIMIZER
#include "glsl_optimizer.h"
#endif

#if defined(__APPLE__)
#include <OpenGL/CGLCurrent.h>
#else
#define CGLGetCurrentContext() 0
#endif

using namespace nvFX;

namespace {

static const GLenum GL_NUM_EXTENSIONS = 0x821D;

static void HandleGLFWError(int /* error */, const char *message)
{
    std::cerr << "HandleGLFWError: " << message << std::endl;
}

static void HandleNVFXError(const char *message)
{
    std::cerr << "NVFXError: " << message;
}

static void HandleIncludeCallback(const char *filename, FILE *&fp, const char *&buffer)
{
    fp = fopen(filename, "rb");
    buffer = 0;
}

static void OptimizeShader(struct glslopt_ctx *context, enum glslopt_shader_type type, std::string &code)
{
    glslopt_shader *s = glslopt_optimize(context, type, code.c_str(), 0);
    if (glslopt_get_status(s)) {
        const char *source = glslopt_get_output(s);
        code.assign(source);
    }
    else {
        //std::cerr << glslopt_get_log(s) << std::endl;
    }
    glslopt_shader_delete(s);
}

static void HandleShader(GLuint shader, const char *name, struct glslopt_ctx *context)
{
    std::string buffer;
    GLint size, shaderType;
    ::glGetShaderiv(shader, GL_SHADER_TYPE, &shaderType);
    ::glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &size);
    buffer.resize(size);
    ::glGetShaderSource(shader, size, &size, &buffer[0]);
    enum glslopt_shader_type optimizeShaderType;
    std::string extension;
    if (shaderType == GL_VERTEX_SHADER) {
        optimizeShaderType = kGlslOptShaderVertex;
        extension.assign(".vert");
    }
    else if (shaderType == GL_FRAGMENT_SHADER) {
        optimizeShaderType = kGlslOptShaderFragment;
        extension.assign(".frag");
    }
    // OptimizeShader(context, optimizeShaderType, buffer);
    if (FILE *fp = fopen((std::string(name) + extension).c_str(), "wb")) {
        fwrite(buffer.c_str(), buffer.size(), 1, fp);
        fclose(fp);
    }
}

static void WritePassShaders(IPass *pass)
{
    IProgram *program = pass->getExInterface()->getProgram(0);
    GLsizei count = 0;
    GLuint shaders[2];
    struct glslopt_ctx *context = glslopt_initialize(kGlslTargetOpenGL);
    glGetAttachedShaders(program->getProgram(), sizeof(shaders) / sizeof(shaders[0]), &count, shaders);
    HandleShader(shaders[0], pass->getName(), context);
    HandleShader(shaders[1], pass->getName(), context);
    glslopt_cleanup(context);
}

static bool ParseEffect(const char *filename, bool isCoreProfileEnabled)
{
    IContainer *container = 0;
    container = IContainer::create("nvFXcc");
    if (!container) {
        std::cerr << "Cannot create IContainer" << std::endl;
        return false;
    }
    if (loadEffectFromFile(container, filename)) {
        container->getExInterface()->separateShadersEnable(false);
        getResourceRepositorySingleton()->setParams(0, 0, 1, 1, 1, 0, 0);
        if (!getResourceRepositorySingleton()->validateAll()) {
            std::cerr << "Cannot validate resource repository" << std::endl;
            IContainer::destroy(container);
            return false;
        }
        getFrameBufferObjectsRepositorySingleton()->setParams(0, 0, 1, 1, 0, 0, 0);
        if (!getFrameBufferObjectsRepositorySingleton()->validateAll()) {
            std::cerr << "Cannot validate frame buffer object repository" << std::endl;
            IContainer::destroy(container);
            return false;
        }

        char appendingHeader[2048];
        static const char kAppendingShaderHeader[] =
        #if 1
                "#if defined(GL_ES) || __VERSION__ >= 150\n"
                "precision highp float;\n"
                "#else\n"
                "#define highp\n"
                "#define mediump\n"
                "#define lowp\n"
                "#endif\n"
                "#if __VERSION__ >= 130\n"
                "#define vpvl2FXGetTexturePixel2D(samp, uv) texture(samp, (uv))\n"
                "#define layout(expr)\n"
                "#else\n"
                "#define vpvl2FXGetTexturePixel2D(samp, uv) texture2D(samp, (uv))\n"
                "#define layout(expr)\n"
                "#endif\n"
                "#if __VERSION__ >= 400\n"
                "#define vpvl2FXFMA(v, m, a) fma((v), (m), (a))\n"
                "#else\n"
                "#define vpvl2FXFMA(v, m, a) ((v) * (m) + (a))\n"
                "#endif\n"
                "#define vpvl2FXSaturate(v, t) clamp((v), t(0), t(1))\n"
        #else
                ""
        #endif
                ;
        if (isCoreProfileEnabled) {
            static const char kFormat[] = "#version %d core\n%s";
            snprintf(appendingHeader, sizeof(appendingHeader), kFormat, 150, kAppendingShaderHeader);
        }
        else {
            static const char kFormat[] = "#version %d\n%s";
            snprintf(appendingHeader, sizeof(appendingHeader), kFormat, 120, kAppendingShaderHeader);
        }
        int i = 0;
        while (IShader *shader = container->findShader(i++)) {
            TargetType type = shader->getType();
            const char *name = shader->getName();
            if (*name == '\0' && type == TGLSL) {
                shader->getExInterface()->addHeaderCode(appendingHeader);
            }
        }
        i = 0;
        ITechnique *technique = container->findTechnique(i);
        while (technique) {
            int j = 0;
            std::cerr << "technique[" << i << "]: " << technique->getName() << std::endl;
            IPass *pass = technique->getPass(j);
            while (pass) {
                bool validated = pass->validate();
                std::cerr << "pass[" << j << "]: " << pass->getName() << " validated=" << validated << std::endl;
                if (validated) {
                    WritePassShaders(pass);
                }
                pass = technique->getPass(++j);
            }
            technique = container->findTechnique(++i);
        }
    }
    else {
        std::cerr << "Cannot parse this effect: " << filename << std::endl;
    }
    IContainer::destroy(container);
    return true;
}

struct DefaultFunctionResolver : nvFX::FunctionResolver {
public:
    DefaultFunctionResolver() {}
    ~DefaultFunctionResolver() {}

    bool hasExtension(const char *name) const {
        return glfwExtensionSupported((std::string("GL_") + name).c_str());
    }
    void *resolve(const char *name) const {
        return reinterpret_cast<void *>(glfwGetProcAddress(name));
    }
    int queryVersion() const {
        if (const GLubyte *s = ::glGetString(GL_VERSION)) {
            int major = s[0] - '0', minor = s[2] - '0';
            return makeVersion(major, minor);
        }
        return 0;
    }
};

}

int main(int argc, char *argv[])
{
    atexit(glfwTerminate);
    glfwSetErrorCallback(HandleGLFWError);
    if (glfwInit() == GL_FALSE) {
        std::cerr << "Cannot initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }
    bool isCoreProfileEnabled = false;
#if 0 //def ENABLE_OPENGL_CORE_PROFILE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    isCoreProfileEnabled = true;
#endif
    GLFWwindow *window = glfwCreateWindow(1, 1, "nvFX", 0, 0);
    if (!window) {
        std::cerr << "Cannot create GLFWwindow" << std::endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    RegalSetErrorCallback(HandleRegalError);
    RegalMakeCurrent(CGLGetCurrentContext());
    std::cerr << "GL_VENDOR:     " << ::glGetString(GL_VENDOR)     << std::endl;
    std::cerr << "GL_VERSION:    " << ::glGetString(GL_VERSION)    << std::endl;
    std::cerr << "GL_RENDERER:   " << ::glGetString(GL_RENDERER)   << std::endl;
    if (isCoreProfileEnabled) {
        typedef const GLubyte * (GLAPIENTRY * PFNGLGETSTRINGIPROC) (GLenum pname, GLuint index);
        PFNGLGETSTRINGIPROC glGetStringi = reinterpret_cast<PFNGLGETSTRINGIPROC>(glfwGetProcAddress("glGetStringi"));
        GLint nextensions;
        ::glGetIntegerv(GL_NUM_EXTENSIONS, &nextensions);
        std::cerr << "GL_EXTENSIONS: ";
        for (GLint i = 0; i < nextensions; i++) {
            std::cerr << glGetStringi(GL_EXTENSIONS, i) << " ";
        }
        std::cerr << std::endl;
    }
    else {
        std::cerr << "GL_EXTENSIONS: " << ::glGetString(GL_EXTENSIONS) << std::endl;
    }

    int version = getVersion();
    ::printf("nvFX: v%d.%d\n", (version >> 16), (version & 0xffff));
    setErrorCallback(HandleNVFXError);
    setIncludeCallback(HandleIncludeCallback);
    static const DefaultFunctionResolver resolver;
    initializeOpenGLFunctions(&resolver);
    nvFX::initialize();
    for (int i = 1; i < argc; i++) {
        const char *filename = argv[i];
        if (!ParseEffect(filename, isCoreProfileEnabled)) {
            std::cerr << "Cannot parse this file: " << filename << std::endl;
        }
    }
    glfwDestroyWindow(window);
    nvFX::cleanup();

    return EXIT_SUCCESS;
}


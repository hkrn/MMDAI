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

#if defined(__APPLE__)
#include <OpenGL/CGLCurrent.h>
#else
#define CGLGetCurrentContext() 0
#endif

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

static bool ParseEffect(const char *filename, bool isCoreProfileEnabled)
{
    nvFX::IContainer *container = 0;
    container = nvFX::IContainer::create("nvFXcc");
    if (!container) {
        std::cerr << "Cannot create nvFX::IContainer" << std::endl;
        return false;
    }
    if (nvFX::loadEffectFromFile(container, filename)) {
        container->getExInterface()->separateShadersEnable(false);
        nvFX::getResourceRepositorySingleton()->setParams(0, 0, 1, 1, 1, 0, 0);
        if (!nvFX::getResourceRepositorySingleton()->validateAll()) {
            std::cerr << "Cannot validate resource repository" << std::endl;
            nvFX::IContainer::destroy(container);
            return false;
        }
        nvFX::getFrameBufferObjectsRepositorySingleton()->setParams(0, 0, 1, 1, 0, 0, 0);
        if (!nvFX::getFrameBufferObjectsRepositorySingleton()->validateAll()) {
            std::cerr << "Cannot validate frame buffer object repository" << std::endl;
            nvFX::IContainer::destroy(container);
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
        while (nvFX::IShader *shader = container->findShader(i++)) {
            nvFX::TargetType type = shader->getType();
            const char *name = shader->getName();
            if (*name == '\0' && type == nvFX::TGLSL) {
                shader->getExInterface()->addHeaderCode(appendingHeader);
            }
        }
        i = 0;
        nvFX::ITechnique *technique = container->findTechnique(i);
        while (technique) {
            int j = 0;
            std::cerr << "technique[" << i << "]: " << technique->getName() << std::endl;
            nvFX::IPass *pass = technique->getPass(j);
            char buffer[16384];
            while (pass) {
                bool validated = pass->validate();
                std::cerr << "pass[" << j << "]: " << pass->getName() << " validated=" << validated << std::endl;
                if (validated) {
                    nvFX::IProgram *program = pass->getExInterface()->getProgram(0);
                    GLsizei size = 0, count = 0;
                    GLuint shaders[2];
                    glGetAttachedShaders(program->getProgram(), sizeof(shaders) / sizeof(shaders[0]), &count, shaders);
                    glGetShaderSource(shaders[0], sizeof(buffer), &size, buffer);
                    if (FILE *fp = fopen((std::string(pass->getName()) + ".vert").c_str(), "wb")) {
                        fwrite(buffer, size, 1, fp);
                        fclose(fp);
                    }
                    glGetShaderSource(shaders[1], sizeof(buffer), &size, buffer);
                    if (FILE *fp = fopen((std::string(pass->getName()) + ".frag").c_str(), "wb")) {
                        fwrite(buffer, size, 1, fp);
                        fclose(fp);
                    }
                    std::cerr << "program[" << j << "]: " << program->getProgram() << std::endl;
                }
                pass = technique->getPass(++j);
            }
            technique = container->findTechnique(++i);
        }
    }
    else {
        std::cerr << "Cannot parse this effect: " << filename << std::endl;
    }
    nvFX::IContainer::destroy(container);
    return true;
}

struct FunctionResolver : nvFX::FunctionResolver {
public:
    FunctionResolver() {}
    ~FunctionResolver() {}

    bool hasExtension(const char *name) const {
        return glfwExtensionSupported((std::string("GL_") + name).c_str());
    }
    void *resolve(const char *name) const {
        return reinterpret_cast<void *>(glfwGetProcAddress(name));
    }
    int queryVersion() const {
        if (const GLubyte *s = glGetString(GL_VERSION)) {
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
    std::cerr << "GL_VENDOR:     " << glGetString(GL_VENDOR)     << std::endl;
    std::cerr << "GL_VERSION:    " << glGetString(GL_VERSION)    << std::endl;
    std::cerr << "GL_RENDERER:   " << glGetString(GL_RENDERER)   << std::endl;
    if (isCoreProfileEnabled) {
        typedef const GLubyte * (GLAPIENTRY * PFNGLGETSTRINGIPROC) (GLenum pname, GLuint index);
        PFNGLGETSTRINGIPROC glGetStringi = reinterpret_cast<PFNGLGETSTRINGIPROC>(glfwGetProcAddress("glGetStringi"));
        GLint nextensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &nextensions);
        std::cerr << "GL_EXTENSIONS: ";
        for (GLint i = 0; i < nextensions; i++) {
            std::cerr << glGetStringi(GL_EXTENSIONS, i) << " ";
        }
        std::cerr << std::endl;
    }
    else {
        std::cerr << "GL_EXTENSIONS: " << glGetString(GL_EXTENSIONS) << std::endl;
    }

    int version = nvFX::getVersion();
    nvFX::printf("nvFX: v%d.%d\n", (version >> 16), (version & 0xffff));
    nvFX::setErrorCallback(HandleNVFXError);
    nvFX::setIncludeCallback(HandleIncludeCallback);
    static const FunctionResolver resolver;
    nvFX::initializeOpenGLFunctions(&resolver);
    for (int i = 1; i < argc; i++) {
        const char *filename = argv[i];
        if (!ParseEffect(filename, isCoreProfileEnabled)) {
            std::cerr << "Cannot parse this file: " << filename << std::endl;
        }
    }
    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}


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

#include "../helper.h"
#include <vpvl2/extensions/glfw/RenderContext.h>

namespace {

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::glfw;

struct UIContext
{
    UIContext(GLFWwindow *windowRef, Scene *scene, StringMap *config, RenderContext *renderContextRef, const glm::vec2 &size)
        : sceneRef(scene),
          configRef(config),
          window(windowRef),
          renderContextRef(renderContextRef),
          width(size.x),
          height(size.y),
          prevX(0),
          prevY(0),
          restarted(glfwGetTime()),
          current(restarted),
          currentFPS(0),
          pressed(false)
    {
    }
    ~UIContext() {
        glfwDestroyWindow(window);
    }

    void updateFPS() {
        current = glfwGetTime();
        if (current - restarted > 1) {
#ifdef _MSC_VER
            _snprintf(title, sizeof(title), "libvpvl2 with GLFW (FPS:%d)", currentFPS);
#else
            snprintf(title, sizeof(title), "libvpvl2 with GLFW (FPS:%d)", currentFPS);
#endif
            glfwSetWindowTitle(window, title);
            restarted = current;
            currentFPS = 0;
        }
        currentFPS++;
    }

    static void handleKeyEvent(GLFWwindow *window, int key, int /* scancode */, int /* action */, int /* modifiers */) {
        const UIContext *context = static_cast<const UIContext *>(glfwGetWindowUserPointer(window));
        const Scalar degree(15.0);
        ICamera *camera = context->sceneRef->camera();
        switch (key) {
        case GLFW_KEY_RIGHT:
            camera->setAngle(camera->angle() + Vector3(0, degree, 0));
            break;
        case GLFW_KEY_LEFT:
            camera->setAngle(camera->angle() + Vector3(0, -degree, 0));
            break;
        case GLFW_KEY_UP:
            camera->setAngle(camera->angle() + Vector3(degree, 0, 0));
            break;
        case GLFW_KEY_DOWN:
            camera->setAngle(camera->angle() + Vector3(-degree, 0, 0));
            break;
        case GLFW_KEY_ESCAPE:
            glfwWindowShouldClose(window);
            break;
        default:
            break;
        }
    }
    static void handleMouseButton(GLFWwindow *window, int /* button */, int action, int /* modifiers */) {
        UIContext *context = static_cast<UIContext *>(glfwGetWindowUserPointer(window));
        context->pressed = action == GLFW_PRESS;
    }
    static void handleCursorPosition(GLFWwindow *window, double x, double y) {
        UIContext *context = static_cast<UIContext *>(glfwGetWindowUserPointer(window));
        if (context->pressed) {
            ICamera *camera = context->sceneRef->camera();
            if (context->prevX > 0 && context->prevY > 0) {
                const Scalar &factor = 0.5;
                camera->setAngle(camera->angle() + Vector3((y - context->prevY) * factor, (x - context->prevX) * factor, 0));
            }
            context->prevX = x;
            context->prevY = y;
        }
    }
    static void handleScroll(GLFWwindow *window, double /* x */, double y) {
        const UIContext *context = static_cast<const UIContext *>(glfwGetWindowUserPointer(window));
        ICamera *camera = context->sceneRef->camera();
        const Scalar &factor = 1.0;
        camera->setDistance(camera->distance() + y * factor);
    }
    static void handleWindowSize(GLFWwindow *window, int width, int height) {
        UIContext *context = static_cast<UIContext *>(glfwGetWindowUserPointer(window));
        context->width = width;
        context->height = height;
        glViewport(0, 0, width, height);
    }

    const Scene *sceneRef;
    const StringMap *configRef;
    GLFWwindow *window;
    RenderContext *renderContextRef;
    size_t width;
    size_t height;
    double prevX;
    double prevY;
    double restarted;
    double current;
    int currentFPS;
    char title[32];
    bool pressed;
};

static void UIErrorCallback(int err, const char *errstr)
{
    std::cerr << "errno=" << err << " errstr=" << errstr << std::endl;
}

struct MemoryMappedFile {
    MemoryMappedFile()
        : address(0),
          size(0),
          opaque(0)
    {
    }
    ~MemoryMappedFile() {
    }
    bool open(const UnicodeString &path) {
        return RenderContext::mapFileDescriptor(path, address, size, opaque);
    }
    void close() {
        RenderContext::unmapFileDescriptor(address, size, opaque);
    }
    uint8_t *address;
    size_t size;
    intptr_t opaque;
};

} /* namespace anonymous */

int main(int /* argc */, char *argv[])
{
    atexit(glfwTerminate);
    glfwSetErrorCallback(&UIErrorCallback);
    if (glfwInit() < 0) {
        std::cerr << "glfwInit() failed: " << std::endl;
        return EXIT_FAILURE;
    }

    StringMap settings;
    ui::loadSettings("config.ini", settings);
    const UnicodeString &path = settings.value("dir.system.data", UnicodeString())
            + "/" + Encoding::commonDataPath();
    MemoryMappedFile file;
    if (file.open(path)) {
        BaseRenderContext::initializeOnce(argv[0], reinterpret_cast<const char *>(file.address));
    }

    size_t width = settings.value("window.width", 640),
            height = settings.value("window.height", 480);
    int redSize = settings.value("opengl.size.red", 8),
            greenSize = settings.value("opengl.size.green", 8),
            blueSize = settings.value("opengl.size.blue", 8),
            alphaSize = settings.value("opengl.size.alpha", 8),
            depthSize = settings.value("opengl.size.depth", 24),
            stencilSize = settings.value("opengl.size.stencil", 8),
            samplesSize = settings.value("opengl.size.samples", 4);
    bool enableAA = settings.value("opengl.enable.aa", false);
    GLFWwindow *window = glfwCreateWindow(width, height, "libvpvl2 with GLFW (FPS:N/A)", 0, 0);
    glfwSetKeyCallback(window, &UIContext::handleKeyEvent);
    glfwSetMouseButtonCallback(window, &UIContext::handleMouseButton);
    glfwSetCursorPosCallback(window, &UIContext::handleCursorPosition);
    glfwSetScrollCallback(window, &UIContext::handleScroll);
    glfwSetWindowSizeCallback(window, &UIContext::handleWindowSize);
    if (!window) {
        std::cerr << "glfwCreateWindow() failed" << std::endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_RED_BITS, redSize);
    glfwWindowHint(GLFW_GREEN_BITS, greenSize);
    glfwWindowHint(GLFW_BLUE_BITS, blueSize);
    glfwWindowHint(GLFW_ALPHA_BITS, alphaSize);
    glfwWindowHint(GLFW_DEPTH_BITS, depthSize);
    glfwWindowHint(GLFW_STENCIL_BITS, stencilSize);
    glfwWindowHint(GLFW_SAMPLES, enableAA ? samplesSize : 0);
    GLenum err = 0;
    if (!Scene::initialize(&err)) {
        std::cerr << "Cannot initialize GLEW: " << err << std::endl;
        return EXIT_FAILURE;
    }
    std::cerr << "GL_VERSION:                " << glGetString(GL_VERSION) << std::endl;
    std::cerr << "GL_VENDOR:                 " << glGetString(GL_VENDOR) << std::endl;
    std::cerr << "GL_RENDERER:               " << glGetString(GL_RENDERER) << std::endl;

    Encoding::Dictionary dictionary;
    ui::initializeDictionary(settings, dictionary);
    WorldSmartPtr world(new World());
    EncodingSmartPtr encoding(new Encoding(&dictionary));
    FactorySmartPtr factory(new Factory(encoding.get()));
    SceneSmartPtr scene(new Scene(true));
    RenderContext renderContext(scene.get(), encoding.get(), &settings);
    bool ok = false;
    const UnicodeString &motionPath = settings.value("dir.motion", UnicodeString())
            + "/" + settings.value("file.motion", UnicodeString());
    if (settings.value("enable.opencl", false)) {
        scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
    }

    scene->light()->setToonEnable(settings.value("enable.toon", true));
    if (settings.value("enable.sm", false)) {
        int sw = settings.value("sm.width", 2048);
        int sh = settings.value("sm.height", 2048);
        renderContext.createShadowMap(Vector3(sw, sh, 0));
    }
    renderContext.updateCameraMatrices(glm::vec2(width, height));

    bool parallel = settings.value("enable.parallel", true);
    int nmodels = settings.value("models/size", 0);
    for (int i = 0; i < nmodels; i++) {
        std::ostringstream stream;
        stream << "models/" << (i + 1);
        const UnicodeString &prefix = UnicodeString::fromUTF8(stream.str()),
                &modelPath = settings.value(prefix + "/path", UnicodeString());
        int indexOf = modelPath.lastIndexOf("/");
        String dir(modelPath.tempSubString(0, indexOf));
        RenderContext::ModelContext modelContext(&renderContext, 0, &dir);
        RenderContext::MapBuffer modelBuffer(&renderContext);
        if (renderContext.mapFile(modelPath, &modelBuffer)) {
            int flags = settings.value(prefix + "/enable.effects", true) ? Scene::kEffectCapable : 0;
            IModelSmartPtr model(factory->createModel(modelBuffer.address, modelBuffer.size, ok));
            IRenderEngineSmartPtr engine(scene->createRenderEngine(&renderContext, model.get(), flags));
            IEffect *effectRef = 0;
            /*
             * BaseRenderContext#addModelPath() must be called before BaseRenderContext#createEffectRef()
             * because BaseRenderContext#createEffectRef() depends on BaseRenderContext#addModelPath() result
             * by BaseRenderContext#findModelPath() via BaseRenderContext#effectFilePath()
             */
            renderContext.addModelPath(model.get(), modelPath);
            if ((flags & Scene::kEffectCapable) != 0) {
                effectRef = renderContext.createEffectRef(model.get(), &dir);
                if (effectRef) {
                    effectRef->createFrameBufferObject();
                    engine->setEffect(effectRef, IEffect::kAutoDetection, &modelContext);
                }
            }
            if (engine->upload(&modelContext)) {
                renderContext.parseOffscreenSemantic(effectRef, &dir);
                engine->setUpdateOptions(parallel ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone);
                model->setEdgeWidth(settings.value(prefix + "/edge.width", 1.0f));
                scene->addModel(model.release(), engine.release(), i);
                RenderContext::MapBuffer motionBuffer(&renderContext);
                if (renderContext.mapFile(motionPath, &motionBuffer)) {
                    IMotionSmartPtr motion(factory->createMotion(motionBuffer.address,
                                                                 motionBuffer.size,
                                                                 model.get(), ok));
                    scene->addMotion(motion.release());
                }
            }
        }
    }

    UIContext context(window, scene.get(), &settings, &renderContext, glm::vec2(width, height));
    glfwSetWindowUserPointer(window, &context);
    double prev = glfwGetTime();
    scene->seek(0, Scene::kUpdateAll);
    scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    while (!glfwWindowShouldClose(window)) {
        renderContext.renderShadowMap();
        renderContext.renderOffscreen();
        renderContext.updateCameraMatrices(glm::vec2(context.width, context.height));
        ui::drawScreen(*context.sceneRef, context.width, context.height);
        double current = glfwGetTime();
        Scalar delta = ((current - prev) * 1000) / 60.0;
        prev = current;
        scene->advance(delta, Scene::kUpdateAll);
        world->stepSimulation(delta);
        scene->update(Scene::kUpdateAll);
        context.updateFPS();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    dictionary.releaseAll();
    /* explicitly release Scene instance to invalidation of Effect correctly before destorying RenderContext */
    scene.release();
    /* explicitly release World instance first to ensure release btRigidBody */
    world.release();

    return EXIT_SUCCESS;
}

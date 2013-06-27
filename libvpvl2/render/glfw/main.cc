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
#include <vpvl2/extensions/glfw/ApplicationContext.h>

namespace {

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::glfw;

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
        return ApplicationContext::mapFileDescriptor(path, address, size, opaque);
    }
    void close() {
        ApplicationContext::unmapFileDescriptor(address, size, opaque);
    }
    uint8 *address;
    vsize size;
    intptr_t opaque;
};

VPVL2_MAKE_SMARTPTR(ApplicationContext);

class Application {
public:
    Application()
        : m_window(0),
          m_world(new World()),
          m_scene(new Scene(true)),
          m_width(0),
          m_height(0),
          m_prevX(0),
          m_prevY(0),
          m_restarted(glfwGetTime()),
          m_current(m_restarted),
          m_currentFPS(0),
          m_pressed(false)
    {
    }
    ~Application() {
        m_dictionary.releaseAll();
        /* explicitly release Scene instance to invalidation of Effect correctly before destorying RenderContext */
        m_scene.release();
        /* explicitly release World instance first to ensure release btRigidBody */
        m_world.release();
        glfwDestroyWindow(m_window);
    }

    void updateFPS() {
        m_current = glfwGetTime();
        if (m_current - m_restarted > 1) {
#ifdef _MSC_VER
            _snprintf(title, sizeof(title), "libvpvl2 with GLFW (FPS:%d)", m_currentFPS);
#else
            snprintf(title, sizeof(title), "libvpvl2 with GLFW (FPS:%d)", m_currentFPS);
#endif
            glfwSetWindowTitle(m_window, title);
            m_restarted = m_current;
            m_currentFPS = 0;
        }
        m_currentFPS++;
    }
    bool initialize(const char *argv0) {
        atexit(glfwTerminate);
        glfwSetErrorCallback(&Application::handleError);
        if (glfwInit() < 0) {
            std::cerr << "glfwInit() failed: " << std::endl;
            return false;
        }
        ui::loadSettings("config.ini", m_config);
        const UnicodeString &path = m_config.value("dir.system.data", UnicodeString())
                + "/" + Encoding::commonDataPath();
        if (m_icuCommonData.open(path)) {
            if (!BaseApplicationContext::initializeOnce(argv0, reinterpret_cast<const char *>(m_icuCommonData.address))) {
                std::cerr << "BaseApplicatioContext::initializeOnce failed" << std::endl;
                return false;
            }
        }
        vsize width = m_width = m_config.value("window.width", 640),
                height = m_height = m_config.value("window.height", 480);
        int redSize = m_config.value("opengl.size.red", 8),
                greenSize = m_config.value("opengl.size.green", 8),
                blueSize = m_config.value("opengl.size.blue", 8),
                alphaSize = m_config.value("opengl.size.alpha", 8),
                depthSize = m_config.value("opengl.size.depth", 24),
                stencilSize = m_config.value("opengl.size.stencil", 8),
                samplesSize = m_config.value("opengl.size.samples", 4);
        bool enableAA = m_config.value("opengl.enable.aa", false);
        m_window = glfwCreateWindow(width, height, "libvpvl2 with GLFW (FPS:N/A)", 0, 0);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, &Application::handleKeyEvent);
        glfwSetMouseButtonCallback(m_window, &Application::handleMouseButton);
        glfwSetCursorPosCallback(m_window, &Application::handleCursorPosition);
        glfwSetScrollCallback(m_window, &Application::handleScroll);
        glfwSetWindowSizeCallback(m_window, &Application::handleWindowSize);
        if (!m_window) {
            std::cerr << "glfwCreateWindow() failed" << std::endl;
            return false;
        }
        glfwMakeContextCurrent(m_window);
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
            return false;
        }
        std::cerr << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;
        std::cerr << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
        std::cerr << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
        m_encoding.reset(new Encoding(&m_dictionary));
        m_factory.reset(new Factory(m_encoding.get()));
        m_applicationContext.reset(new ApplicationContext(m_scene.get(), m_encoding.get(), &m_config));
        return true;
    }
    void load() {
        if (m_config.value("enable.opencl", false)) {
            m_scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
        }
        m_scene->light()->setToonEnable(m_config.value("enable.toon", true));
        if (m_config.value("enable.sm", false)) {
            int sw = m_config.value("sm.width", 2048);
            int sh = m_config.value("sm.height", 2048);
            m_applicationContext->createShadowMap(Vector3(sw, sh, 0));
        }
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ui::initializeDictionary(m_config, m_dictionary);
        ui::loadAllModels(m_config, m_applicationContext.get(), m_scene.get(), m_factory.get(), m_encoding.get());
        m_scene->seek(0, Scene::kUpdateAll);
        m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    }
    bool isActive() const {
        return !glfwWindowShouldClose(m_window);
    }
    void handleFrame(double base, double &last) {
        m_applicationContext->renderShadowMap();
        m_applicationContext->renderOffscreen();
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ui::drawScreen(*m_scene.get(), m_width, m_height);
        double current = glfwGetTime();
        const IKeyframe::TimeIndex &timeIndex = IKeyframe::TimeIndex(((current - base) * 1000) / Scene::defaultFPS());
        m_scene->seek(timeIndex, Scene::kUpdateAll);
        m_world->stepSimulation(current - last);
        m_scene->update(Scene::kUpdateAll);
        updateFPS();
        last = current;
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

private:
    static void handleError(int err, const char *errstr) {
        std::cerr << "errno=" << err << " errstr=" << errstr << std::endl;
    }
    static void handleKeyEvent(GLFWwindow *window, int key, int /* scancode */, int /* action */, int /* modifiers */) {
        const Application *context = static_cast<const Application *>(glfwGetWindowUserPointer(window));
        const Scalar degree(15.0);
        ICamera *camera = context->m_scene->camera();
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
        Application *context = static_cast<Application *>(glfwGetWindowUserPointer(window));
        context->m_pressed = action == GLFW_PRESS;
    }
    static void handleCursorPosition(GLFWwindow *window, double x, double y) {
        Application *context = static_cast<Application *>(glfwGetWindowUserPointer(window));
        if (context->m_pressed) {
            ICamera *camera = context->m_scene->camera();
            if (context->m_prevX > 0 && context->m_prevY > 0) {
                const Scalar &factor = 0.5;
                camera->setAngle(camera->angle() + Vector3((y - context->m_prevY) * factor, (x - context->m_prevX) * factor, 0));
            }
            context->m_prevX = x;
            context->m_prevY = y;
        }
    }
    static void handleScroll(GLFWwindow *window, double /* x */, double y) {
        const Application *context = static_cast<const Application *>(glfwGetWindowUserPointer(window));
        ICamera *camera = context->m_scene->camera();
        const Scalar &factor = 1.0;
        camera->setDistance(camera->distance() + y * factor);
    }
    static void handleWindowSize(GLFWwindow *window, int width, int height) {
        Application *context = static_cast<Application *>(glfwGetWindowUserPointer(window));
        context->m_width = width;
        context->m_height = height;
        glViewport(0, 0, width, height);
    }

    GLFWwindow *m_window;
    StringMap m_config;
    Encoding::Dictionary m_dictionary;
    MemoryMappedFile m_icuCommonData;
    WorldSmartPtr m_world;
    EncodingSmartPtr m_encoding;
    FactorySmartPtr m_factory;
    SceneSmartPtr m_scene;
    ApplicationContextSmartPtr m_applicationContext;
    vsize m_width;
    vsize m_height;
    double m_prevX;
    double m_prevY;
    double m_restarted;
    double m_current;
    int m_currentFPS;
    char title[32];
    bool m_pressed;
};

} /* namespace anonymous */

int main(int /* argc */, char *argv[])
{
    Application application;
    if (!application.initialize(argv[0])) {
        return EXIT_FAILURE;
    }
    application.load();
    double base = glfwGetTime(), last = base;
    while (application.isActive()) {
        application.handleFrame(base, last);
    }
    return EXIT_SUCCESS;
}

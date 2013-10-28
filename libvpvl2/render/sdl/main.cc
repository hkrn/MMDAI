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
#include <vpvl2/extensions/sdl/ApplicationContext.h>

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::sdl;

#ifdef VPVL2_LINK_ATB
#include <vpvl2/extensions/ui/AntTweakBar.h>
using namespace vpvl2::extensions::ui;
#endif

namespace {

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
          m_contextGL(0),
          m_world(new World()),
          m_scene(new Scene(true)),
          m_width(0),
          m_height(0),
          m_restarted(SDL_GetTicks()),
          m_current(m_restarted),
          m_currentFPS(0),
          m_active(true),
          m_pressed(false)
    {
    }
    ~Application() {
        SDL_GL_DeleteContext(m_contextGL);
        SDL_DestroyWindow(m_window);
        m_dictionary.releaseAll();
        /* explicitly release Scene instance to invalidation of Effect correctly before destorying RenderContext */
        m_scene.release();
        /* explicitly release World instance first to ensure release btRigidBody */
        m_world.release();
        SDL_Quit();
    }

    bool initialize() {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            std::cerr << "SDL_Init(SDL_INIT_EVERYTHING) failed: " << SDL_GetError() << std::endl;
            return false;
        }
#if 0
        atexit(IMG_Quit);
        if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0) {
            std::cerr << "IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) failed: " << SDL_GetError() << std::endl;
            return false;
        }
#endif
        char *configDir = SDL_GetBasePath();
        std::string configPath(configDir);
        SDL_free(configDir);
        configPath.append("config.ini");
        ::ui::loadSettings(configPath, m_config);
        bool enableDebug = m_config.value("opengl.enable.debug", false);
        if (!initializeWindow(enableDebug)) {
            return false;
        }
        m_encoding.reset(new Encoding(&m_dictionary));
        m_factory.reset(new Factory(m_encoding.get()));
        m_applicationContext.reset(new ApplicationContext(m_scene.get(), m_encoding.get(), &m_config));
        m_applicationContext->initialize(enableDebug);
#ifdef VPVL2_LINK_ATB
        AntTweakBar::initialize(m_config.value("opengl.enable.core", false));
        m_controller.create(m_applicationContext.get());
#endif
        return true;
    }
    void load() {
        if (m_config.value("enable.opencl", false)) {
            m_scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
        }
        m_scene->lightRef()->setToonEnable(m_config.value("enable.toon", true));
        if (m_config.value("enable.sm", false)) {
            int sw = m_config.value("sm.width", 2048);
            int sh = m_config.value("sm.height", 2048);
            m_applicationContext->createShadowMap(Vector3(sw, sh, 0));
            const Vector3 &direction = m_scene->lightRef()->direction(), &eye = -direction * 100, &center = direction * 100;
            const glm::mat4 &view = glm::lookAt(glm::vec3(eye.x(), eye.y(), eye.z()), glm::vec3(center.x(), center.y(), center.z()), glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::mat4 &projection = glm::infinitePerspective(45.0f, sw / float(sh), 0.1f);
            m_applicationContext->setLightMatrices(glm::mat4(), view, projection);
        }
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ::ui::initializeDictionary(m_config, m_dictionary);
        ::ui::loadAllModels(m_config, m_applicationContext.get(), m_scene.get(), m_factory.get(), m_encoding.get());
        m_scene->setWorldRef(m_world->dynamicWorldRef());
        m_scene->seek(0, Scene::kUpdateAll);
        m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
#ifdef VPVL2_LINK_ATB
        m_controller.resize(m_width, m_height);
        m_controller.setCurrentModelRef(m_applicationContext->currentModelRef());
#endif
    }
    bool isActive() const {
        return m_active;
    }
    void handleFrame(Uint32 base, Uint32 &last) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_MOUSEMOTION: {
                handleMouseMotion(event.motion);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
#ifdef VPVL2_LINK_ATB
                IApplicationContext::MousePositionType type = static_cast<IApplicationContext::MousePositionType>(event.button.button);
                m_controller.handleAction(type, event.type == SDL_MOUSEBUTTONDOWN);
#endif
                m_pressed = event.type == SDL_MOUSEBUTTONDOWN;
                break;
            }
            case SDL_KEYDOWN: {
                handleKeyEvent(event.key);
                break;
            }
            case SDL_MOUSEWHEEL: {
                handleMouseWheel(event.wheel);
                break;
            }
            case SDL_QUIT: {
                m_active = false;
                break;
            }
            default:
                break;
            }
        }
        m_applicationContext->renderShadowMap();
        m_applicationContext->renderOffscreen();
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ::ui::drawScreen(*m_scene.get(), m_width, m_height);
        Uint32 current = SDL_GetTicks();
        const IKeyframe::TimeIndex &timeIndex = IKeyframe::TimeIndex((current - base) / Scene::defaultFPS());
        m_scene->seek(timeIndex, Scene::kUpdateAll);
        m_world->stepSimulation(current - last);
        m_scene->update(Scene::kUpdateAll);
        updateFPS();
        last = current;
#ifdef VPVL2_LINK_ATB
        m_controller.render();
#endif
        SDL_GL_SwapWindow(m_window);
    }

private:
    bool initializeWindow(bool enableDebug) {
        vsize w = m_width = m_config.value("window.width", 640),
                h = m_height = m_config.value("window.height", 480);
        int redSize = m_config.value("opengl.size.red", 8),
                greenSize = m_config.value("opengl.size.green", 8),
                blueSize = m_config.value("opengl.size.blue", 8),
                alphaSize = m_config.value("opengl.size.alpha", 8),
                depthSize = m_config.value("opengl.size.depth", 24),
                stencilSize = m_config.value("opengl.size.stencil", 8),
                samplesSize = m_config.value("opengl.size.samples", 4),
                flags = 0;
        bool enableSW = m_config.value("opengl.enable.software", false),
                enableAA = m_config.value("opengl.enable.aa", false),
                enableCore = m_config.value("opengl.enable.core", false);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, redSize);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, greenSize);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, blueSize);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, alphaSize);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthSize);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilSize);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, enableAA ? 1 : 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, enableAA ? samplesSize : 0);
        if (enableCore) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
        }
        if (enableDebug) {
            flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
        }
        if (enableSW) {
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
        m_window = SDL_CreateWindow("libvpvl2 with SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    w, h, SDL_WINDOW_OPENGL);
        if (!m_window) {
            std::cerr << "SDL_CreateWindow(title, x, y, width, height, SDL_OPENGL) failed: " << SDL_GetError() << std::endl;
            return false;
        }
        m_contextGL = SDL_GL_CreateContext(m_window);
        if (!m_contextGL) {
            std::cerr << "SDL_GL_CreateContext(window) failed: " << SDL_GetError() << std::endl;
            return false;
        }
        if (!Scene::initialize(ApplicationContext::staticSharedFunctionResolverInstance())) {
            std::cerr << "Cannot initialize scene";
            return false;
        }
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
        SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &value);
        std::cerr << "SDL_GL_ALPHA_SIZE:         " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
        std::cerr << "SDL_GL_DEPTH_SIZE:         " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value);
        std::cerr << "SDL_GL_STENCIL_SIZE:       " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value);
        std::cerr << "SDL_GL_DOUBLEBUFFER:       " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
        std::cerr << "SDL_GL_MULTISAMPLEBUFFERS: " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
        std::cerr << "SDL_GL_MULTISAMPLESAMPLES: " << value << std::endl;
        SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
        std::cerr << "SDL_GL_ACCELERATED_VISUAL: " << value << std::endl;
        return true;
    }
    void handleKeyEvent(const SDL_KeyboardEvent &event) {
        const SDL_Keysym &keysym = event.keysym;
        const Scalar degree(15.0);
        ICamera *camera = m_scene->cameraRef();
        switch (keysym.sym) {
        case SDLK_RIGHT:
            camera->setAngle(camera->angle() + Vector3(0, degree, 0));
            break;
        case SDLK_LEFT:
            camera->setAngle(camera->angle() + Vector3(0, -degree, 0));
            break;
        case SDLK_UP:
            camera->setAngle(camera->angle() + Vector3(degree, 0, 0));
            break;
        case SDLK_DOWN:
            camera->setAngle(camera->angle() + Vector3(-degree, 0, 0));
            break;
        case SDLK_ESCAPE:
            m_active = false;
            break;
        default:
            break;
        }
    }
    void handleMouseMotion(const SDL_MouseMotionEvent &event) {
        bool handled = false;
#ifdef VPVL2_LINK_ATB
        handled = m_controller.handleMotion(event.x, event.y);
#endif
        if (!handled && event.state == SDL_PRESSED) {
            ICamera *camera = m_scene->cameraRef();
            const Scalar &factor = 0.5;
            camera->setAngle(camera->angle() + Vector3(event.yrel * factor, event.xrel * factor, 0));
        }
    }
    void handleMouseWheel(const SDL_MouseWheelEvent &event) {
        bool handled = false;
        int delta = event.y;
#ifdef VPVL2_LINK_ATB
        handled = m_controller.handleWheel(delta);
#endif
        if (!handled) {
            const Scalar &factor = 1.0;
            ICamera *camera = m_scene->cameraRef();
            camera->setDistance(camera->distance() + delta * factor);
            if (m_pressed) {
                const Matrix3x3 &m = camera->modelViewTransform().getBasis();
                const Vector3 &v = m[0] * event.x * factor;
                camera->setLookAt(camera->lookAt() + v);
            }
        }
    }
    void updateFPS() {
        m_current = SDL_GetTicks();
        if (m_current - m_restarted > 1000) {
#ifdef _MSC_VER
            _snprintf(m_title, sizeof(m_title), "libvpvl2 with SDL2 (FPS:%d)", m_currentFPS);
#else
            snprintf(m_title, sizeof(m_title), "libvpvl2 with SDL2 (FPS:%d)", m_currentFPS);
#endif
            SDL_SetWindowTitle(m_window, m_title);
            m_restarted = m_current;
            m_currentFPS = 0;
        }
        m_currentFPS++;
    }

    SDL_Window *m_window;
    SDL_GLContext m_contextGL;
#ifdef VPVL2_LINK_ATB
    AntTweakBar m_controller;
#endif
    StringMap m_config;
    Encoding::Dictionary m_dictionary;
    WorldSmartPtr m_world;
    EncodingSmartPtr m_encoding;
    FactorySmartPtr m_factory;
    SceneSmartPtr m_scene;
    ApplicationContextSmartPtr m_applicationContext;
    vsize m_width;
    vsize m_height;
    Uint32 m_restarted;
    Uint32 m_current;
    int m_currentFPS;
    char m_title[32];
    bool m_active;
    bool m_pressed;
};

} /* namespace anonymous */

int main(int /* argc */, char *argv[])
{
    Application application;
    tbb::task_scheduler_init initializer; (void) initializer;
    BaseApplicationContext::initializeOnce(argv[0], 0, 2);
    if (!application.initialize()) {
        BaseApplicationContext::terminate();
        return EXIT_FAILURE;
    }
    application.load();
    Uint32 base = SDL_GetTicks(), last = base;
    SDL_DisableScreenSaver();
    while (application.isActive()) {
        application.handleFrame(base, last);
    }
    SDL_EnableScreenSaver();
    BaseApplicationContext::terminate();
    return EXIT_SUCCESS;
}

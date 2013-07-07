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
#include <vpvl2/extensions/allegro/ApplicationContext.h>

#ifdef VPVL2_LINK_ATB
#include <vpvl2/extensions/ui/AntTweakBar.h>
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::allegro;
using namespace vpvl2::extensions::ui;

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
        : m_display(0),
          m_queue(0),
          m_world(new World()),
          m_scene(new Scene(true)),
          m_width(0),
          m_height(0),
          m_restarted(0),
          m_current(0),
          m_prevX(0),
          m_prevY(0),
          m_prevZ(0),
          m_prevW(0),
          m_currentFPS(0),
          m_mousePressed(false),
          m_active(true)
    {
    }
    ~Application() {
        if (m_display) {
            al_destroy_display(m_display);
            m_display = 0;
        }
        if (m_queue) {
            al_destroy_event_queue(m_queue);
            m_queue = 0;
        }
        m_dictionary.releaseAll();
        /* explicitly release Scene instance to invalidation of Effect correctly before destorying RenderContext */
        m_scene.release();
        /* explicitly release World instance first to ensure release btRigidBody */
        m_world.release();
        al_uninstall_system();
    }

    bool initialize() {
        ::ui::loadSettings("config.ini", m_config);
        if (!initializeWindow()) {
            return false;
        }
        m_encoding.reset(new Encoding(&m_dictionary));
        m_factory.reset(new Factory(m_encoding.get()));
        m_applicationContext.reset(new ApplicationContext(m_scene.get(), m_encoding.get(), &m_config));
#ifdef VPVL2_LINK_ASSIMP
        AntTweakBar::initialize();
        m_controller.create(m_applicationContext.get());
#endif
        m_restarted = al_get_time();
        m_current = m_restarted;
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
        }
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ::ui::initializeDictionary(m_config, m_dictionary);
        ::ui::loadAllModels(m_config, m_applicationContext.get(), m_scene.get(), m_factory.get(), m_encoding.get());
        m_scene->seek(0, Scene::kUpdateAll);
        m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
#ifdef VPVL2_LINK_ATB
        m_controller.resize(m_width, m_height);
        m_controller.setCurrentModelRef(m_applicationContext->currentModelRef());
#endif
        al_flush_event_queue(m_queue);
    }
    bool isActive() const {
        return m_active;
    }
    void handleFrame(const double &base, double &last) {
        if (!al_is_event_queue_empty(m_queue)) {
            ALLEGRO_EVENT event;
            while (al_get_next_event(m_queue, &event)) {
                switch (event.type) {
                case ALLEGRO_EVENT_DISPLAY_DISCONNECTED: {
                    handleMouseMotion(event.mouse);
                    break;
                }
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
                    const ALLEGRO_MOUSE_EVENT &mouse = event.mouse;
                    IApplicationContext::MousePositionType type(IApplicationContext::kMouseCursorPosition);
                    if ((mouse.button & 1) == 1) {
                        type = IApplicationContext::kMouseLeftPressPosition;
                    }
                    else if ((mouse.button & 4) == 4) {
                        type = IApplicationContext::kMouseMiddlePressPosition;
                    }
                    else if ((mouse.button & 2) == 2) {
                        type = IApplicationContext::kMouseRightPressPosition;
                    }
                    m_mousePressed = event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
#ifdef VPVL2_LINK_ATB
                    m_controller.handleAction(type, m_mousePressed);
#endif
                    break;
                }
                case ALLEGRO_EVENT_KEY_DOWN: {
                    handleKeyEvent(event.keyboard);
                    break;
                }
                case ALLEGRO_EVENT_MOUSE_AXES: {
                    handleMouseMotion(event.mouse);
                    break;
                }
                case ALLEGRO_EVENT_DISPLAY_RESIZE: {
                    const ALLEGRO_DISPLAY_EVENT &display = event.display;
                    int w = display.width, h = display.height;
                    glViewport(0, 0, w, h);
                    m_applicationContext->updateCameraMatrices(glm::vec2(w, h));
                    al_acknowledge_resize(m_display);
                    break;
                }
                default:
                    break;
                }
            }
        }
        m_applicationContext->renderShadowMap();
        m_applicationContext->renderOffscreen();
        m_applicationContext->updateCameraMatrices(glm::vec2(m_width, m_height));
        ::ui::drawScreen(*m_scene.get(), m_width, m_height);
        double current = al_get_time() - base;
        const IKeyframe::TimeIndex &timeIndex = IKeyframe::TimeIndex((current * 1000) / Scene::defaultFPS());
        m_scene->seek(timeIndex, Scene::kUpdateAll);
        m_world->stepSimulation(current - last);
        m_scene->update(Scene::kUpdateAll);
        updateFPS();
        last = current;
#ifdef VPVL2_LINK_ATB
        m_controller.render();
#endif
        al_flip_display();
    }

private:
    bool initializeWindow() {
        vsize width = m_width = m_config.value("window.width", 640),
                height = m_height = m_config.value("window.height", 480);
        int redSize = m_config.value("opengl.size.red", 8),
                greenSize = m_config.value("opengl.size.green", 8),
                blueSize = m_config.value("opengl.size.blue", 8),
                alphaSize = m_config.value("opengl.size.alpha", 8),
                depthSize = m_config.value("opengl.size.depth", 24),
                stencilSize = m_config.value("opengl.size.stencil", 8),
                samplesSize = m_config.value("opengl.size.samples", 4);
        bool /* enableSW = m_config.value("opengl.enable.software", false), */
                enableAA = m_config.value("opengl.enable.aa", false);
        al_set_new_display_flags(ALLEGRO_OPENGL_3_0 | ALLEGRO_OPENGL_FORWARD_COMPATIBLE | ALLEGRO_RESIZABLE);
        al_set_new_display_option(ALLEGRO_RED_SIZE, redSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_GREEN_SIZE, greenSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_BLUE_SIZE, blueSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_ALPHA_SIZE, alphaSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_DEPTH_SIZE, depthSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_STENCIL_SIZE, stencilSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_SAMPLES, samplesSize, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, enableAA ? 1 : 0, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
        m_queue = al_create_event_queue();
        m_display = al_create_display(width, height);
        GLenum err = 0;
        if (!Scene::initialize(&err)) {
            std::cerr << "Cannot initialize GLEW: " << err << std::endl;
            return false;
        }
        std::cerr << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;
        std::cerr << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
        std::cerr << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
        std::cerr << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        al_register_event_source(m_queue, al_get_mouse_event_source());
        al_register_event_source(m_queue, al_get_keyboard_event_source());
        al_register_event_source(m_queue, al_get_display_event_source(m_display));
        return true;
    }
    void handleKeyEvent(const ALLEGRO_KEYBOARD_EVENT &event) {
        const Scalar degree(15.0);
        ICamera *camera = m_scene->cameraRef();
        switch (event.keycode) {
        case ALLEGRO_KEY_RIGHT:
            camera->setAngle(camera->angle() + Vector3(0, degree, 0));
            break;
        case ALLEGRO_KEY_LEFT:
            camera->setAngle(camera->angle() + Vector3(0, -degree, 0));
            break;
        case ALLEGRO_KEY_UP:
            camera->setAngle(camera->angle() + Vector3(degree, 0, 0));
            break;
        case ALLEGRO_KEY_DOWN:
            camera->setAngle(camera->angle() + Vector3(-degree, 0, 0));
            break;
        case ALLEGRO_KEY_ESCAPE:
            m_active = false;
            break;
        default:
            break;
        }
    }
    void handleMouseMotion(const ALLEGRO_MOUSE_EVENT &event) {
        bool handleMotion = false, handleWheel = false;
        int x = event.x, y = event.y, z = event.z, w = event.w;
#ifdef VPVL2_LINK_ATB
        handleMotion = m_controller.handleMotion(x, y);
        handleWheel = m_controller.handleWheel(z);
#endif
        if (!handleMotion && m_mousePressed) {
            ICamera *camera = m_scene->cameraRef();
            if (m_prevX > 0 && m_prevY > 0) {
                const Scalar &factor = 0.5;
                camera->setAngle(camera->angle() + Vector3((y - m_prevY) * factor, (x - m_prevX) * factor, 0));
            }
        }
        if (!handleWheel) {
            const Scalar &factor = 1.0;
            ICamera *camera = m_scene->cameraRef();
            if (m_prevZ != 0 || m_prevW != 0) {
                camera->setDistance(camera->distance() + (z - m_prevZ) * factor);
                const Matrix3x3 &m = camera->modelViewTransform().getBasis();
                const Vector3 &v = m[0] * (w - m_prevW) * factor;
                camera->setLookAt(camera->lookAt() + v);
            }
        }
        m_prevX = x;
        m_prevY = y;
        m_prevZ = z;
        m_prevW = w;
    }
    void updateFPS() {
        m_current = al_get_time();
        if (m_current - m_restarted > 1) {
#ifdef _MSC_VER
            _snprintf(m_title, sizeof(m_title), "libvpvl2 with Allegro (FPS:%d)", m_currentFPS);
#else
            snprintf(m_title, sizeof(m_title), "libvpvl2 with Allegro (FPS:%d)", m_currentFPS);
#endif
            al_set_window_title(m_display, m_title);
            m_restarted = m_current;
            m_currentFPS = 0;
        }
        m_currentFPS++;
    }

    ALLEGRO_DISPLAY *m_display;
    ALLEGRO_EVENT_QUEUE *m_queue;
#ifdef VPVL2_LINK_ASSIMP
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
    double m_restarted;
    double m_current;
    int m_prevX;
    int m_prevY;
    int m_prevZ;
    int m_prevW;
    int m_currentFPS;
    char m_title[32];
    bool m_mousePressed;
    bool m_active;
};

} /* namespace anonymous */

int main(int /* argc */, char *argv[])
{
    Application application;
    tbb::task_scheduler_init initializer; (void) initializer;
    if (!al_init()) {
        std::cerr << "Cannot initialize Allegro" << std::endl;
        return EXIT_FAILURE;
    }
    if (!al_install_keyboard()) {
        std::cerr << "Cannot install keyboard" << std::endl;
        return EXIT_FAILURE;
    }
    if (!al_install_mouse()) {
        std::cerr << "Cannot install mouse" << std::endl;
        return EXIT_FAILURE;
    }
    BaseApplicationContext::initializeOnce(argv[0]);
    if (!application.initialize()) {
        BaseApplicationContext::terminate();
        return EXIT_FAILURE;
    }
    application.load();
    al_inhibit_screensaver(true);
    double base = al_get_time(), last;
    while (application.isActive()) {
        application.handleFrame(base, last);
    }
    al_inhibit_screensaver(false);
    BaseApplicationContext::terminate();
    return EXIT_SUCCESS;
}

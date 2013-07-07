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
#include <vpvl2/extensions/sfml/ApplicationContext.h>

#ifdef VPVL2_LINK_ATB
#include <vpvl2/extensions/ui/AntTweakBar.h>
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::sfml;
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
        : m_window(0),
          m_world(new World()),
          m_scene(new Scene(true)),
          m_width(0),
          m_height(0),
          m_restarted(m_clock.getElapsedTime()),
          m_current(m_restarted),
          m_prevX(0),
          m_prevY(0),
          m_currentFPS(0),
          m_mousePressed(false)
    {
    }
    ~Application() {
        delete m_window;
        m_dictionary.releaseAll();
        /* explicitly release Scene instance to invalidation of Effect correctly before destorying RenderContext */
        m_scene.release();
        /* explicitly release World instance first to ensure release btRigidBody */
        m_world.release();
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
    }
    bool isActive() const {
        return m_window->isOpen();
    }
    void handleFrame(const sf::Clock &base, sf::Time &last) {
        sf::Event event;
        while (m_window->pollEvent(event)) {
            switch (event.type) {
            case sf::Event::MouseMoved: {
                handleMouseMotion(event.mouseMove);
                break;
            }
            case sf::Event::MouseButtonPressed:
            case sf::Event::MouseButtonReleased: {
                const sf::Event::MouseButtonEvent &mouseButton = event.mouseButton;
                IApplicationContext::MousePositionType type(IApplicationContext::kMouseCursorPosition);
                switch (mouseButton.button) {
                case sf::Mouse::Left:
                    type = IApplicationContext::kMouseLeftPressPosition;
                    break;
                case sf::Mouse::Middle:
                    type = IApplicationContext::kMouseMiddlePressPosition;
                    break;
                case sf::Mouse::Right:
                    type = IApplicationContext::kMouseRightPressPosition;
                    break;
                default:
                    break;
                }
                m_mousePressed = event.type == sf::Event::MouseButtonPressed;
#ifdef VPVL2_LINK_ATB
                m_controller.handleAction(type, m_mousePressed);
#endif
                break;
            }
            case sf::Event::KeyPressed: {
                handleKeyEvent(event.key);
                break;
            }
            case sf::Event::MouseWheelMoved: {
                handleMouseWheel(event.mouseWheel);
                break;
            }
            case sf::Event::Resized: {
                const sf::Event::SizeEvent &size = event.size;
                int w = size.width, h = size.height;
                glViewport(0, 0, w, h);
                m_applicationContext->updateCameraMatrices(glm::vec2(w, h));
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
        sf::Time current = base.getElapsedTime();
        const IKeyframe::TimeIndex &timeIndex = IKeyframe::TimeIndex(current.asMilliseconds() / Scene::defaultFPS());
        VPVL2_LOG(INFO, timeIndex << ":" << current.asSeconds() << ":" << last.asSeconds());
        m_scene->seek(timeIndex, Scene::kUpdateAll);
        m_world->stepSimulation((current - last).asMilliseconds());
        m_scene->update(Scene::kUpdateAll);
        updateFPS();
        last = current;
#ifdef VPVL2_LINK_ATB
        m_controller.render();
#endif
        m_window->display();
    }

private:
    bool initializeWindow() {
        vsize w = m_width = m_config.value("window.width", 640),
                h = m_height = m_config.value("window.height", 480);
        int /* redSize = m_config.value("opengl.size.red", 8),
                                        greenSize = m_config.value("opengl.size.green", 8),
                                        blueSize = m_config.value("opengl.size.blue", 8),
                                        alphaSize = m_config.value("opengl.size.alpha", 8), */
                depthSize = m_config.value("opengl.size.depth", 24),
                stencilSize = m_config.value("opengl.size.stencil", 8),
                samplesSize = m_config.value("opengl.size.samples", 4);
        bool /* enableSW = m_config.value("opengl.enable.software", false), */
                enableAA = m_config.value("opengl.enable.aa", false);
        sf::ContextSettings settings;
        settings.antialiasingLevel = enableAA ? samplesSize : 0;
        settings.depthBits = depthSize;
        settings.stencilBits = stencilSize;
        m_window = new sf::RenderWindow(sf::VideoMode(w, h), "libvpvl2 with SFML", sf::Style::Default, settings);
        GLenum err = 0;
        if (!Scene::initialize(&err)) {
            std::cerr << "Cannot initialize GLEW: " << err << std::endl;
            return false;
        }
        std::cerr << "GL_VERSION:                " << glGetString(GL_VERSION) << std::endl;
        std::cerr << "GL_VENDOR:                 " << glGetString(GL_VENDOR) << std::endl;
        std::cerr << "GL_RENDERER:               " << glGetString(GL_RENDERER) << std::endl;
        return true;
    }
    void handleKeyEvent(const sf::Event::KeyEvent &event) {
        const sf::Keyboard::Key &keysym = event.code;
        const Scalar degree(15.0);
        ICamera *camera = m_scene->cameraRef();
        switch (keysym) {
        case sf::Keyboard::Right:
            camera->setAngle(camera->angle() + Vector3(0, degree, 0));
            break;
        case sf::Keyboard::Left:
            camera->setAngle(camera->angle() + Vector3(0, -degree, 0));
            break;
        case sf::Keyboard::Up:
            camera->setAngle(camera->angle() + Vector3(degree, 0, 0));
            break;
        case sf::Keyboard::Down:
            camera->setAngle(camera->angle() + Vector3(-degree, 0, 0));
            break;
        case sf::Keyboard::Escape:
            m_window->close();
            break;
        default:
            break;
        }
    }
    void handleMouseMotion(const sf::Event::MouseMoveEvent &event) {
        bool handled = false;
        int x = event.x, y = event.y;
#ifdef VPVL2_LINK_ATB
        handled = m_controller.handleMotion(x, y);
#endif
        if (!handled && m_mousePressed) {
            ICamera *camera = m_scene->cameraRef();
            if (m_prevX > 0 && m_prevY > 0) {
                const Scalar &factor = 0.5;
                camera->setAngle(camera->angle() + Vector3((y - m_prevY) * factor, (x - m_prevX) * factor, 0));
            }
        }
        m_prevX = x;
        m_prevY = y;
    }
    void handleMouseWheel(const sf::Event::MouseWheelEvent &event) {
        bool handled = false;
#ifdef VPVL2_LINK_ATB
        handled = m_controller.handleWheel(event.y);
#endif
        if (!handled) {
            const Scalar &factor = 1.0;
            ICamera *camera = m_scene->cameraRef();
            camera->setDistance(camera->distance() + event.delta * factor);
        }
    }
    void updateFPS() {
        m_current = m_clock.getElapsedTime();
        if ((m_current - m_restarted).asMilliseconds() > 1000) {
#ifdef _MSC_VER
            _snprintf(m_title, sizeof(m_title), "libvpvl2 with SDL2 (FPS:%d)", m_currentFPS);
#else
            snprintf(m_title, sizeof(m_title), "libvpvl2 with SDL2 (FPS:%d)", m_currentFPS);
#endif
            m_window->setTitle(m_title);
            m_restarted = m_current;
            m_currentFPS = 0;
        }
        m_currentFPS++;
    }

    sf::RenderWindow *m_window;
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
    sf::Clock m_clock;
    sf::Time m_restarted;
    sf::Time m_current;
    int m_prevX;
    int m_prevY;
    int m_currentFPS;
    char m_title[32];
    bool m_mousePressed;
};

} /* namespace anonymous */

int main(int /* argc */, char *argv[])
{
    Application application;
    tbb::task_scheduler_init initializer; (void) initializer;
    BaseApplicationContext::initializeOnce(argv[0]);
    if (!application.initialize()) {
        BaseApplicationContext::terminate();
        return EXIT_FAILURE;
    }
    application.load();
    sf::Clock base;
    sf::Time last;
    while (application.isActive()) {
        application.handleFrame(base, last);
    }
    BaseApplicationContext::terminate();
    return EXIT_SUCCESS;
}

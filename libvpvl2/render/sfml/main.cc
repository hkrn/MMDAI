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
#include <vpvl2/extensions/sfml/RenderContext.h>

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::sfml;

static void UIDrawScreen(const Scene &scene, size_t width, size_t height)
{
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    const Array<IRenderEngine *> &engines = scene.renderEngines();
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
}

static void UIUpdateCamera(const Scene &scene, size_t width, size_t height, RenderContext &renderContext)
{
    const ICamera *camera = scene.camera();
    Scalar matrix[16];
    camera->modelViewTransform().getOpenGLMatrix(matrix);
    const float &aspect = width / float(height);
    const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
            &projection = glm::perspective(camera->fov(), aspect, camera->znear(), camera->zfar());
    renderContext.setCameraMatrix(world, view, projection);
}

int main(int /* argc */, char ** /* argv */)
{
    std::ifstream stream("config.ini");
    std::string line;
    UIStringMap config;
    UnicodeString k, v;
    while (stream && std::getline(stream, line)) {
        if (line.empty() || line.find_first_of("#;") != std::string::npos)
            continue;
        std::istringstream ss(line);
        std::string key, value;
        std::getline(ss, key, '=');
        std::getline(ss, value);
        k.setTo(UnicodeString::fromUTF8(key));
        v.setTo(UnicodeString::fromUTF8(value));
        config[k.trim()] = v.trim();
    }

    size_t width = vpvl2::extensions::icu::String::toInt(config["window.width"], 640),
            height = vpvl2::extensions::icu::String::toInt(config["window.height"], 480);
    int  depthSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.depth"], 24),
            stencilSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.stencil"], 8),
            samplesSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.samples"], 4);

    sf::VideoMode videoMode(width, height);
    sf::ContextSettings settings(depthSize, stencilSize, samplesSize);
    sf::RenderWindow window(videoMode, "libvpvl2 SFML rendering test", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

#ifdef VPVL2_LINK_GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(err) << std::endl;
        return EXIT_FAILURE;
    }
#endif
    std::cerr << "GL_VERSION:        " << glGetString(GL_VERSION) << std::endl;
    std::cerr << "GL_VENDOR:         " << glGetString(GL_VENDOR) << std::endl;
    std::cerr << "GL_RENDERER:       " << glGetString(GL_RENDERER) << std::endl;
    const sf::ContextSettings &actualSetting = window.getSettings();
    std::cerr << "antialiasingLevel: " << actualSetting.antialiasingLevel << std::endl;
    std::cerr << "depthBits:         " << actualSetting.depthBits << std::endl;
    std::cerr << "majorVersion:      " << actualSetting.majorVersion << std::endl;
    std::cerr << "minorVersion:      " << actualSetting.minorVersion << std::endl;
    std::cerr << "stencilBits:       " << actualSetting.stencilBits << std::endl;

    Encoding encoding;
    Factory factory(&encoding);
    Scene scene;
    RenderContext renderContext(&scene, &config);
    World world;
    bool ok = false;
    const UnicodeString &motionPath = config["dir.motion"] + "/" + config["file.motion"];
    if (vpvl2::extensions::icu::String::toBoolean(config["enable.opencl"])) {
        scene.setAccelerationType(Scene::kOpenCLAccelerationType1);
    }
    std::string data;
    int nmodels = vpvl2::extensions::icu::String::toInt(config["models/size"]);
    for (int i = 0; i < nmodels; i++) {
        std::ostringstream stream;
        stream << "models/" << (i + 1);
        const UnicodeString &prefix = UnicodeString::fromUTF8(stream.str()),
                &modelPath = config[prefix + "/path"];
        int indexOf = modelPath.lastIndexOf("/");
        String dir(modelPath.tempSubString(0, indexOf));
        if (renderContext.loadFile(modelPath, data)) {
            int flags = 0;
            IModel *model = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&renderContext, model, flags);
            model->setEdgeWidth(float(vpvl2::extensions::icu::String::toDouble(config[prefix + "/edge.width"])));
            if (engine->upload(&dir)) {
                if (String::toBoolean(config[prefix + "/enable.physics"]))
                    world.addModel(model);
                scene.addModel(model, engine);
                if (renderContext.loadFile(motionPath, data)) {
                    IMotion *motion = factory.createMotion(UICastData(data), data.size(), model, ok);
                    scene.addMotion(motion);
                }
            }
        }
    }
    int nassets = vpvl2::extensions::icu::String::toInt(config["assets/size"]);
    for (int i = 0; i < nassets; i++) {
        std::ostringstream stream;
        stream << "assets/" << (i + 1);
        const UnicodeString &prefix = UnicodeString::fromUTF8(stream.str()),
                &assetPath = config[prefix + "/path"];
        if (renderContext.loadFile(assetPath, data)) {
            int indexOf = assetPath.lastIndexOf("/");
            String dir(assetPath.tempSubString(0, indexOf));
            IModel *asset = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&renderContext, asset, 0);
            if (engine->upload(&dir)) {
                scene.addModel(asset, engine);
            }
        }
    }

    sf::Clock clock;
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    scene.seek(0, Scene::kUpdateAll);
    scene.update(Scene::kUpdateAll | Scene::kResetMotionState);

    int x = 0, y = 0;
    bool isPressed = false;
    sf::Event event;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
            else if (event.type == sf::Event::MouseMoved && isPressed) {
                ICamera *camera = scene.camera();
                int newX = event.mouseMove.x, newY = event.mouseMove.y;
                const Scalar factor(0.5);
                const Vector3 value((newY - y) * factor, (newX - x) * factor, 0);
                x = newX;
                y = newY;
                camera->setAngle(camera->angle() + value);
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                x = event.mouseButton.x;
                y = event.mouseButton.y;
                isPressed = true;
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                isPressed = false;
            }
            else if (event.type == sf::Event::MouseWheelMoved) {
                ICamera *camera = scene.camera();
                camera->setDistance(camera->distance() + event.mouseWheel.delta);
            }
            else if (event.type == sf::Event::Resized) {
                width = event.size.width;
                height = event.size.height;
            }
        }
        UIUpdateCamera(scene, width, height, renderContext);
        Scalar delta = clock.getElapsedTime().asMilliseconds() / 60.0;
        clock.restart();
        window.clear(sf::Color::Blue);
        UIDrawScreen(scene, width, height);
        window.display();
        scene.advance(delta, Scene::kUpdateAll);
        world.stepSimulation(delta);
        scene.update(Scene::kUpdateAll);
    }

    return EXIT_SUCCESS;
}

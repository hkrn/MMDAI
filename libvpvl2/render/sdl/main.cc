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
#include <vpvl2/extensions/sdl/Delegate.h>

/* internal headers for debug */
#include <assert.h> /* for libvpvl via vpvl2::pmd::Model */
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

namespace {

using namespace vpvl2;
using namespace vpvl2::extensions::sdl;

struct UIContext
{
    UIContext(Scene *scene, UIStringMap *config, Delegate *delegate)
        : sceneRef(scene),
          configRef(config),
      #if SDL_VERSION_ATLEAST(2, 0, 0)
          windowRef(0),
      #endif
          delegateRef(delegate),
          width(640),
          height(480),
          restarted(SDL_GetTicks()),
          current(restarted),
          currentFPS(0),
          active(true)
    {
    }
    void updateFPS() {
        current = SDL_GetTicks();
        if (current - restarted > 1000) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
#ifdef _MSC_VER
            _snprintf(title, sizeof(title), "libvpvl2 with SDL2 (FPS:%d)", currentFPS);
#else
            snprintf(title, sizeof(title), "libvpvl2 with SDL2 (FPS:%d)", currentFPS);
#endif
            SDL_SetWindowTitle(windowRef, title);
#else
#ifdef _MSC_VER
            _snprintf(title, sizeof(title), "libvpvl2 with SDL (FPS:%d)", currentFPS);
#else
            snprintf(title, sizeof(title), "libvpvl2 with SDL (FPS:%d)", currentFPS);
#endif
            SDL_WM_SetCaption(title, 0);
#endif
            restarted = current;
            currentFPS = 0;
        }
        currentFPS++;
    }

    const Scene *sceneRef;
    const UIStringMap *configRef;
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Window *windowRef;
#endif
    Delegate *delegateRef;
    size_t width;
    size_t height;
    Uint32 restarted;
    Uint32 current;
    int currentFPS;
    char title[32];
    bool active;
};

static void UIHandleKeyEvent(const SDL_KeyboardEvent &event, UIContext &context)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
    const SDL_Keysym &keysym = event.keysym;
#else
    const SDL_keysym &keysym = event.keysym;
#endif
    const Scalar degree(15.0);
    ICamera *camera = context.sceneRef->camera();
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
        context.active = false;
        break;
    default:
        break;
    }
}

static void UIHandleMouseMotion(const SDL_MouseMotionEvent &event, UIContext &context)
{
    if (event.state == SDL_PRESSED) {
        ICamera *camera = context.sceneRef->camera();
        const Scalar &factor = 0.5;
        camera->setAngle(camera->angle() + Vector3(event.yrel * factor, event.xrel * factor, 0));
    }
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static void UIHandleMouseWheel(const SDL_MouseWheelEvent &event, UIContext &context)
{
    ICamera *camera = context.sceneRef->camera();
    const Scalar &factor = 1.0;
    camera->setDistance(camera->distance() + event.y * factor);
}
#endif

static void UIUpdateCamera(UIContext &context)
{
    const ICamera *camera = context.sceneRef->camera();
    Scalar matrix[16];
    camera->modelViewTransform().getOpenGLMatrix(matrix);
    const float &aspect = context.width / float(context.height);
    const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
            &projection = glm::perspective(camera->fov(), aspect, camera->znear(), camera->zfar());
    context.delegateRef->setCameraMatrix(world, view, projection);
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
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_GL_SwapWindow(context.windowRef);
#else
    SDL_GL_SwapBuffers();
#endif
}

static void UIProceedEvents(UIContext &context)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_MOUSEMOTION:
            UIHandleMouseMotion(event.motion, context);
            break;
        case SDL_KEYDOWN:
            UIHandleKeyEvent(event.key, context);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 0)
        case SDL_MOUSEWHEEL:
            UIHandleMouseWheel(event.wheel, context);
            break;
#else
        case SDL_VIDEORESIZE:
            context.width = event.resize.w;
            context.height = event.resize.h;
            break;
#endif
        case SDL_QUIT:
            context.active = false;
            break;
        default:
            break;
        }
    }
    UIUpdateCamera(context);
}

} /* namespace anonymous */

int main(int /* argc */, char ** /* argv[] */)
{
    atexit(SDL_Quit);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    atexit(IMG_Quit);
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0) {
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
#if !SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_WM_SetCaption("libvpvl2 with SDL", 0);
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        std::cerr << "SDL_GetVideoInfo() failed: " << SDL_GetError() << std::endl;
        return -1;
    }
#endif

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
    bool enableSW = vpvl2::extensions::icu::String::toBoolean(config["enable.opengl.software"]);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, enableSW ? 0 : 1);
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_Window *window = SDL_CreateWindow("libvpvl2 with SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          width, height, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "SDL_CreateWindow(title, x, y, width, height, SDL_OPENGL) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_GLContext contextGL = SDL_GL_CreateContext(window);
    if (!contextGL) {
        std::cerr << "SDL_GL_CreateContext(window) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_DisableScreenSaver();
#else
    SDL_Surface *surface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, SDL_OPENGL);
    if (!surface) {
        std::cerr << "SDL_SetVideoMode(width, height, bpp, SDL_OPENGL) failed: " << SDL_GetError() << std::endl;
        return -1;
    }
#endif
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

    Encoding encoding;
    Factory factory(&encoding);
    Scene scene;
    Delegate delegate(&scene, &config);
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
        if (UILoadFile(modelPath, data)) {
            int flags = 0;
            IModel *model = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&delegate, model, flags);
            model->setEdgeWidth(float(vpvl2::extensions::icu::String::toDouble(config[prefix + "/edge.width"])));
            if (engine->upload(&dir)) {
                if (String::toBoolean(config[prefix + "/enable.physics"]))
                    world.addModel(model);
                scene.addModel(model, engine);
                if (UILoadFile(motionPath, data)) {
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
        if (UILoadFile(assetPath, data)) {
            int indexOf = assetPath.lastIndexOf("/");
            String dir(assetPath.tempSubString(0, indexOf));
            IModel *asset = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&delegate, asset, 0);
            if (engine->upload(&dir)) {
                scene.addModel(asset, engine);
            }
        }
    }

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glClearColor(0, 0, 1, 0);

    UIContext context(&scene, &config, &delegate);
#if SDL_VERSION_ATLEAST(2, 0, 0)
    context.windowRef = window;
#endif
    Uint32 prev = SDL_GetTicks();
    scene.seek(0, Scene::kUpdateAll);
    scene.update(Scene::kUpdateAll | Scene::kResetMotionState);
    while (context.active) {
        UIProceedEvents(context);
        UIDrawScreen(context);
        Uint32 current = SDL_GetTicks();
        Scalar delta = (current - prev) / 60.0;
        prev = current;
        scene.advance(delta, Scene::kUpdateAll);
        world.stepSimulation(delta);
        scene.update(Scene::kUpdateAll);
        context.updateFPS();
    }
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_EnableScreenSaver();
    SDL_GL_DeleteContext(contextGL);
    SDL_DestroyWindow(window);
#else
    SDL_FreeSurface(surface);
#endif

    return 0;
}

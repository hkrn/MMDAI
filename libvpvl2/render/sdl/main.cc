/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "../helper.h"
#include <vpvl2/extensions/sdl/RenderContext.h>

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
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::sdl;

struct UIContext
{
    UIContext(Scene *scene, UIStringMap *config, RenderContext *renderContextRef)
        : sceneRef(scene),
          configRef(config),
      #if SDL_VERSION_ATLEAST(2, 0, 0)
          windowRef(0),
      #endif
          renderContextRef(renderContextRef),
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
    RenderContext *renderContextRef;
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
    glm::vec2 size(context.width, context.height);
    context.renderContextRef->updateCameraMatrix(size);
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

    UIStringMap config;
    UILoadConfig("config.ini", config);
    size_t width = vpvl2::extensions::icu::String::toInt(config["window.width"], 640),
            height = vpvl2::extensions::icu::String::toInt(config["window.height"], 480);
    int redSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.red"], 8),
            greenSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.green"], 8),
            blueSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.blue"], 8),
            alphaSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.alpha"], 8),
            depthSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.depth"], 24),
            stencilSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.stencil"], 8),
            samplesSize = vpvl2::extensions::icu::String::toInt(config["opengl.size.samples"], 4);
    bool enableSW = vpvl2::extensions::icu::String::toBoolean(config["opengl.enable.software"]),
            enableAA = vpvl2::extensions::icu::String::toBoolean(config["opengl.enable.aa"]);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, redSize);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, greenSize);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, blueSize);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, alphaSize);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthSize);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilSize);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, enableAA ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, enableAA ? samplesSize : 0);
    if (enableSW)
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
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

#ifdef VPVL2_LINK_GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Cannot initialize GLEW: " << glewGetErrorString(err);
        return -1;
    }
#endif /* VPVL2_LINK_GLEW */

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

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glClearColor(0, 0, 1, 0);

    UIContext context(&scene, &config, &renderContext);
#if SDL_VERSION_ATLEAST(2, 0, 0)
    context.windowRef = window;
#endif
    Uint32 prev = SDL_GetTicks();
    scene.seek(0, Scene::kUpdateAll);
    scene.update(Scene::kUpdateAll | Scene::kResetMotionState);
    while (context.active) {
        UIProceedEvents(context);
        UIDrawScreen(*context.sceneRef, context.width, context.height);
        Uint32 current = SDL_GetTicks();
        Scalar delta = (current - prev) / 60.0;
        prev = current;
        scene.advance(delta, Scene::kUpdateAll);
        world.stepSimulation(delta);
        scene.update(Scene::kUpdateAll);
        context.updateFPS();
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_GL_SwapWindow(context.windowRef);
#else
    SDL_GL_SwapBuffers();
#endif
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

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
    UIContext(Scene *scene, StringMap *config, RenderContext *renderContextRef, const glm::vec2 &size)
        : sceneRef(scene),
          configRef(config),
      #if SDL_VERSION_ATLEAST(2, 0, 0)
          windowRef(0),
      #endif
          renderContextRef(renderContextRef),
          width(size.x),
          height(size.y),
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
    const StringMap *configRef;
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
#endif
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 0)
        case SDL_VIDEORESIZE:
            context.width = event.resize.w;
            context.height = event.resize.h;
            glViewport(0, 0, context.width, context.height);
            break;
#endif
        case SDL_QUIT:
            context.active = false;
            break;
        default:
            break;
        }
    }
}

} /* namespace anonymous */

int main(int /* argc */, char ** /* argv[] */)
{
    atexit(SDL_Quit);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
    atexit(IMG_Quit);
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0) {
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#if !SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_WM_SetCaption("libvpvl2 with SDL", 0);
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        std::cerr << "SDL_GetVideoInfo() failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#endif

    StringMap settings;
    UILoadSettings("config.ini", settings);
    size_t width = settings.value("window.width", 640),
            height = settings.value("window.height", 480);
    int redSize = settings.value("opengl.size.red", 8),
            greenSize = settings.value("opengl.size.green", 8),
            blueSize = settings.value("opengl.size.blue", 8),
            alphaSize = settings.value("opengl.size.alpha", 8),
            depthSize = settings.value("opengl.size.depth", 24),
            stencilSize = settings.value("opengl.size.stencil", 8),
            samplesSize = settings.value("opengl.size.samples", 4);
    bool enableSW = settings.value("opengl.enable.software", false),
            enableAA = settings.value("opengl.enable.aa", false);
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
        return EXIT_FAILURE;
    }
#endif
    GLenum err = 0;
    if (!Scene::initialize(&err)) {
        std::cerr << "Cannot initialize GLEW: " << err << std::endl;
        return EXIT_FAILURE;
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

    Encoding::Dictionary dictionary;
    dictionary.insert(IEncoding::kArm, new String(settings.value("encoding.constant.arm", "")));
    dictionary.insert(IEncoding::kAsterisk, new String(settings.value("encoding.constant.asterisk", "*")));
    dictionary.insert(IEncoding::kCenter, new String(settings.value("encoding.constant.center", "")));
    dictionary.insert(IEncoding::kElbow, new String(settings.value("encoding.constant.elbow", "")));
    dictionary.insert(IEncoding::kFinger, new String(settings.value("encoding.constant.finger", "")));
    dictionary.insert(IEncoding::kLeft, new String(settings.value("encoding.constant.left", "")));
    dictionary.insert(IEncoding::kOpacityMorphAsset, new String(settings.value("encoding.constant.opacityMorphAsset", "")));
    dictionary.insert(IEncoding::kRight, new String(settings.value("encoding.constant.right", "")));
    dictionary.insert(IEncoding::kRootBoneAsset, new String(settings.value("encoding.constant.rootBoneAsset", "")));
    dictionary.insert(IEncoding::kScaleBoneAsset, new String(settings.value("encoding.constant.scaleBoneAsset", "")));
    dictionary.insert(IEncoding::kSPAExtension, new String(settings.value("encoding.constant.spa", ".spa")));
    dictionary.insert(IEncoding::kSPHExtension, new String(settings.value("encoding.constant.sph", ".sph")));
    dictionary.insert(IEncoding::kWrist, new String(settings.value("encoding.constant.wrist", "")));
    WorldSmartPtr world(new World());
    EncodingSmartPtr encoding(new Encoding(&dictionary));
    FactorySmartPtr factory(new Factory(encoding.get()));
    SceneSmartPtr scene(new Scene(true));
    RenderContext renderContext(scene.get(), &settings);
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

    int nmodels = settings.value("models/size", 0);
    for (int i = 0; i < nmodels; i++) {
        std::ostringstream stream;
        stream << "models/" << (i + 1);
        const UnicodeString &prefix = UnicodeString::fromUTF8(stream.str()),
                &modelPath = settings.value(prefix + "/path", UnicodeString());
        int indexOf = modelPath.lastIndexOf("/");
        String dir(modelPath.tempSubString(0, indexOf));
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
                engine->setEffect(IEffect::kAutoDetection, effectRef, &dir);
            }
            if (engine->upload(&dir)) {
                renderContext.parseOffscreenSemantic(effectRef, &dir);
                if (settings.value(prefix + "/enable.physics", true))
                    world->addModel(model.get());
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

    UIContext context(scene.get(), &settings, &renderContext, glm::vec2(width, height));
#if SDL_VERSION_ATLEAST(2, 0, 0)
    context.windowRef = window;
#endif
    Uint32 prev = SDL_GetTicks();
    scene->seek(0, Scene::kUpdateAll);
    scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    while (context.active) {
        UIProceedEvents(context);
        renderContext.renderShadowMap();
        renderContext.renderOffscreen();
        renderContext.updateCameraMatrices(glm::vec2(context.width, context.height));
        UIDrawScreen(*context.sceneRef, context.width, context.height);
        Uint32 current = SDL_GetTicks();
        Scalar delta = (current - prev) / 60.0;
        prev = current;
        scene->advance(delta, Scene::kUpdateAll);
        world->stepSimulation(delta);
        scene->update(Scene::kUpdateAll);
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
    dictionary.releaseAll();
    /* explicitly release World instance first to ensure release btRigidBody */
    world.release();

    return EXIT_SUCCESS;
}

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
#include <vpvl2/extensions/egl/RenderContext.h>

#include <GL/glx.h>

using namespace vpvl2::extensions::egl;

static bool UICreateEGLWindow(size_t width, size_t height, const char *title,
                              EGLNativeDisplayType &display, EGLNativeWindowType &window)
{
    display = XOpenDisplay(NULL);
    if (!display) {
        return false;
    }
    Window root = DefaultRootWindow(display);
    XSetWindowAttributes attribs = { 0 };
    attribs.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
    window = XCreateWindow(display, root, 0, 0, width, height, 0,
                           CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &attribs);
    XSetWindowAttributes  newAttribs = { 0 };
    newAttribs.override_redirect = FALSE;
    XChangeWindowAttributes(display, window, CWOverrideRedirect, &newAttribs);
    XWMHints hints = { 0 };
    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(display, window, &hints);
    XMapWindow(display, window);
    XStoreName(display, window, title);
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", FALSE);
    XEvent event = { 0 };
    event.type                 = ClientMessage;
    event.xclient.window       = window;
    event.xclient.message_type = wm_state;
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = 1;
    event.xclient.data.l[1]    = FALSE;
    XSendEvent(display, DefaultRootWindow(display), FALSE, SubstructureNotifyMask, &event);
    return true;
}

static void UITerminateEGLSession(EGLDisplay display, EGLSurface surface, EGLContext context)
{
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

int main(int /* argc */, char ** /* argv */)
{
    UIStringMap settings;
    UILoadSettings("config.ini", settings);
    if (!eglBindAPI(EGL_OPENGL_API)) {
        std::cerr << "Cannot bind OpenGL API to EGL session: " << eglGetError() << std::endl;
        return EXIT_FAILURE;
    }

    size_t width = vpvl2::extensions::icu::String::toInt(settings["window.width"], 640),
            height = vpvl2::extensions::icu::String::toInt(settings["window.height"], 480);
    int redSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.red"], 8),
            greenSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.green"], 8),
            blueSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.blue"], 8),
            alphaSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.alpha"], 8),
            depthSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.depth"], 24),
            stencilSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.stencil"], 8),
            samplesSize = vpvl2::extensions::icu::String::toInt(settings["opengl.size.samples"], 0);

    EGLNativeDisplayType nativeDisplay;
    EGLNativeWindowType nativeWindow;
    if (!UICreateEGLWindow(width, height, "MMDAI2 for EGL", nativeDisplay, nativeWindow)) {
        return EXIT_FAILURE;
    }
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        UITerminateEGLSession(display, 0, 0);
        std::cerr << "Cannot initialize EGL session: " << eglGetError() << std::endl;
        return EXIT_FAILURE;
    }

    EGLint attrs[] = {
        EGL_LEVEL, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, redSize,
        EGL_GREEN_SIZE, greenSize,
        EGL_BLUE_SIZE, blueSize,
        EGL_ALPHA_SIZE, alphaSize,
        EGL_DEPTH_SIZE, depthSize,
        EGL_STENCIL_SIZE, stencilSize,
        EGL_SAMPLE_BUFFERS, samplesSize,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    EGLConfig configs;
    EGLint nconfigs;
    if (!eglChooseConfig(display, attrs, &configs, 1, &nconfigs)) {
        std::cerr << "Cannot choose EGL configuration: " << eglGetError() << std::endl;
        UITerminateEGLSession(display, 0, 0);
        return EXIT_FAILURE;
    }
    EGLint surfaceAttribs[] = {
        EGL_NONE
    };
    EGLSurface surface = eglCreateWindowSurface(display, configs, nativeWindow, surfaceAttribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Cannot create EGL surface: " << eglGetError() << std::endl;
        UITerminateEGLSession(display, surface, 0);
        return EXIT_FAILURE;
    }
    EGLint contextAttribs[] = {
        EGL_NONE
    };
    EGLContext context = eglCreateContext(display, configs, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Cannot create EGL context: " << eglGetError() << std::endl;
        UITerminateEGLSession(display, surface, context);
        return EXIT_FAILURE;
    }
    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "Cannot make OpenGL context current: " << eglGetError() << std::endl;
        UITerminateEGLSession(display, surface, context);
        return EXIT_FAILURE;
    }

    std::cerr << "EGL_VENDOR: " << eglQueryString(display, EGL_VENDOR) << std::endl;
    std::cerr << "EGL_VERSION: " << eglQueryString(display, EGL_VERSION) << std::endl;
    std::cerr << "EGL_EXTENSIONS: " << eglQueryString(display, EGL_EXTENSIONS) << std::endl;
    std::cerr << "EGL_CLIENT_APIS: " << eglQueryString(display, EGL_CLIENT_APIS) << std::endl;

    std::cerr << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cerr << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    std::cerr << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;

    GLenum err = 0;
    if (!Scene::initialize(&err)) {
        UITerminateEGLSession(display, surface, context);
        std::cerr << "Cannot initialize GLEW: " << err << std::endl;
        return EXIT_FAILURE;
    }

    Encoding encoding;
    Factory factory(&encoding);
    Scene scene;
    RenderContext renderContext(&scene, &settings);
    World world;
    bool ok = false;
    const UnicodeString &motionPath = settings["dir.motion"] + "/" + settings["file.motion"];
    if (vpvl2::extensions::icu::String::toBoolean(settings["enable.opencl"])) {
        scene.setAccelerationType(Scene::kOpenCLAccelerationType1);
    }
    std::string data;
    int nmodels = vpvl2::extensions::icu::String::toInt(settings["models/size"]);
    for (int i = 0; i < nmodels; i++) {
        std::ostringstream stream;
        stream << "models/" << (i + 1);
        const UnicodeString &prefix = UnicodeString::fromUTF8(stream.str()),
                &modelPath = settings[prefix + "/path"];
        int indexOf = modelPath.lastIndexOf("/");
        String dir(modelPath.tempSubString(0, indexOf));
        if (renderContext.loadFile(modelPath, data)) {
            int flags = 0;
            IModel *model = factory.createModel(UICastData(data), data.size(), ok);
            IRenderEngine *engine = scene.createRenderEngine(&renderContext, model, flags);
            model->setEdgeWidth(float(vpvl2::extensions::icu::String::toDouble(settings[prefix + "/edge.width"])));
            if (engine->upload(&dir)) {
                if (String::toBoolean(settings[prefix + "/enable.physics"]))
                    world.addModel(model);
                scene.addModel(model, engine);
                if (renderContext.loadFile(motionPath, data)) {
                    IMotion *motion = factory.createMotion(UICastData(data), data.size(), model, ok);
                    scene.addMotion(motion);
                }
            }
        }
    }

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    scene.seek(0, Scene::kUpdateAll);
    scene.update(Scene::kUpdateAll | Scene::kResetMotionState);

    while (true) {
        scene.advance(0, Scene::kUpdateAll);
        world.stepSimulation(0);
        scene.update(Scene::kUpdateAll);
        eglSwapBuffers(display, surface);
    }
    UITerminateEGLSession(display, surface, context);

    return EXIT_SUCCESS;
}

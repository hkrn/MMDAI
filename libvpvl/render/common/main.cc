/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>

#include <SDL.h>
#include <SDL_image.h>

#include <sys/stat.h>
#include <errno.h>

#define VPVL_SDL_LOAD_ASSET 0

namespace internal
{

static const int kWidth = 800;
static const int kHeight = 600;
static const int kFPS = 60;

static const std::string kSystemDir = "render/res/system";
static const std::string kModelDir = "render/res/lat";
static const std::string kStageDir = "render/res/stage";
static const std::string kMotion = "test/res/motion.vmd";
static const std::string kCamera = "test/res/camera.vmd";
static const std::string kModelName = "normal.pmd";
static const std::string kStageName = "stage.x";

static const std::string concatPath(const std::string &dir, const std::string &name) {
    return dir + "/" + name;
}

static void slurpFile(const std::string &path, uint8_t *&data, size_t &size) {
    FILE *fp = fopen(path.c_str(), "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new uint8_t[size];
        fread(data, size, 1, fp);
        fclose(fp);
    }
    else {
        data = 0;
        size = 0;
        fprintf(stderr, "Failed loading %s: %s\n", path.c_str(), strerror(errno));
    }
}

class Delegate : public vpvl::gl::Delegate
{
public:
    Delegate(const std::string &system) : m_system(system) {
    }
    ~Delegate() {
    }

    bool loadTexture(const std::string &path, GLuint &textureID) {
        static const GLfloat priority = 1.0f;
        SDL_Surface *surface = IMG_Load(path.c_str());
        if (surface) {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            GLenum format, internal;
            if (surface->format->BitsPerPixel == 32) {
                format = surface->format->Rmask & 0xff ? GL_RGBA : GL_BGRA;
                internal = GL_RGBA8;
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
            else if (surface->format->BitsPerPixel == 24) {
                format = surface->format->Rmask & 0xff ? GL_RGB : GL_BGR;
                internal = GL_RGB8;
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            }
            else {
                fprintf(stderr, "Unknown image format: %s\n", path.c_str());
                SDL_FreeSurface(surface);
                return false;
            }
            SDL_LockSurface(surface);
            glTexImage2D(GL_TEXTURE_2D, 0, internal, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
            SDL_UnlockSurface(surface);
            SDL_FreeSurface(surface);
            glPrioritizeTextures(1, &textureID, &priority);
            glBindTexture(GL_TEXTURE_2D, 0);
            return true;
        }
        else {
            fprintf(stderr, "Failed loading %s: %s\n", path.c_str(), IMG_GetError());
            return false;
        }
    }
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        struct stat sb;
        std::string path = dir + "/" + name;
        if (!(stat(path.c_str(), &sb) != -1 && S_ISREG(sb.st_mode))) {
            path = m_system + "/" + name;
            if (!(stat(path.c_str(), &sb) != -1 && S_ISREG(sb.st_mode))) {
                fprintf(stderr, "%s is not found, skipped...\n", path.c_str());
                return false;
            }
        }
        return loadTexture(path, textureID);
    }
    const std::string toUnicode(const uint8_t *value) {
        return reinterpret_cast<const char *>(value);
    }

private:
    std::string m_system;
};

}

static bool InitializeSurface(SDL_Surface *&surface, int width, int height)
{
    SDL_WM_SetCaption("libvpvl render testing program", NULL);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        fprintf(stderr, "Unable to get video info: %s\n", SDL_GetError());
        return false;
    }
    int bpp = info->vfmt->BitsPerPixel;
    if (SDL_VideoModeOK(width, height, bpp, SDL_OPENGL)) {
        if ((surface = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL)) == NULL) {
            fprintf(stderr, "Unable to init surface: %s\n", SDL_GetError());
            return false;
        }
    }
    else {
        fprintf(stderr, "It seems OpenGL is not supported\n");
        return false;
    }
    return true;
}

static void DrawSurface(vpvl::gl::Renderer *renderer)
{
    renderer->initializeSurface();
    renderer->drawSurface();
    SDL_GL_SwapBuffers();
}

static bool PollEvents()
{
    SDL_Event event;
    SDLKey key;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return true;
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE || key == SDLK_q)
                return true;
            break;
        default:
            break;
        }
    }
    return false;
}

static Uint32 UpdateTimer(Uint32 internal, void *data)
{
    vpvl::gl::Renderer *renderer = static_cast<vpvl::gl::Renderer *>(data);
    vpvl::Scene *scene = renderer->scene();
    scene->updateModelView(0);
    scene->updateProjection(0);
    scene->update(0.5);
    return internal;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);
    if (IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) < 0) {
        fprintf(stderr, "Unable to init SDL_image: %s\n", IMG_GetError());
        return -1;
    }
    atexit(IMG_Quit);

    SDL_Surface *surface;
    if (!InitializeSurface(surface, internal::kWidth, internal::kHeight)) {
        return -1;
    }

    GLenum err;
    if (!vpvl::gl::Renderer::initializeGLEW(err)) {
        fprintf(stderr, "Unable to init GLEW: %s\n", glewGetErrorString(err));
        return -1;
    }

    const float dist = 400.0f;
    btDefaultCollisionConfiguration config;
    btCollisionDispatcher dispatcher(&config);
    btAxisSweep3 broadphase(btVector3(-dist, -dist, -dist), btVector3(dist, dist, dist), 1024);
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &config);
    world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
    world.getSolverInfo().m_numIterations = static_cast<int>(10.0f * 60.0f / internal::kFPS);

    internal::Delegate delegate(internal::kSystemDir);
    vpvl::gl::Renderer renderer(&delegate, internal::kWidth, internal::kHeight, internal::kFPS);

    uint8_t *modelData = 0;
    uint8_t *motionData = 0;
    uint8_t *cameraData = 0;
    uint8_t *stageData = 0;
    size_t size = 0;

    internal::slurpFile(internal::concatPath(internal::kModelDir, internal::kModelName), modelData, size);
    vpvl::PMDModel model;
    if (!model.load(modelData, size)) {
        fprintf(stderr, "Failed parsing the model\n");
        delete[] modelData;
        return -1;
    }
    renderer.loadModel(&model, internal::kModelDir);

#if VPVL_SDL_LOAD_ASSET
    internal::slurpFile(internal::concatPath(internal::kStageDir, internal::kStageName), stageData, size);
    vpvl::XModel stage;
    if (!stage.load(stageData, size)) {
        fprintf(stderr, "Failed parsing the stage\n");
        delete[] modelData;
        delete[] stageData;
        return -1;
    }
    renderer.loadAsset(&stage, internal::kStageDir);
#endif

    vpvl::Scene *scene = renderer.scene();
    renderer.setLighting();
    scene->addModel(&model);
    scene->setWorld(&world);

    internal::slurpFile(internal::kMotion, motionData, size);
    vpvl::VMDMotion motion;
    if (!motion.load(motionData, size)) {
        fprintf(stderr, "Failed parsing the model motion\n");
        delete[] modelData;
        delete[] stageData;
        delete[] motionData;
        return -1;
    }
    model.addMotion(&motion);

    internal::slurpFile(internal::kCamera, cameraData, size);
    vpvl::VMDMotion camera;
    if (!camera.load(cameraData, size)) {
        fprintf(stderr, "Failed parsing the camera motion\n");
        delete[] modelData;
        delete[] stageData;
        delete[] motionData;
        delete[] cameraData;
        return -1;
    }
    //scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
    scene->setCameraMotion(&camera);

    uint32_t interval = static_cast<uint32_t>(1000.0f / internal::kFPS);
    SDL_TimerID timerID = SDL_AddTimer(interval, UpdateTimer, &renderer);
    while (true) {
        if (PollEvents())
            break;
        DrawSurface(&renderer);
    }
    SDL_RemoveTimer(timerID);

    renderer.unloadModel(&model);
#if VPVL_SDL_LOAD_ASSET
    //renderer.unloadAsset(&stage);
#endif
    SDL_FreeSurface(surface);

    delete[] stageData;
    delete[] motionData;
    delete[] modelData;
    delete[] cameraData;

    return 0;
}

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

static Uint32 UpdateTimer(Uint32 internal, void *data)
{
    vpvl::gl::Renderer *renderer = static_cast<vpvl::gl::Renderer *>(data);
    vpvl::Scene *scene = renderer->scene();
    scene->updateModelView(0);
    scene->updateProjection(0);
    scene->update(0.5);
    return internal;
}

class UI
{
public:
    UI(int argc, char **argv)
        : m_surface(0),
          m_world(0),
          m_dispatcher(&m_config),
          m_broadphase(btVector3(-400.0f, -400.0f, -400.0f), btVector3(400.0f, 400.0f, 400.0f), 1024),
          m_delegate(internal::kSystemDir),
          m_renderer(&m_delegate, internal::kWidth, internal::kHeight, internal::kFPS),
          m_model(0),
          m_modelData(0),
          m_motionData(0),
          m_cameraData(0),
          m_stageData(0),
          m_argc(argc),
          m_argv(argv)
    {
        m_world = new btDiscreteDynamicsWorld(&m_dispatcher, &m_broadphase, &m_solver, &m_config);
        m_world->setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        m_world->getSolverInfo().m_numIterations = static_cast<int>(10.0f * 60.0f / internal::kFPS);
    }
    ~UI() {
        m_renderer.unloadModel(m_model);
#if VPVL_SDL_LOAD_ASSET
        // m_renderer.unloadAsset(&m_stage);
#endif
        delete m_model;
        delete m_world;
        delete[] m_modelData;
        delete[] m_motionData;
        delete[] m_cameraData;
        delete[] m_stageData;
        SDL_FreeSurface(m_surface);
        m_surface = 0;
    }

    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
            fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
            return false;
        }
        atexit(SDL_Quit);
        if (IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) < 0) {
            fprintf(stderr, "Unable to init SDL_image: %s\n", IMG_GetError());
            return false;
        }
        atexit(IMG_Quit);

        if (!initializeSurface(internal::kWidth, internal::kHeight))
            return false;

        GLenum err;
        if (!vpvl::gl::Renderer::initializeGLEW(err)) {
            fprintf(stderr, "Unable to init GLEW: %s\n", glewGetErrorString(err));
            return false;
        }

        if (!loadScene())
            return false;

        return true;
    }
    int execute() {
        uint32_t interval = static_cast<uint32_t>(1000.0f / internal::kFPS);
        SDL_TimerID timerID = SDL_AddTimer(interval, UpdateTimer, &m_renderer);
        while (true) {
            if (pollEvent())
                return 0;
            draw();
            SDL_GL_SwapBuffers();
        }
        SDL_RemoveTimer(timerID);
        sleep(1); /* wait for ensuring SDL's timer thread is killed */
        return 0;
    }

protected:
    virtual void draw() {
        m_renderer.initializeSurface();
        m_renderer.drawSurface();
    }

private:
    bool initializeSurface(int width, int height) {
        SDL_WM_SetCaption("libvpvl + SDL", NULL);
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
            if ((m_surface = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL)) == NULL) {
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
    bool loadScene() {
        size_t size = 0;
        internal::slurpFile(internal::concatPath(internal::kModelDir, internal::kModelName), m_modelData, size);
        m_model = new vpvl::PMDModel();
        if (!m_model->load(m_modelData, size)) {
            fprintf(stderr, "Failed parsing the model\n");
            return false;
        }
        m_renderer.loadModel(m_model, internal::kModelDir);

#if VPVL_SDL_LOAD_ASSET
        internal::slurpFile(internal::concatPath(internal::kStageDir, internal::kStageName), m_stageData, size);
        if (!m_stage.load(m_stageData, size)) {
            fprintf(stderr, "Failed parsing the stage\n");
            return -1;
        }
        m_renderer.loadAsset(&m_stage, internal::kStageDir);
#endif

        vpvl::Scene *scene = m_renderer.scene();
        m_renderer.setLighting();
        scene->addModel(m_model);

        internal::slurpFile(internal::kMotion, m_motionData, size);
        if (!m_motion.load(m_motionData, size)) {
            fprintf(stderr, "Failed parsing the model motion\n");
            return false;
        }
        m_model->addMotion(&m_motion);

        internal::slurpFile(internal::kCamera, m_cameraData, size);
        if (!m_camera.load(m_cameraData, size)) {
            fprintf(stderr, "Failed parsing the camera motion\n");
            return false;
        }
        //scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
        scene->setCameraMotion(&m_camera);
        scene->setWorld(m_world);

        return true;
    }
    bool pollEvent() {
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

    SDL_Surface *m_surface;
    btDiscreteDynamicsWorld *m_world;
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    internal::Delegate m_delegate;
    vpvl::gl::Renderer m_renderer;
    vpvl::PMDModel *m_model; /* for destruction order problem with btDiscreteDynamicsWorld */
    vpvl::VMDMotion m_motion;
    vpvl::VMDMotion m_camera;
    vpvl::XModel m_stage;
    uint8_t *m_modelData;
    uint8_t *m_motionData;
    uint8_t *m_cameraData;
    uint8_t *m_stageData;
    int m_argc;
    char **m_argv;
};

int main(int argc, char *argv[])
{
    UI ui(argc, argv);
    if (!ui.initialize())
        return -1;
    return ui.execute();
}

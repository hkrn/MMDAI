#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include <SDL.h>
#include <SDL_image.h>

#include <vpvl/vpvl.h>
#include <vpvl/internal/PMDModel.h>

static const int g_width = 800;
static const int g_height = 600;

static bool InitializeSurface(SDL_Surface *&surface, int width, int height)
{
    SDL_WM_SetCaption("libvpvl render testing program", NULL);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        fprintf(stderr, "Unable to get video info: %s", SDL_GetError());
        return false;
    }
    if ((surface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, SDL_OPENGL)) == NULL) {
        fprintf(stderr, "Unable to init surface: %s", SDL_GetError());
        return false;
    }
    glClearStencil(0);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    return true;
}

static void SetLighting()
{
    btVector3 color(0.0f, 0.0f, 0.0f);
    btVector4 direction(0.0f, 0.0f, 0.0f, 0.0f);
    btScalar diffuseValue = 0, ambientValue = 0, specularValue = 0, lightIntensity = 0;

    // use MMD like cartoon
    diffuseValue = 0.2f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = lightIntensity;

    btVector3 diffuse = color * diffuseValue;
    btVector3 ambient = color * ambientValue;
    btVector3 specular = color * specularValue;
    diffuse.setW(1.0f);
    ambient.setW(1.0f);
    specular.setW(1.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, static_cast<const btScalar *>(direction));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, static_cast<const btScalar *>(diffuse));
    glLightfv(GL_LIGHT0, GL_AMBIENT, static_cast<const btScalar *>(ambient));
    glLightfv(GL_LIGHT0, GL_SPECULAR, static_cast<const btScalar *>(specular));
}

static void DrawSurface(vpvl::PMDModel &model, int width, int height)
{
    (void) model;
    double ratio = static_cast<double>(width) / height;
    // initialize
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // initialize matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, ratio, 1.0, 1024.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_EQUAL, GL_KEEP, GL_INCR);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_LIGHTING);
    // render model and shadow
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

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

    SDL_Surface *surface;
    vpvl::PMDModel model("", 0);
    if (!InitializeSurface(surface, g_width, g_height))
        return -1;

    model.parse();
    SetLighting();

    while (true) {
        if (PollEvents())
            break;
        DrawSurface(model, g_width, g_height);
    }

    SDL_FreeSurface(surface);

    return 0;
}

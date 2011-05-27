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

struct vpvl::MaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
};

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

static void LoadTexture(const char *path, GLuint &texture)
{
    SDL_Surface *surface = IMG_Load(path);
    if (surface) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GLenum format;
        if (surface->format->BitsPerPixel == 32) {
            format = GL_RGBA;
        }
        else if (surface->format->BitsPerPixel == 24) {
            format = GL_RGB;
        }
        else {
            printf("unknown image format: %s", path);
            SDL_FreeSurface(surface);
            return;
        }
        SDL_LockSurface(surface);
        glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
        printf("failed loading %s: %s", path, IMG_GetError());
    }
}

static void LoadModelTextures(const vpvl::PMDModel &model)
{
    const vpvl::MaterialList materials = model.materials();
    uint32_t nMaterials = materials.size();
    for (uint32_t i = 0; i <nMaterials; i++) {
        vpvl::Material *material = materials[i];
        const char *primary = material->primaryTextureName();
        const char *second = material->secondTextureName();
        GLuint textureID = 0;
        vpvl::MaterialPrivate *data = new vpvl::MaterialPrivate;
        data->primaryTextureID = 0;
        data->secondTextureID = 0;
        if (*primary) {
            char path[256];
            snprintf(path, sizeof(path), "render/res/%s", primary);
            LoadTexture(path, textureID);
            data->primaryTextureID = textureID;
        }
        if (*second) {
            char path[256];
            snprintf(path, sizeof(path), "render/res/%s", second);
            LoadTexture(path, textureID);
            data->secondTextureID = textureID;
        }
        material->setPrivateData(data);
    }
}

static void UnloadModelTextures(const vpvl::PMDModel &model)
{
    const vpvl::MaterialList materials = model.materials();
    uint32_t nMaterials = materials.size();
    for (uint32_t i = 0; i < nMaterials; i++) {
        vpvl::MaterialPrivate *data = materials[i]->privateData();
        glDeleteTextures(1, &data->primaryTextureID);
        glDeleteTextures(1, &data->secondTextureID);
        delete data;
    }
}

static void SetLighting(vpvl::PMDModel &model)
{
    btVector3 color(1.0f, 1.0f, 1.0f), direction(0.5f, 1.0f, 0.5f);
    btScalar diffuseValue, ambientValue, specularValue, lightIntensity = 0.6;

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
    model.setLightDirection(direction);

    glLightfv(GL_LIGHT0, GL_POSITION, static_cast<const btScalar *>(direction));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, static_cast<const btScalar *>(diffuse));
    glLightfv(GL_LIGHT0, GL_AMBIENT, static_cast<const btScalar *>(ambient));
    glLightfv(GL_LIGHT0, GL_SPECULAR, static_cast<const btScalar *>(specular));
}

static void DrawModel(const vpvl::PMDModel &model)
{
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, model.stride(), model.verticesPointer());
    glNormalPointer(GL_FLOAT, model.stride(), model.normalsPointer());
    glTexCoordPointer(2, GL_FLOAT, model.stride(), model.textureCoordsPointer());
    bool enableToon = true;
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glClientActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        // shadow map
        if (false)
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        else
            glTexCoordPointer(2, GL_FLOAT, 0, model.toonTextureCoordsPointer());
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    bool hasSPH = false, hasSPA = false;
    const vpvl::MaterialList materials = model.materials();
    uint32_t nMaterials = materials.size();
    uint16_t *indicesPtr = const_cast<uint16_t *>(model.indices());
    for (uint32_t i = 0; i <nMaterials; i++) {
        vpvl::Material *material = materials[i];
        vpvl::MaterialPrivate *data = material->privateData();
        // first sphere map
        if (!hasSPH && (material->isSpherePrimary() || material->isSphereSecond())) {
            glEnable(GL_TEXTURE_2D);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glDisable(GL_TEXTURE_2D);
            hasSPH = true;
        }
        // second sphere map
        if (!hasSPA && (material->isSphereAuxPrimary() || material->isSphereAuxSecond())) {
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            hasSPA = true;
        }
        // toon
        if (enableToon) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(material->averageColor()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
        }
        else {
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(material->diffuse()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, static_cast<const GLfloat *>(material->ambient()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shiness());
        material->alpha() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        // if toon or second sphere
        if (enableToon || hasSPA)
            glActiveTexture(GL_TEXTURE0);
        // has texture
        if (data->primaryTextureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, data->primaryTextureID);
            // is sphere map
            if (material->isSpherePrimary() || material->isSphereAuxPrimary()) {
                // is second sphere map
                if (material->isSphereAuxPrimary())
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
            }
            else {
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
            }
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
        // toon
        if (enableToon) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        // second sphere
        if (data->secondTextureID > 0) {
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            // is second sphere
            if (material->isSphereAuxSecond())
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
            else
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, data->secondTextureID);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
        }
        else {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_2D);
        }
        // draw
        uint32_t nIndices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, indicesPtr);
        indicesPtr += nIndices;
        // is aux sphere map
        if (material->isSphereAuxPrimary()) {
            if (enableToon)
                glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // toon
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSPH) {
            glActiveTexture(GL_TEXTURE0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // second sphere map
        if (hasSPA) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glActiveTexture(GL_TEXTURE0);
    }
    else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSPH) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        // second sphere map
        if (hasSPA) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glActiveTexture(GL_TEXTURE0);
        }
    }
    // first or second sphere map
    if (hasSPH || hasSPA) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        // second sphere map
        if (hasSPA) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_2D);
        }
    }
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
}

static void DrawSurface(vpvl::PMDModel &model, int width, int height)
{
    float matrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, -13, -100, 1
    };
    model.updateRootBone();
    model.updateMotion();
    model.updateSkins();
    double ratio = static_cast<double>(width) / height;
    // initialize
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // initialize matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(16.0, ratio, 0.5, 8000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(matrix);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // render model and shadow
    DrawModel(model);
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

static void FileSlurp(const char *path, char *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new char[size];
    fread(data, size, 1, fp);
    fclose(fp);
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
    char *data;
    size_t size;
    FileSlurp("render/res/miku.pmd", data, size);
    vpvl::PMDModel model(data, size);
    if (!InitializeSurface(surface, g_width, g_height)) {
        delete data;
        return -1;
    }

    if (!model.parse()) {
        delete data;
        return -1;
    }
    SetLighting(model);
    LoadModelTextures(model);

    while (true) {
        if (PollEvents())
            break;
        DrawSurface(model, g_width, g_height);
    }

    UnloadModelTextures(model);
    SDL_FreeSurface(surface);
    delete data;

    return 0;
}

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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <SDL.h>
#include <SDL_image.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vpvl/vpvl.h>

#ifdef __GNUC__
#define WILL_BE_UNUSED __attribute__ ((unused))
#else
#define WILL_BE_UNUSED
#endif

enum VertexBufferObject {
    kModelVertices,
    kModelNormals,
    kModelColors,
    kModelTexCoords,
    kModelToonTexCoords,
    kEdgeVertices,
    kEdgeIndices,
    kShadowIndices,
    kVertexBufferObjectMax
};

struct PMDModelMaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
    GLuint vertexBufferObject;
};

struct vpvl::PMDModelUserData {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    PMDModelMaterialPrivate *materials;
};

#define OLD_STYLE_XMESH_RENDER 1
#if OLD_STYLE_XMESH_RENDER
struct vpvl::XModelUserData {
    GLuint listID;
    btHashMap<btHashString, GLuint> textures;
};
#else
struct XModelMaterialPrivate {
    GLuint vertexBufferObject;
};

struct vpvl::XModelUserData {
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    XModelMaterialPrivate *materials;
};
#endif

static const int g_width = 800;
static const int g_height = 600;
static const int g_FPS = 60;

#define CONCAT_PATH(path) "" #path

static const uint8_t g_sysdir[] = CONCAT_PATH(render/res/system);
static const uint8_t g_modeldir[] = CONCAT_PATH(render/res/lat);
static const uint8_t g_stagedir[] = CONCAT_PATH(render/res/stage);
static const uint8_t g_motion[] = CONCAT_PATH(test/res/motion.vmd);
static const uint8_t g_camera[] = CONCAT_PATH(test/res/camera.vmd);
static const uint8_t g_modelname[] = "normal.pmd";
static const uint8_t g_stagename[] = "stage.x";

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
    Uint8 bpp = info->vfmt->BitsPerPixel;
    if (SDL_VideoModeOK(width, height, bpp, SDL_OPENGL)) {
        if ((surface = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL)) == NULL) {
            fprintf(stderr, "Unable to init surface: %s\n", SDL_GetError());
            return false;
        }
    }
    else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
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
    }
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    return true;
}

static void SetLighting(vpvl::Scene &scene)
{
    btVector4 color(1.0f, 1.0f, 1.0f, 1.0f), direction(0.5f, 1.0f, 0.5f, 0.0f);
    btScalar diffuseValue, ambientValue, specularValue, lightIntensity = 0.6;

    // use MMD like cartoon
#if 0
    diffuseValue = 0.2f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = 0.4f;
#else
    diffuseValue = 0.0f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = lightIntensity;
#endif

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
    scene.setLight(color, direction);
}

static bool LoadTexture(const uint8_t *path, GLuint &textureID)
{
    static const GLfloat priority = 1.0f;
    SDL_Surface *surface = IMG_Load(reinterpret_cast<const char *>(path));
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
            fprintf(stderr, "Unknown image format: %s\n", path);
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
        fprintf(stderr, "Failed loading %s: %s\n", path, IMG_GetError());
        return false;
    }
}

static bool LoadToonTexture(const uint8_t *system, const uint8_t *dir, const uint8_t *name, GLuint &textureID)
{
    char path[256];
    struct stat sb;
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    if (!(stat(path, &sb) != -1 && S_ISREG(sb.st_mode))) {
        snprintf(path, sizeof(path), "%s/%s", system, name);
        if (!(stat(path, &sb) != -1 && S_ISREG(sb.st_mode))) {
            fprintf(stderr, "%s is not found, skipped...\n", path);
            return false;
        }
    }
    //fprintf(stderr, "%s\n", path);
    return LoadTexture(reinterpret_cast<const uint8_t *>(path), textureID);
}

static void LoadModel(vpvl::PMDModel *model, const uint8_t *system, const uint8_t *dir)
{
    const vpvl::MaterialList materials = model->materials();
    const uint32_t nMaterials = materials.size();
    char path[256];
    GLuint textureID = 0;
    vpvl::PMDModelUserData *userData = new vpvl::PMDModelUserData;
    PMDModelMaterialPrivate *materialPrivates = new PMDModelMaterialPrivate[nMaterials];
    uint16_t *indicesPtr = const_cast<uint16_t *>(model->indicesPointer());
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const uint8_t *primary = material->primaryTextureName();
        const uint8_t *second = material->secondTextureName();
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.primaryTextureID = 0;
        materialPrivate.secondTextureID = 0;
        const uint32_t nIndices = material->countIndices();
        glGenBuffers(1, &materialPrivate.vertexBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialPrivate.vertexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(uint16_t), indicesPtr, GL_STATIC_DRAW);
        indicesPtr += nIndices;
        if (*primary) {
            snprintf(path, sizeof(path), "%s/%s", dir, primary);
            if (LoadTexture(reinterpret_cast<const uint8_t *>(path), textureID))
                materialPrivate.primaryTextureID = textureID;
        }
        if (*second) {
            snprintf(path, sizeof(path), "%s/%s", dir, second);
            if (LoadTexture(reinterpret_cast<const uint8_t *>(path), textureID))
                materialPrivate.secondTextureID = textureID;
        }
        hasSingleSphere |= material->isSpherePrimary() && !material->isSphereAuxSecond();
        hasMultipleSphere |= material->isSphereAuxSecond();
    }
    userData->hasSingleSphereMap = hasSingleSphere;
    userData->hasMultipleSphereMap = hasMultipleSphere;
    glGenBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->stride(vpvl::PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().size() * model->stride(vpvl::PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * model->stride(vpvl::PMDModel::kTextureCoordsStride),
                 model->textureCoordsPointer(), GL_STATIC_DRAW);
    if (LoadToonTexture(system, dir, reinterpret_cast<const uint8_t *>("toon0.bmp"), textureID))
        userData->toonTextureID[0] = textureID;
    for (uint32_t i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (LoadToonTexture(system, dir, name, textureID))
            userData->toonTextureID[i + 1] = textureID;
    }
    userData->materials = materialPrivates;
    model->setUserData(userData);
}

static void UnloadModel(const vpvl::PMDModel *model)
{
    const vpvl::MaterialList materials = model->materials();
    const uint32_t nMaterials = materials.size();
    vpvl::PMDModelUserData *userData = model->userData();
    for (uint32_t i = 0; i < nMaterials; i++) {
        PMDModelMaterialPrivate &materialPrivate = userData->materials[i];
        glDeleteTextures(1, &materialPrivate.primaryTextureID);
        glDeleteTextures(1, &materialPrivate.secondTextureID);
        glDeleteBuffers(1, &materialPrivate.vertexBufferObject);
    }
    glDeleteTextures(vpvl::PMDModel::kSystemTextureMax, userData->toonTextureID);
    glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    delete[] userData->materials;
    delete userData;
}

static void DrawModel(const vpvl::PMDModel *model)
{
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif

    const vpvl::PMDModelUserData *userData = model->userData();
    size_t stride = model->stride(vpvl::PMDModel::kNormalsStride), vsize = model->vertices().size();
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, model->stride(vpvl::PMDModel::kVerticesStride), 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
    glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->normalsPointer(), GL_DYNAMIC_DRAW);
    glNormalPointer(GL_FLOAT, stride, 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glTexCoordPointer(2, GL_FLOAT, model->stride(vpvl::PMDModel::kTextureCoordsStride), 0);

    const bool enableToon = true;
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelToonTexCoords]);
        // shadow map
        stride = model->stride(vpvl::PMDModel::kToonTextureStride);
        if (false)
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
        else
            glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->toonTextureCoordsPointer(), GL_DYNAMIC_DRAW);
        glTexCoordPointer(2, GL_FLOAT, stride, 0);
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    bool hasSingleSphereMap = false, hasMultipleSphereMap = false;
    // first sphere map
    if (userData->hasSingleSphereMap) {
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        hasSingleSphereMap = true;
    }
    // second sphere map
    if (userData->hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        hasMultipleSphereMap = true;
    }

    const vpvl::MaterialList materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const uint32_t nMaterials = materials.size();
    btVector4 average, ambient, diffuse, specular;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        // toon
        const float alpha = material->alpha();
        if (enableToon) {
            average = material->averageColor();
            average.setW(average.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(average));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        else {
            ambient = material->ambient();
            ambient.setW(ambient.w() * alpha);
            diffuse = material->diffuse();
            diffuse.setW(diffuse.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, static_cast<const GLfloat *>(ambient));
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(diffuse));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shiness());
        material->alpha() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        // has texture
        if (materialPrivate.primaryTextureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, materialPrivate.primaryTextureID);
            if (hasSingleSphereMap) {
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
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
        // toon
        if (enableToon) {
            const GLuint textureID = userData->toonTextureID[material->toonID()];
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        if (hasMultipleSphereMap) {
            // second sphere
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            if (materialPrivate.secondTextureID > 0) {
                // is second sphere
                if (material->isSphereAuxSecond())
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glBindTexture(GL_TEXTURE_2D, materialPrivate.secondTextureID);
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        // draw
        const uint32_t nIndices = material->countIndices();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialPrivate.vertexBufferObject);
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, 0);
        // is aux sphere map
        if (material->isSphereAuxPrimary()) {
            glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // toon
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glActiveTexture(GL_TEXTURE0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    // first or second sphere map
    if (hasSingleSphereMap || hasMultipleSphereMap) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    // second sphere map
    if (hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
}

static void DrawModelEdge(const vpvl::PMDModel *model)
{
#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_FRONT);
#else
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_BACK);
#endif

    const float alpha = 1.0f;
    const size_t stride = model->stride(vpvl::PMDModel::kEdgeVerticesStride);
    const btVector4 color(0.0f, 0.0f, 0.0f, alpha);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();

    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * stride, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glColor4fv(static_cast<const btScalar *>(color));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeIndices]);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);

#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_BACK);
#else
    glPopMatrix();
    glCullFace(GL_FRONT);
#endif
}

static void DrawModelShadow(const vpvl::PMDModel *model)
{
    const size_t stride = model->stride(vpvl::PMDModel::kVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * stride, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kShadowIndices]);
    glDrawElements(GL_TRIANGLES, model->indices().size(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
}

#if OLD_STYLE_XMESH_RENDER
static void LoadStage(vpvl::XModel *model, const uint8_t *dir)
{
    vpvl::XModelUserData *userData = new vpvl::XModelUserData;
    userData->listID = glGenLists(1);
    glNewList(userData->listID, GL_COMPILE);
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif
    const btAlignedObjectArray<vpvl::XModelFaceIndex> &faces = model->faces();
    const btAlignedObjectArray<btVector3> &vertices = model->vertices();
    const btAlignedObjectArray<btVector3> &textureCoords = model->textureCoords();
    const btAlignedObjectArray<btVector3> &normals = model->normals();
    const btAlignedObjectArray<btVector4> &colors = model->colors();
    const bool hasMaterials = model->countMatreials() > 0;
    const bool hasTextureCoords = textureCoords.size() > 0;
    const bool hasNormals = normals.size();
    const bool hasColors = colors.size();
    uint32_t nFaces = faces.size();
    uint32_t prevIndex = 0;
    glEnable(GL_TEXTURE_2D);
    for (uint32_t i = 0; i < nFaces; i++) {
        const vpvl::XModelFaceIndex &face = faces[i];
        const btVector4 &value = face.value;
        const uint32_t count = face.count;
        const uint32_t currentIndex = face.index;
        if (hasMaterials && prevIndex != currentIndex) {
            const vpvl::XMaterial *material = model->materialAt(currentIndex);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(material->color()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, static_cast<const GLfloat *>(material->emmisive()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->power());
            const char *textureName = material->textureName();
            if (textureName) {
                btHashString key(textureName);
                GLuint *textureID = userData->textures[key];
                if (!textureID) {
                    GLuint value;
                    char path[256];
                    snprintf(path, sizeof(path), "%s/%s", dir, textureName);
                    if (LoadTexture(reinterpret_cast<const uint8_t *>(path), value)) {
                        userData->textures.insert(key, value);
                        glBindTexture(GL_TEXTURE_2D, value);
                    }
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, *textureID);
                }
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            prevIndex = currentIndex;
        }
        switch (count) {
        case 3:
            glBegin(GL_TRIANGLES);
            break;
        case 4:
            glBegin(GL_QUADS);
            break;
        }
        for (uint32_t j = 0; j < count; j++) {
            const uint32_t x = static_cast<const uint32_t>(value[j]);
            if (hasTextureCoords)
                glTexCoord2fv(textureCoords[x]);
            if (hasColors)
                glColor4fv(colors[x]);
            if (hasNormals)
                glNormal3fv(normals[x]);
            glVertex3fv(vertices[x]);
        }
        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
    glEndList();
    model->setUserData(userData);
}

static void UnloadStage(const vpvl::XModel *model)
{
    vpvl::XModelUserData *userData = model->userData();
    glDeleteLists(userData->listID, 1);
    btHashMap<btHashString, GLuint> &textures = userData->textures;
    uint32_t nTextures = textures.size();
    for (uint32_t i = 0; i < nTextures; i++)
        glDeleteTextures(1, textures.getAtIndex(i));
    textures.clear();
    delete userData;
}

static void DrawStage(const vpvl::XModel *model)
{
    glCallList(model->userData()->listID);
}
#else
static void LoadStage(vpvl::XModel *model, const uint8_t *dir)
{
    vpvl::XModelUserData *userData = new vpvl::XModelUserData;
    uint32_t size = model.countMatreials();
    userData->materials = new XModelMaterialPrivate[size];
    for (uint32_t i = 1; i <= size; i++) {
        const vpvl::XModelIndexList *indices = model.indicesAt(i);
        GLuint &vbo = userData->materials[i - 1].vertexBufferObject;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(uint16_t), &indices[0], GL_STATIC_DRAW);
    }
    glGenBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model.vertices().size() * model.stride(vpvl::XModel::kVerticesStride),
                 model.verticesPointer(), GL_STATIC_DRAW);
    size = model.normals().size();
    if (size > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
        glBufferData(GL_ARRAY_BUFFER, size * model.stride(vpvl::XModel::kNormalsStride),
                     model.normalsPointer(), GL_STATIC_DRAW);
    }
    size = model.colors().size();
    if (size > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelColors]);
        glBufferData(GL_ARRAY_BUFFER, size * model.stride(vpvl::XModel::kColorsStride),
                     model.colorsPointer(), GL_STATIC_DRAW);
    }
    size = model.textureCoords().size();
    if (size > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
        glBufferData(GL_ARRAY_BUFFER, size * model.stride(vpvl::XModel::kTextureCoordsStride),
                     model.textureCoordsPointer(), GL_STATIC_DRAW);
    }
    model.setUserData(userData);
}

static void UnloadStage(const vpvl::XModel *model)
{
    vpvl::XModelUserData *userData = model->userData();
    uint32_t nMaterials = model->countMatreials();
    for (uint32_t i = 0; i < nMaterials; i++) {
        glDeleteBuffers(1, &userData->materials[i].vertexBufferObject);
    }
    delete[] userData->materials;
    glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    delete userData;
}

static void DrawStage(const vpvl::XModel *model)
{
    const vpvl::XModelUserData *userData = model->userData();
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, model->stride(vpvl::XModel::kVerticesStride), 0);
    size_t nNormals = model->normals().size();
    if (nNormals > 0) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
        glNormalPointer(GL_FLOAT, model->stride(vpvl::XModel::kNormalsStride), 0);
    }
    size_t nTextureCoords = model->textureCoords().size();
    if (nTextureCoords > 0) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
        glTexCoordPointer(2, GL_FLOAT, model->stride(vpvl::XModel::kTextureCoordsStride), 0);
    }
    size_t nColors = model->colors().size();
    if (nColors > 0) {
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelColors]);
        glColorPointer(4, GL_FLOAT, model->stride(vpvl::XModel::kColorsStride), 0);
    }

    uint32_t nMaterials = model->countMatreials();
    for (uint32_t i = 1; i <= nMaterials; i++) {
        const vpvl::XMaterial *material = model->materialAt(i - 1);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(material->color()));
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, static_cast<const GLfloat *>(material->emmisive()));
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->power());
        const GLuint &vbo = userData->materials[i - 1].vertexBufferObject;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
        glDrawElements(GL_TRIANGLES, model->indicesAt(i)->size(), GL_UNSIGNED_SHORT, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    if (nNormals > 0)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (nTextureCoords > 0)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (nColors > 0)
        glDisableClientState(GL_COLOR_ARRAY);
}
#endif

static void DrawSurface(vpvl::Scene &scene, vpvl::XModel &stage, int width, int height)
{
    float matrix[16];
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    scene.getProjectionMatrix(matrix);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_MODELVIEW);
    scene.getModelViewMatrix(matrix);
    glLoadMatrixf(matrix);
    glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    DrawStage(&stage);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow
    size_t size = 0;
    vpvl::PMDModel **models = scene.getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        DrawModelShadow(model);
    }
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // render model and edge
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        DrawModel(model);
        DrawModelEdge(model);
    }
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

static void FileSlurp(const char *path, uint8_t *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
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
        fprintf(stderr, "Failed loading the model: %s\n", strerror(errno));
    }
}

static Uint32 UpdateTimer(Uint32 internal, void *data)
{
    vpvl::Scene *scene = static_cast<vpvl::Scene *>(data);
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
    if (!InitializeSurface(surface, g_width, g_height)) {
        return -1;
    }

    char path[256];
    uint8_t *modelData = 0;
    uint8_t *motionData = 0;
    uint8_t *cameraData = 0;
    uint8_t *stageData = 0;
    size_t size = 0;

    snprintf(path, sizeof(path), "%s/%s", g_modeldir, g_modelname);
    FileSlurp(path, modelData, size);
    vpvl::PMDModel model(modelData, size);
    if (!model.load()) {
        fprintf(stderr, "Failed parsing the model\n");
        delete[] modelData;
        return -1;
    }
    LoadModel(&model, g_sysdir, g_modeldir);

    snprintf(path, sizeof(path), "%s/%s", g_stagedir, g_stagename);
    FileSlurp(path, stageData, size);
    vpvl::XModel stage(stageData, size);
    if (!stage.load()) {
        fprintf(stderr, "Failed parsing the stage\n");
        delete[] modelData;
        delete[] stageData;
        return -1;
    }
    LoadStage(&stage, g_stagedir);

    const float dist = 400.0f;
    btDefaultCollisionConfiguration config;
    btCollisionDispatcher dispatcher(&config);
    btAxisSweep3 broadphase(btVector3(-dist, -dist, -dist), btVector3(dist, dist, dist), 1024);
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &config);
    world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
    world.getSolverInfo().m_numIterations = static_cast<int>(10.0f * 60.0f / g_FPS);

    vpvl::Scene scene(g_width, g_height, g_FPS);
    SetLighting(scene);
    //scene.addModel("miku", &model);
    scene.setWorld(&world);

    snprintf(path, sizeof(path), "%s", g_motion);
    FileSlurp(path, motionData, size);
    vpvl::VMDMotion motion(motionData, size);
    if (!motion.load()) {
        fprintf(stderr, "Failed parsing the model motion\n");
        delete[] modelData;
        delete[] stageData;
        delete[] motionData;
        return -1;
    }
    model.addMotion(&motion);

    snprintf(path, sizeof(path), "%s", g_camera);
    FileSlurp(path, cameraData, size);
    vpvl::VMDMotion camera(cameraData, size);
    if (!camera.load()) {
        fprintf(stderr, "Failed parsing the camera motion\n");
        delete[] modelData;
        delete[] stageData;
        delete[] motionData;
        delete[] cameraData;
        return -1;
    }
    scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
    //scene.setCameraMotion(&camera);

    uint32_t interval = static_cast<uint32_t>(1000.0f / g_FPS);
    SDL_TimerID timerID = SDL_AddTimer(interval, UpdateTimer, &scene);
    while (true) {
        if (PollEvents())
            break;
        DrawSurface(scene, stage, g_width, g_height);
    }
    SDL_RemoveTimer(timerID);

    UnloadModel(&model);
    UnloadStage(&stage);
    SDL_FreeSurface(surface);

    delete[] stageData;
    delete[] motionData;
    delete[] modelData;
    delete[] cameraData;

    return 0;
}

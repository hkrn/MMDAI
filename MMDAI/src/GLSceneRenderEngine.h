/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

/* headers */

#if defined(MMDAI_OPENGL_ES1)
#include <OpenGLES/ES1/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <MMDAI/MMDAI.h>

class btConvexShape;

#define SHADOW_PCF                   /* use hardware PCF for shadow mapping */
#define SHADOW_AUTO_VIEW             /* automatically define depth frustum */
#define SHADOW_AUTO_VIEW_ANGLE 15.0f /* view angle for automatic depth frustum */

namespace MMDAI {

#ifdef MMDAI_OPENGL_ES1
static void GLGenSphereCoords(TexCoord **ptr,
                              const btVector3 *positions,
                              const btVector3 *normals,
                              const unsigned int count)
{
    TexCoord *p = *ptr;
    for (unsigned int i = 0; i <  count; i++) {
        const btVector3 position = positions[i];
        const btVector3 normal = normals[i];
        const btVector3 r = position - 2 * normal.dot(position) * normal;
        const btScalar x = r.x();
        const btScalar y = r.y();
        const btScalar z = r.z() + 1.0;
        const btScalar m = 2.0 * sqrt(x * x + y * y + z * z);
        TexCoord *coord = &p[i];
        coord->u = x / m + 0.5;
        coord->v = y / m + 0.5;
    }
}
#endif /* MMDAI_OPENGL_ES1 */

enum GLSceneRenderEngineTextureID {
    kModelTextureID = GL_TEXTURE0,
    kToonTextureID  = GL_TEXTURE1,
    kSPATextureID   = GL_TEXTURE2,
    kDepthTextureID = GL_TEXTURE3,
};

enum GLSceneRenderEngineBufferID {
    kEdgeVertices,
    kEdgeIndices,
    kShadowVertices,
    kShadowIndices,
    kModelVertices,
    kModelNormals,
    kModelTexCoords,
    kModelToonTexCoords,
#ifdef MMDAI_OPENGL_ES1
    kModelSphereMapCoords,
    kModelSphereMap2Coords,
#endif
    kModelBufferMax,
};

class GLPMDModel : public PMDModel {
public:
    GLPMDModel(PMDRenderEngine *engine)
      : PMDModel(engine),
        m_materialVBO(NULL),
        m_nmaterials(0)
    {
#ifdef MMDAI_OPENGL_ES1
        m_spheres = NULL;
        m_spheres2 = NULL;
#endif
    }
    ~GLPMDModel() {
        glDeleteBuffers(sizeof(m_modelVBO) / sizeof(GLuint), m_modelVBO);
        if (m_materialVBO) {
            glDeleteBuffers(m_nmaterials, m_materialVBO);
            MMDAIMemoryRelease(m_materialVBO);
            m_materialVBO = NULL;
        }
#ifdef MMDAI_OPENGL_ES1
        if (m_spheres) {
            for (unsigned int i = 0; i < m_nmaterials; i++) {
                MMDAIMemoryRelease(m_spheres[i]);
            }
            MMDAIMemoryRelease(m_spheres);
            m_spheres = NULL;
        }
        if (m_spheres2) {
            for (unsigned int i = 0; i < m_nmaterials; i++) {
                MMDAIMemoryRelease(m_spheres2[i]);
            }
            MMDAIMemoryRelease(m_spheres2);
            m_spheres2 = NULL;
        }
#endif /* MMDAI_OPENGL_ES1 */
    }
    bool load(IModelLoader *loader, BulletPhysics *bullet) {
        if (!PMDModel::load(loader, bullet))
            return false;
        const unsigned short *surfaceData = getSurfacesPtr();
#ifdef MMDAI_OPENGL_ES1
        const unsigned int nvertices = countVertices();
        const bool hasSingleSphereMap = this->hasSingleSphereMap();
        const bool hasMultipleSphereMap = this->hasMultipleSphereMap();
#endif /* MMDAI_OPENGL_ES1 */
        m_nmaterials = countMaterials();
        m_materialVBO = static_cast<GLuint *>(MMDAIMemoryAllocate(m_nmaterials * sizeof(GLuint)));
        if (m_materialVBO == NULL)
            return false;
#ifdef MMDAI_OPENGL_ES1
        m_spheres = static_cast<TexCoord **>(MMDAIMemoryAllocate(sizeof(void *) * m_nmaterials));
        if (m_spheres == NULL)
            return false;
        m_spheres2 = static_cast<TexCoord **>(MMDAIMemoryAllocate(sizeof(void *) * m_nmaterials));
        if (m_spheres2 == NULL)
            return false;
#endif /* MMDAI_OPENGL_ES1 */
        glGenBuffers(sizeof(m_modelVBO) / sizeof(GLuint), m_modelVBO);
        glGenBuffers(m_nmaterials, m_materialVBO);
        // edge buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelVBO[kEdgeIndices]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumSurfaceForEdge() * sizeof(GLushort), getSurfacesForEdgePtr(), GL_STATIC_DRAW);
        // shadow buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelVBO[kShadowIndices]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, countSurfaces() * sizeof(GLushort), getSurfacesPtr(), GL_STATIC_DRAW);
        // texture for model buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_modelVBO[kModelTexCoords]);
        glBufferData(GL_ARRAY_BUFFER, countVertices() * sizeof(TexCoord), getTexCoordsPtr(), GL_STATIC_DRAW);
        // material indices
        for (unsigned int i = 0; i < m_nmaterials; i++) {
            PMDMaterial *m = getMaterialAt(i);
#ifdef MMDAI_OPENGL_ES1
            const PMDTexture *tex = m->getTexture();
            m_spheres[i] = NULL;
            m_spheres2[i] = NULL;
            if (tex != NULL) {
                const PMDTextureNative *native = tex->getNative();
                if (native != NULL && hasSingleSphereMap && tex->isSPH()) {
                    TexCoord *coords = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * nvertices));
                    if (coords != NULL) {
                        memset(coords, 0, sizeof(TexCoord) * nvertices);
                        m_spheres[i] = coords;
                    }
                }
            }
            if (hasMultipleSphereMap) {
                const PMDTexture *addtex = m->getAdditionalTexture();
                if (addtex != NULL) {
                    const PMDTextureNative *native = addtex->getNative();
                    if (native != NULL) {
                        TexCoord *coords = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * nvertices));
                        if (coords != NULL) {
                            memset(coords, 0, sizeof(TexCoord) * nvertices);
                            m_spheres2[i] = coords;
                        }
                    }
                }
            }
#endif /* MMDAI_OPENGL_ES1 */
            const int nsurfaces = m->countSurfaces();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_materialVBO[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nsurfaces * sizeof(GLushort), surfaceData, GL_STATIC_DRAW);
            surfaceData += nsurfaces;
        }
        return true;
    }
    inline GLuint getMaterialVBOAt(GLuint index) const {
        return m_materialVBO[index];
    }
    inline GLuint getModelVBOAt(GLSceneRenderEngineBufferID index) const {
        return m_modelVBO[index];
    }

#ifdef MMDAI_OPENGL_ES1
    inline TexCoord *getSphereCoordsAt(GLuint index) const {
        return m_spheres[index];
    }
    inline TexCoord *getSphereCoords2At(GLuint index) const {
        return m_spheres2[index];
    }
#endif /* MMDAI_OPENGL_ES1 */

private:
    GLuint *m_materialVBO;
    GLuint m_modelVBO[kModelBufferMax];
    GLuint m_nmaterials;

#ifdef MMDAI_OPENGL_ES1
    TexCoord **m_spheres;
    TexCoord **m_spheres2;
#endif /* MMDAI_OPENGL_ES1 */

};

class GLPMDMaterial : public PMDMaterial {
public:
    GLPMDMaterial(PMDRenderEngine *engine) : PMDMaterial(engine) {
    }
};

struct PMDTextureNative {
    GLuint id;
};

struct PMDRenderCacheNative {
    GLuint id;
};

class GLSceneRenderEngine : public SceneRenderEngine {
public:
    GLSceneRenderEngine(IPreference *preference)
        : m_preference(preference),
          m_lightVec(0.0f, 0.0f, 0.0f),
          m_shadowMapAutoViewEyePoint(0.0f, 0.0f, 0.0f),
          m_shadowMapAutoViewRadius(0.0f),
          m_depthTextureID(0),
          m_fboID(0),
          m_shadowMapInitialized(false)
    {
        memset(m_modelView, 0, sizeof(m_modelView));
        memset(m_projection, 0, sizeof(m_projection));
    }

    ~GLSceneRenderEngine()
    {
#ifndef MMDAI_OPENGL_ES1
        if (m_shadowMapInitialized) {
            glDeleteFramebuffers(1, &m_fboID);
            glDeleteTextures(1, &m_depthTextureID);
            m_shadowMapInitialized = false;
        }
#endif /* MMDAI_OPENGL_ES1 */
    }

    PMDMaterial **allocateMaterials(int size)
    {
        PMDMaterial **materials = reinterpret_cast<PMDMaterial **>(new GLPMDMaterial*[size]);
        for (int i = 0; i < size; i++) {
            materials[i] = new GLPMDMaterial(this);
        }
        return materials;
    }

    void releaseMaterials(PMDMaterial **materials, int size)
    {
        for (int i = 0; i < size; i++) {
            delete materials[i];
        }
        delete[] materials;
    }

    PMDModel *allocateModel()
    {
        return new GLPMDModel(this);
    }

    bool loadModel(PMDModel *model, IModelLoader *loader, BulletPhysics *bullet)
    {
        return reinterpret_cast<GLPMDModel *>(model)->load(loader, bullet);
    }

    void releaseModel(PMDModel *model)
    {
        delete model;
    }

    /* needs multi-texture function on OpenGL: */
    /* texture unit 0: model texture */
    /* texture unit 1: toon texture for toon shading */
    /* texture unit 2: additional sphere map texture, if exist */
    void renderModel(PMDModel *ptr)
    {
        GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
        const btVector3 *vertices = model->getSkinnedVerticesPtr();
        const btVector3 *normals = model->getSkinnedNormalsPtr();
        const unsigned int nvertices = model->countVertices();
        if (!vertices || !normals)
            return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
        glPushMatrix();
        glScalef(1.0f, 1.0f, -1.0f); /* from left-hand to right-hand */
        glCullFace(GL_FRONT);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */

        /* activate texture unit 0 */
        glActiveTexture(kModelTextureID);
        glClientActiveTexture(kModelTextureID);

        /* set lists */
        size_t vsize = nvertices  * sizeof(btVector3);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelVertices));
        glBufferData(GL_ARRAY_BUFFER, vsize, model->getSkinnedVerticesPtr(), GL_DYNAMIC_DRAW);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelNormals));
        glBufferData(GL_ARRAY_BUFFER, vsize, model->getSkinnedNormalsPtr(), GL_DYNAMIC_DRAW);
        glNormalPointer(GL_FLOAT, sizeof(btVector3), NULL);

        /* set model texture coordinates to texture unit 0 */
        glClientActiveTexture(kModelTextureID);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelTexCoords));
        glTexCoordPointer(2, GL_FLOAT, 0, NULL);

        const bool enableToon = model->isToonEnabled();
        const bool hasSingleSphereMap = model->hasSingleSphereMap();
        const bool hasMultipleSphereMap = model->hasMultipleSphereMap();

        if (enableToon) {
            /* set toon texture coordinates to texture unit 1 */
            glActiveTexture(kToonTextureID);
            glEnable(GL_TEXTURE_2D);
            glClientActiveTexture(kToonTextureID);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelToonTexCoords));
            if (model->isSelfShadowEnabled()) {
                /* when drawing a shadow part in shadow mapping, force toon texture coordinates to (0, 0) */
                glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), model->getToonTexCoordsForSelfShadowPtr(), GL_DYNAMIC_DRAW);
                glTexCoordPointer(2, GL_FLOAT, 0, NULL);
            }
            else {
                glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), model->getToonTexCoordsPtr(), GL_DYNAMIC_DRAW);
                glTexCoordPointer(2, GL_FLOAT, 0, NULL);
            }
            glActiveTexture(kModelTextureID);
            glClientActiveTexture(kModelTextureID);
        }

#ifndef MMDAI_OPENGL_ES1
        if (hasSingleSphereMap) {
            /* this model contains single sphere map texture */
            /* set texture coordinate generation for sphere map on texture unit 0 */
            glEnable(GL_TEXTURE_2D);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glDisable(GL_TEXTURE_2D);
        }
        if (hasMultipleSphereMap) {
            /* this model contains additional sphere map texture */
            /* set texture coordinate generation for sphere map on texture unit 2 */
            glActiveTexture(kSPATextureID);
            glEnable(GL_TEXTURE_2D);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(kModelTextureID);
        }
#endif /* MMDAI_OPENGL_ES1 */

        /* calculate alpha value, applying model global alpha */
        const float modelAlpha = model->getGlobalAlpha();

        /* render per material */
        const int nmaterials = model->countMaterials();
        for (int i = 0; i < nmaterials; i++) {
            float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            PMDMaterial *m = model->getMaterialAt(i);
            /* set colors */
            double alpha = m->getAlpha();
            c[3] = alpha * modelAlpha;
            if (c[3] > 0.99f) {
                c[3] = 1.0f; /* clamp to 1.0 */
            }
            if (enableToon) {
                /* use averaged color of diffuse and ambient for both */
                m->copyAvgcol(c);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &(c[0]));
                m->copySpecular(c);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
            }
            else {
                /* use each color */
                m->copyDiffuse(c);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(c[0]));
                m->copyAmbient(c);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(c[0]));
                m->copySpecular(c);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
            }
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m->getShiness());

            /* disable face culling for transparent materials */
            if (alpha < 1.0f) {
                glDisable(GL_CULL_FACE);
            }
            else {
                glEnable(GL_CULL_FACE);
            }

            /* if using multiple texture units, set current unit to 0 */
            if (enableToon || hasMultipleSphereMap) {
                glActiveTexture(kModelTextureID);
            }

            const PMDTexture *texture = m->getTexture();
            if (texture != NULL) {
                /* bind model texture */
                const PMDTextureNative *native = texture->getNative();
                if (native != NULL) {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, native->id);
                    if (hasSingleSphereMap) {
                        if (texture->isSPH()) {
                            /* this is sphere map */
                            /* enable texture coordinate generation */
                            if (texture->isSPA())
                                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
#ifndef MMDAI_OPENGL_ES1
                            glEnable(GL_TEXTURE_GEN_S);
                            glEnable(GL_TEXTURE_GEN_T);
#else
                            TexCoord *coords = model->getSphereCoordsAt(i);
                            if (coords != NULL) {
                                GLGenSphereCoords(&coords, vertices, normals, nvertices);
                                glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelSphereMapCoords));
                                glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), coords, GL_DYNAMIC_DRAW);
                                glTexCoordPointer(2, GL_FLOAT, 0, NULL);
                            }
#endif /* MMDAI_OPENGL_ES1 */
                        }
                        else {
#ifndef MMDAI_OPENGL_ES1
                            /* disable generation */
                            glDisable(GL_TEXTURE_GEN_S);
                            glDisable(GL_TEXTURE_GEN_T);
#else
                            glEnable(GL_TEXTURE_2D);
                            glBindTexture(GL_TEXTURE_2D, 0);
#endif /* MMDAI_OPENGL_ES1 */
                        }
                    }
                }
                else {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
            else {
                glDisable(GL_TEXTURE_2D);
            }

            if (enableToon) {
                /* set toon texture for texture unit 1 */
                const PMDTextureNative *native = model->getToonTextureAt(m->getToonID())->getNative();
                if (native != NULL) {
                    glActiveTexture(kToonTextureID);
                    glBindTexture(GL_TEXTURE_2D, native->id);
                    /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
            }

            if (hasMultipleSphereMap) {
                const PMDTexture *addtex = m->getAdditionalTexture();
                if (addtex != NULL) {
                    /* this material has additional sphere map texture, bind it at texture unit 2 */
                    glActiveTexture(kSPATextureID);
                    glEnable(GL_TEXTURE_2D);
                    if (addtex->isSPA()) {
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                    }
                    else {
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                    }
                    const PMDTextureNative *native = addtex->getNative();
                    if (native != NULL) {
                        glBindTexture(GL_TEXTURE_2D, native->id);
#ifdef MMDAI_OPENGL_ES1
                        TexCoord *coords = model->getSphereCoords2At(i);
                        if (coords != NULL) {
                            GLGenSphereCoords(&coords, vertices, normals, nvertices);
                            glClientActiveTexture(kSPATextureID);
                            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                            glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelSphereMap2Coords));
                            glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), coords, GL_DYNAMIC_DRAW);
                            glTexCoordPointer(2, GL_FLOAT, 0, NULL);
                        }
#endif /* MMDAI_OPENGL_ES1 */
                    }
                    else {
                        glBindTexture(GL_TEXTURE_2D, 0);
                    }
#ifndef MMDAI_OPENGL_ES1
                    glEnable(GL_TEXTURE_GEN_S);
                    glEnable(GL_TEXTURE_GEN_T);
#endif /* MMDAI_OPENGL_ES1 */
                }
                else {
                    /* disable generation */
                    glActiveTexture(kSPATextureID);
                    glDisable(GL_TEXTURE_2D);
                }
            }

            /* draw elements */
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getMaterialVBOAt(i));
            glDrawElements(GL_TRIANGLES, m->countSurfaces(), GL_UNSIGNED_SHORT, NULL);

            /* reset some parameters */
            if (texture && texture->isSPH() && texture->isSPA()) {
                if (enableToon)
                    glActiveTexture(kModelTextureID);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if (enableToon) {
            glClientActiveTexture(kModelTextureID);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef MMDAI_OPENGL_ES1
            if (hasSingleSphereMap) {
                glActiveTexture(kModelTextureID);
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
            }
#endif /* MMDAI_OPENGL_ES1 */
            glClientActiveTexture(kToonTextureID);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef MMDAI_OPENGL_ES1
            if (hasMultipleSphereMap) {
                glActiveTexture(kSPATextureID);
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
            }
#endif /* MMDAI_OPENGL_ES1 */
            glActiveTexture(kModelTextureID);
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifndef MMDAI_OPENGL_ES1
            if (hasSingleSphereMap) {
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
            }
#endif /* MMDAI_OPENGL_ES1 */
            if (hasMultipleSphereMap) {
                glActiveTexture(kSPATextureID);
#ifndef MMDAI_OPENGL_ES1
                glDisable(GL_TEXTURE_GEN_S);
                glDisable(GL_TEXTURE_GEN_T);
#endif /* MMDAI_OPENGL_ES1 */
                glActiveTexture(kModelTextureID);
            }
        }

#ifndef MMDAI_OPENGL_ES1
        if (hasSingleSphereMap || hasMultipleSphereMap) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
#endif /* MMDAI_OPENGL_ES1 */

        if (enableToon) {
            glActiveTexture(kToonTextureID);
            glDisable(GL_TEXTURE_2D);
        }
        if (hasMultipleSphereMap) {
            glActiveTexture(kSPATextureID);
            glDisable(GL_TEXTURE_2D);
        }
        glActiveTexture(kModelTextureID);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_CULL_FACE);
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
        glCullFace(GL_BACK);
        glPopMatrix();
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
    }

    void renderEdge(PMDModel *ptr)
    {
        GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
        const btVector3 *vertices = model->getVerticesPtr();
        const bool enableToon = model->isToonEnabled();
        const unsigned int nsurfaces = model->getNumSurfaceForEdge();
        if (!vertices || !enableToon || nsurfaces == 0)
            return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
        glPushMatrix();
        glScalef(1.0f, 1.0f, -1.0f);
        glCullFace(GL_BACK);
#else
        /* draw back surface only */
        glCullFace(GL_FRONT);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */

        /* calculate alpha value */
        const float modelAlpha = model->getGlobalAlpha();
        const float *edgeColors = model->getEdgeColors();

        /* FIXME: imlement force edge */
        glDisable(GL_LIGHTING);
        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kEdgeVertices));
        glBufferData(GL_ARRAY_BUFFER, model->countVertices() * sizeof(btVector3), model->getEdgeVerticesPtr(), GL_DYNAMIC_DRAW);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
        glColor4f(edgeColors[0], edgeColors[1], edgeColors[2], edgeColors[3] * modelAlpha);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getModelVBOAt(kEdgeIndices));
        glDrawElements(GL_TRIANGLES, nsurfaces, GL_UNSIGNED_SHORT, NULL);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_LIGHTING);

        /* draw front again */
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
        glPopMatrix();
        glCullFace(GL_FRONT);
#else
        glCullFace(GL_BACK);
#endif /* MMDFILES_CONVERTCOORDINATESYSTEM */
    }

    void renderShadow(PMDModel *ptr)
    {
        GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
        const btVector3 *vertices = model->getVerticesPtr();
        const unsigned int nsurfaces = model->countSurfaces();
        if (!vertices || nsurfaces == 0)
            return;

        glDisable(GL_CULL_FACE);
        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kShadowVertices));
        glBufferData(GL_ARRAY_BUFFER, model->countVertices() * sizeof(btVector3), model->getSkinnedVerticesPtr(), GL_DYNAMIC_DRAW);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getModelVBOAt(kShadowIndices));
        glDrawElements(GL_TRIANGLES, nsurfaces, GL_UNSIGNED_SHORT, NULL);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_CULL_FACE);
    }

    PMDTextureNative *allocateTexture(const unsigned char *data,
                                      const int width,
                                      const int height,
                                      const int components)
    {
        /* generate texture */
        GLuint format = 0;
        PMDTextureNative *native = new PMDTextureNative;
        glGenTextures(1, &native->id);
        glBindTexture(GL_TEXTURE_2D, native->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (components == 3) {
            format = GL_RGB;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        } else {
            format = GL_RGBA;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

#ifndef MMDAI_OPENGL_ES1
        /* set highest priority to this texture to tell OpenGL to keep textures in GPU memory */
        GLfloat priority = 1.0f;
        glPrioritizeTextures(1, &native->id, &priority);
#endif /* MMDAI_OPENGL_ES1 */

        return native;
    }

    void releaseTexture(PMDTextureNative *native)
    {
        if (native != NULL) {
            glDeleteTextures(1, &native->id);
            delete native;
        }
    }

    void renderModelCached(PMDModel *model, PMDRenderCacheNative **ptr)
    {
#ifndef MMDAI_OPENGL_ES1
        PMDRenderCacheNative *native = *ptr;
        if (native != NULL) {
            glCallList(native->id);
        }
        else {
            *ptr = native = new PMDRenderCacheNative();
            native->id = glGenLists(1);
            glNewList(native->id, GL_COMPILE);
            glPushMatrix();
            renderModel(model);
            glPopMatrix();
            glEndList();
        }
#else
        (void) ptr;
        renderModel(model);
#endif /* MMDAI_OPENGL_ES1 */
    }

    void renderTileTexture(PMDTexture *texture,
                           const float *color,
                           const float *normal,
                           const float *vertices1,
                           const float *vertices2,
                           const float *vertices3,
                           const float *vertices4,
                           const float nX,
                           const float nY,
                           const bool cullFace,
                           PMDRenderCacheNative **ptr)
    {
#ifndef MMDAI_OPENGL_ES1
        PMDRenderCacheNative *native = *ptr;
        if (native != NULL) {
            glCallList(native->id);
            return;
        }

        *ptr = native = new PMDRenderCacheNative();
        native->id = glGenLists(1);
        glNewList(native->id, GL_COMPILE);
#endif /* MMDAI_OPENGL_ES1 */

        /* register rendering command */
        if (!cullFace)
            glDisable(GL_CULL_FACE);

        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
#ifndef MMDAI_OPENGL_ES1
        glNormal3f(normal[0], normal[1], normal[2]);
        glBindTexture(GL_TEXTURE_2D, texture->getNative()->id);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, nY);
        glVertex3fv(vertices1);
        glTexCoord2f(nX, nY);
        glVertex3fv(vertices2);
        glTexCoord2f(nX, 0.0);
        glVertex3fv(vertices3);
        glTexCoord2f(0.0, 0.0);
        glVertex3fv(vertices4);
        glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
#else
        (void) ptr;
        const float *vertices[] = {
            vertices1,
            vertices2,
            vertices3,
            vertices4
        };
        const float *normals[] = {
            normal,
            normal,
            normal,
            normal
        };
        const float coords[] = {
            0.0, nY,
            nX, nY,
            nX, 0.0,
            0.0, 0.0
        };
        const int indices[] = {
            0, 1, 2, 2, 1, 3
        };
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindTexture(GL_TEXTURE_2D, texture->getNative()->id);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glNormalPointer(GL_FLOAT, 0, normals);
        glTexCoordPointer(2, GL_FLOAT, 0, coords);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_SHORT, indices);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif /* MMDAI_OPENGL_ES1 */

        if (!cullFace)
            glEnable(GL_CULL_FACE);

#ifndef MMDAI_OPENGL_ES1
        /* end of regist */
        glEndList();
#endif /* MMDAI_OPENGL_ES1 */
    }

    void deleteCache(PMDRenderCacheNative **ptr)
    {
#ifndef MMDAI_OPENGL_ES1
        PMDRenderCacheNative *native = *ptr;
        if (native != NULL) {
            delete native;
            *ptr = 0;
        }
#else
        (void) ptr;
#endif
    }

    /* setup: initialize and setup Renderer */
    bool setup()
    {
        /* set clear color */
        float campusColor[3];
        m_preference->getFloat3(kPreferenceCampusColor, campusColor);
        glClearColor(campusColor[0], campusColor[1], campusColor[2], 0.0f);
        glClearStencil(0);

        /* enable depth test */
        glEnable(GL_DEPTH_TEST);

        /* enable texture */
        glEnable(GL_TEXTURE_2D);

        /* enable face culling */
        glEnable(GL_CULL_FACE);
        /* not Renderer the back surface */
        glCullFace(GL_BACK);

        /* enable alpha blending */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* enable alpha test, to avoid zero-alpha surfaces to depend on the Renderering order */
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, 0.05f);

        /* enable lighting */
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);

        /* initialization for shadow mapping */
        setShadowMapping();

        return true;
    }

    /* initializeShadowMap: initialize OpenGL for shadow mapping */
    void initializeShadowMap()
    {
#ifndef MMDAI_OPENGL_ES1
        static const GLdouble genfunc[][4] = {
            { 1.0, 0.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0, 0.0 },
            { 0.0, 0.0, 1.0, 0.0 },
            { 0.0, 0.0, 0.0, 1.0 },
        };

        /* initialize model view matrix */
        glPushMatrix();
        glLoadIdentity();

        /* use 4th texture unit for depth texture, make it current */
        glActiveTexture(kDepthTextureID);

        /* prepare a texture object for depth texture Renderering in frame buffer object */
        glGenTextures(1, &m_depthTextureID);
        glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

        /* assign depth component to the texture */
        int shadowMapTextureSize = m_preference->getInt(kPreferenceShadowMappingTextureSize);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapTextureSize, shadowMapTextureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

        /* set texture parameters for shadow mapping */
#ifdef SHADOW_PCF
        /* use hardware PCF */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif /* SHADOW_PCF */

        /* tell OpenGL to compare the R texture coordinates to the (depth) texture value */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

        /* also tell OpenGL to get the compasiron result as alpha value */
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);

        /* set texture coordinates generation mode to use the raw texture coordinates (S, T, R, Q) in eye view */
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]);
        glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
        glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
        glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);

        /* finished configuration of depth texture: unbind the texture */
        glBindTexture(GL_TEXTURE_2D, 0);

        /* allocate a frame buffer object (FBO) for depth buffer Renderering */
        glGenFramebuffers(1, &m_fboID);
        /* switch to the newly allocated FBO */
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        /* bind the texture to the FBO, telling that it should Renderer the depth information to the texture */
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
        /* also tell OpenGL not to draw and read the color buffers */
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        /* check FBO status */
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            /* cannot use FBO */
        }
        /* finished configuration of FBO, now switch to default frame buffer */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* reset the current texture unit to default */
        glActiveTexture(kModelTextureID);

        /* restore the model view matrix */
        glPopMatrix();
#endif /* MMDAI_OPENGL_ES1 */
    }

    /* setShadowMapping: switch shadow mapping */
    void setShadowMapping()
    {
#ifndef MMDAI_OPENGL_ES1
        if (m_preference->getBool(kPreferenceUseShadowMapping)) {
            /* enabled */
            if (!m_shadowMapInitialized) {
                /* initialize now */
                initializeShadowMap();
                m_shadowMapInitialized = true;
            }
            /* set how to set the comparison result value of R coordinates and texture (depth) value */
            glActiveTexture(kDepthTextureID);
            glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
            if (m_preference->getBool(kPreferenceShadowMappingLightFirst)) {
                /* when Renderering order is light(full) - dark(shadow part), OpenGL should set the shadow part as true */
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
            } else {
                /* when Renderering order is dark(full) - light(non-shadow part), OpenGL should set the shadow part as false */
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            }
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(kModelTextureID);
            MMDAILogInfoString("Shadow mapping enabled");
        } else {
            /* disabled */
            if (m_shadowMapInitialized) {
                /* disable depth texture unit */
                glActiveTexture(kDepthTextureID);
                glDisable(GL_TEXTURE_2D);
                glActiveTexture(kModelTextureID);
            }
            MMDAILogInfoString("Shadow mapping disabled");
        }
#endif /* MMDAI_OPENGL_ES1 */
    }

    void prerender(PMDObject **objects, int16_t *order, int size)
    {
#ifndef MMDAI_OPENGL_ES1
        if (m_preference->getBool(kPreferenceUseShadowMapping)) {
            int i = 0;
            GLint viewport[4]; /* store viewport */
            GLdouble projection[16]; /* store projection transform */

#ifdef SHADOW_AUTO_VIEW
            float fovy = 0.0, eyeDist = 0.0;
            btVector3 v;
#endif /* SHADOW_AUTO_VIEW */

            /* Renderer the depth texture */
            /* store the current viewport */
            glGetIntegerv(GL_VIEWPORT, viewport);

            /* store the current projection matrix */
            glGetDoublev(GL_PROJECTION_MATRIX, projection);

            /* switch to FBO for depth buffer Renderering */
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

            /* clear the buffer */
            /* clear only the depth buffer, since other buffers will not be used */
            glClear(GL_DEPTH_BUFFER_BIT);

            /* set the viewport to the required texture size */
            const int shadowMappingTextureSize = m_preference->getInt(kPreferenceShadowMappingTextureSize);
            glViewport(0, 0, shadowMappingTextureSize, shadowMappingTextureSize);

            /* reset the projection matrix */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            /* set the model view matrix to make the light position as eye point and capture the whole scene in the view */
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

#ifdef SHADOW_AUTO_VIEW
            /* auto-update the view area according to the model */
            /* the model range and position has been computed at model updates before */
            /* set the view angle */
            fovy = SHADOW_AUTO_VIEW_ANGLE;
            /* set the distance to cover all the model range */
            eyeDist = m_shadowMapAutoViewRadius / sinf(fovy * 0.5f * 3.1415926f / 180.0f);
            /* set the perspective */
            gluPerspective(fovy, 1.0, 1.0, eyeDist + m_shadowMapAutoViewRadius + 50.0f); /* +50.0f is needed to cover the background */
            /* the viewpoint should be at eyeDist far toward light direction from the model center */
            v = m_lightVec * eyeDist + m_shadowMapAutoViewEyePoint;
            gluLookAt(v.x(), v.y(), v.z(), m_shadowMapAutoViewEyePoint.x(), m_shadowMapAutoViewEyePoint.y(), m_shadowMapAutoViewEyePoint.z(), 0.0, 1.0, 0.0);
#else
            /* fixed view */
            gluPerspective(25.0, 1.0, 1.0, 120.0);
            gluLookAt(30.0, 77.0, 30.0, 0.0, 17.0, 0.0, 0.0, 1.0, 0.0);
#endif

            /* keep the current model view for later process */
            glGetDoublev(GL_MODELVIEW_MATRIX, m_modelView2);

            /* do not write into frame buffer other than depth information */
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            /* also, lighting is not needed */
            glDisable(GL_LIGHTING);

            /* disable Renderering the front surface to get the depth of back face */
            glCullFace(GL_FRONT);

            /* disable alpha test */
            glDisable(GL_ALPHA_TEST);

            /* we are now writing to depth texture using FBO, so disable the depth texture mapping here */
            glActiveTexture(kDepthTextureID);
            glDisable(GL_TEXTURE_2D);
            glActiveTexture(kModelTextureID);

            /* set polygon offset to avoid "moire" */
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(4.0, 4.0);

            /* Renderer objects for depth */
            /* only objects that wants to drop shadow should be Renderered here */
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                renderShadow(object->getModel());
            }

            /* reset the polygon offset */
            glDisable(GL_POLYGON_OFFSET_FILL);

            /* switch to default FBO */
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            /* revert configurations to normal Renderering */
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixd(projection);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_LIGHTING);
            glCullFace(GL_BACK);
            glEnable(GL_ALPHA_TEST);

            /* clear all the buffers */
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        else {
            /* clear Renderering buffer */
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
#else
        (void) objects;
        (void) order;
        (void) size;
        /* clear Renderering buffer */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif /* MMDAI_OPENGL_ES1 */
    }

    /* render: Render all */
    void render(PMDObject **objects, int16_t *order, int size, Stage *stage)
    {
#ifndef MMDAI_OPENGL_ES1
        if (m_preference->getBool(kPreferenceUseShadowMapping))
            renderSceneShadowMap(objects, order, size, stage);
        else
            renderScene(objects, order, size, stage);
#else
        renderScene(objects, order, size, stage);
#endif
    }

    /* pickModel: pick up a model at the screen position */
    int pickModel(PMDObject **objects,
                  int size,
                  int x,
                  int y,
                  int *allowDropPicked)
    {
#ifndef MMDAI_OPENGL_ES1
        int i;

        GLuint selectionBuffer[512];
        GLint viewport[4];

        GLint hits;
        GLuint *data;
        GLuint minDepth = 0, minDepthAllowDrop = 0;
        int minID, minIDAllowDrop;
        GLuint depth;
        int id;

        /* get current viewport */
        glGetIntegerv(GL_VIEWPORT, viewport);
        /* set selection buffer */
        glSelectBuffer(512, selectionBuffer);
        /* begin selection mode */
        glRenderMode(GL_SELECT);
        /* save projection matrix */
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        /* set projection matrix for picking */
        glLoadIdentity();
        /* apply picking matrix */
        gluPickMatrix(x, viewport[3] - y, 15.0, 15.0, viewport);
        /* apply normal projection matrix */
        glLoadIdentity();
        glMultMatrixf(m_projection);
        /* switch to model view mode */
        glMatrixMode(GL_MODELVIEW);
        /* initialize name buffer */
        glInitNames();
        glPushName(0);

        /* draw models with selection names */
        for (i = 0; i < size; i++) {
            PMDObject *object = objects[i];
            if (!object->isEnable())
                continue;
            glLoadName(i);
            renderShadow(object->getModel());
        }

        /* restore projection matrix */
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        /* switch to model view mode */
        glMatrixMode(GL_MODELVIEW);
        /* end selection mode and get number of hits */
        hits = glRenderMode(GL_RENDER);
        if (hits == 0)
            return -1;

        data = &(selectionBuffer[0]);
        minID = -1;
        minIDAllowDrop = -1;
        for (i = 0; i < hits; i++) {
            depth = *(data + 1);
            id = *(data + 3);
            if (minID == -1 || minDepth > depth) {
                minDepth = depth;
                minID = id;
            }
            if (allowDropPicked && objects[id]->allowMotionFileDrop()) {
                if (minIDAllowDrop == -1 || minDepthAllowDrop > depth) {
                    minDepthAllowDrop = depth;
                    minIDAllowDrop = id;
                }
            }
            data += *data + 3;
        }
        if (allowDropPicked)
            *allowDropPicked = minIDAllowDrop;

        return minID;
#else
        (void) objects;
        (void) size;
        (void) x;
        (void) y;
        (void) allowDropPicked;
        return -1;
#endif
    }

    /* updateLigithing: update light */
    void updateLighting()
    {
        const float lightIntensity = m_preference->getFloat(kPreferenceLightIntensity);
        float lightColor[3];
        float lightDirection[4];
        float lightDiffuse[4];
        float lightAmbient[4];
        float lightSpecular[4];
        int i = 0;
        float diffuse = 0, ambinet = 0, specular = 0;

        m_preference->getFloat3(kPreferenceLightColor, lightColor);
        m_preference->getFloat4(kPreferenceLightDirection, lightDirection);
        if (!m_preference->getBool(kPreferenceUseMMDLikeCartoon)) {
            /* MMDAgent original cartoon */
            diffuse = 0.2f;
            ambinet = lightIntensity * 2.0f;
            specular = 0.4f;
        } else if (m_preference->getBool(kPreferenceUseCartoonRendering)) {
            /* like MikuMikuDance */
            diffuse = 0.0f;
            ambinet = lightIntensity * 2.0f;
            specular = lightIntensity;
        } else {
            /* no toon */
            diffuse = lightIntensity;
            ambinet = 1.0f;
            specular = 1.0f; /* OpenGL default */
        }

        for (i = 0; i < 3; i++)
            lightDiffuse[i] = lightColor[i] * diffuse;
        lightDiffuse[3] = 1.0f;
        for (i = 0; i < 3; i++)
            lightAmbient[i] = lightColor[i] * ambinet;
        lightAmbient[3] = 1.0f;
        for (i = 0; i < 3; i++)
            lightSpecular[i] = lightColor[i] * specular;
        lightSpecular[3] = 1.0f;

        glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

        /* update light direction vector */
        m_lightVec = btVector3(lightDirection[0], lightDirection[1], lightDirection[2]);
        m_lightVec.normalize();
    }

    void setViewport(const int width, const int height)
    {
        glViewport(0, 0, width, height);
        /* camera setting */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMultMatrixf(m_projection);
        glMatrixMode(GL_MODELVIEW);
    }

    void setModelView(const btTransform &modelView)
    {
        float matrix[16], inverse[16];
        modelView.getOpenGLMatrix(matrix);
        modelView.inverse().getOpenGLMatrix(inverse);
        memcpy(m_modelView, matrix, sizeof(m_modelView));
        memcpy(m_modelViewInversed, inverse, sizeof(m_modelViewInversed));
    }

    void setProjection(const float projection[16])
    {
        memcpy(m_projection, projection, sizeof(m_projection));
    }

    void setShadowMapAutoView(const btVector3 &eyePoint,
                              const float radius)
    {
        m_shadowMapAutoViewEyePoint = btVector3(eyePoint);
        m_shadowMapAutoViewRadius = radius;
    }

private:
    /* RendererSceneShadowMap: shadow mapping */
    void renderSceneShadowMap(PMDObject **objects, int16_t *order, int size, Stage *stage)
    {
#ifndef MMDAI_OPENGL_ES1
        int i = 0;
        static GLfloat lightdim[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        static const GLfloat lightblk[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        const float shadowMappingSelfDensity = m_preference->getFloat(kPreferenceShadowMappingSelfDensity);
        const bool shadowMappingLightFirst = m_preference->getBool(kPreferenceShadowMappingLightFirst);
        const bool useCartoonRendering = m_preference->getBool(kPreferenceUseCartoonRendering);

        /* Renderer the full scene */
        /* set model view matrix, as the same as normal Renderering */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMultMatrixf(m_modelView);

        /* Renderer the whole scene */
        if (m_preference->getBool(kPreferenceShadowMappingLightFirst)) {
            /* Renderer light setting, later Renderer only the shadow part with dark setting */
            stage->renderBackground();
            stage->renderFloor();
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                PMDModel *model = object->getModel();
                renderModel(model);
                renderEdge(model);
            }
        } else {
            /* Renderer in dark setting, later Renderer only the non-shadow part with light setting */
            /* light setting for non-toon objects */
            lightdim[0] = lightdim[1] = lightdim[2] = 0.55f - 0.2f * shadowMappingSelfDensity;
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdim);
            glLightfv(GL_LIGHT0, GL_AMBIENT, lightdim);
            glLightfv(GL_LIGHT0, GL_SPECULAR, lightblk);

            /* Renderer the non-toon objects (back, floor, non-toon models) */
            stage->renderBackground();
            stage->renderFloor();
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                PMDModel *model = object->getModel();
                if (model->isToonEnabled())
                    continue;
                renderModel(model);
            }

            /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
            /* so restore the light setting */
            if (useCartoonRendering)
                updateLighting();
            /* Renderer the toon objects */
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                PMDModel *model = object->getModel();
                if (!model->isToonEnabled())
                    continue;
                /* set texture coordinates for shadow mapping */
                model->updateShadowColorTexCoord(shadowMappingSelfDensity);
                /* tell model to Renderer with the shadow corrdinates */
                model->setSelfShadowDrawing(true);
                /* Renderer model and edge */
                renderModel(model);
                renderEdge(model);
                /* disable shadow Renderering */
                model->setSelfShadowDrawing(false);
            }
            if (!useCartoonRendering)
                updateLighting();
        }

        /* Renderer the part clipped by the depth texture */
        /* activate the texture unit for shadow mapping and make it current */
        glActiveTexture(kDepthTextureID);

        /* set texture matrix (note: matrices should be set in reverse order) */
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        /* move the range from [-1,1] to [0,1] */
        glTranslated(0.5, 0.5, 0.5);
        glScaled(0.5, 0.5, 0.5);
        /* multiply the model view matrix when the depth texture was Renderered */
        glMultMatrixd(m_modelView2);
        /* multiply the inverse matrix of current model view matrix */
        glMultMatrixf(m_modelViewInversed);

        /* revert to model view matrix mode */
        glMatrixMode(GL_MODELVIEW);

        /* enable texture mapping with texture coordinate generation */
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R);
        glEnable(GL_TEXTURE_GEN_Q);

        /* bind the depth texture Renderered at the first step */
        glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

        /* depth texture set up was done, now switch current texture unit to default */
        glActiveTexture(kModelTextureID);

        /* set depth func to allow overwrite for the same surface in the following Renderering */
        glDepthFunc(GL_LEQUAL);

        if (shadowMappingLightFirst) {
            /* the area clipped by depth texture by alpha test is dark part */
            glAlphaFunc(GL_GEQUAL, 0.1f);

            /* light setting for non-toon objects */
            lightdim[0] = lightdim[1] = lightdim[2] = 0.55f - 0.2f * shadowMappingSelfDensity;
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdim);
            glLightfv(GL_LIGHT0, GL_AMBIENT, lightdim);
            glLightfv(GL_LIGHT0, GL_SPECULAR, lightblk);

            /* Renderer the non-toon objects (back, floor, non-toon models) */
            stage->renderBackground();
            stage->renderFloor();
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                PMDModel *model = object->getModel();
                if (!model->isToonEnabled())
                    continue;
                renderModel(model);
            }

            /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
            /* so restore the light setting */
            if (useCartoonRendering)
                updateLighting();
            /* Renderer the toon objects */
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                PMDModel *model = object->getModel();
                if (!model->isToonEnabled())
                    continue;
                /* set texture coordinates for shadow mapping */
                model->updateShadowColorTexCoord(shadowMappingSelfDensity);
                /* tell model to Renderer with the shadow corrdinates */
                model->setSelfShadowDrawing(true);
                /* Renderer model and edge */
                renderModel(model);
                /* disable shadow Renderering */
                model->setSelfShadowDrawing(false);
            }
            if (!useCartoonRendering)
                updateLighting();
        } else {
            /* the area clipped by depth texture by alpha test is light part */
            glAlphaFunc(GL_GEQUAL, 0.001f);
            stage->renderBackground();
            stage->renderFloor();
            for (i = 0; i < size; i++) {
                PMDObject *object = objects[order[i]];
                if (!object->isEnable())
                    continue;
                renderModel(object->getModel());
            }
        }

        /* reset settings */
        glDepthFunc(GL_LESS);
        glAlphaFunc(GL_GEQUAL, 0.05f);

        glActiveTexture(kDepthTextureID);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(kModelTextureID);
#else
        (void) objects;
        (void) order;
        (void) size;
        (void) stage;
#endif
    }

    /* RendererScene: Renderer scene */
    void renderScene(PMDObject **objects, int16_t *order, int size, Stage *stage)
    {
        int i = 0;

        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);

        /* set model viwe matrix */
        glLoadIdentity();
        glMultMatrixf(m_modelView);

        /* stage and shadhow */
        /* background */
        stage->renderBackground();
        /* enable stencil */
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, ~0);
        /* make stencil tag true */
        glStencilOp(GL_KEEP, GL_KEEP , GL_REPLACE);
        /* Renderer floor */
        stage->renderFloor();
        /* Renderer shadow stencil */
        glColorMask(0, 0, 0, 0) ;
        glDepthMask(0);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 1, ~0);
        /* increment 1 pixel stencil */
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        /* Render model */
        glDisable(GL_DEPTH_TEST);
        glPushMatrix();
        glMultMatrixf(stage->getShadowMatrix());
        for (i = 0; i < size; i++) {
            PMDObject *object = objects[order[i]];
            if (!object->isEnable())
                continue;
            renderShadow(object->getModel());
        }
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glColorMask(1, 1, 1, 1);
        glDepthMask(1);
        /* if stencil is 2, Renderer shadow with blend on */
        glStencilFunc(GL_EQUAL, 2, ~0);
        glDisable(GL_LIGHTING);
        glColor4f(0.1f, 0.1f, 0.1f, m_preference->getFloat(kPreferenceShadowMappingSelfDensity));
        glDisable(GL_DEPTH_TEST);
        stage->renderFloor();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_LIGHTING);

        /* Render model */
        for (i = 0; i < size; i++) {
            PMDObject *object = objects[order[i]];
            if (!object->isEnable())
                continue;
            PMDModel *model = object->getModel();
            renderModel(model);
            renderEdge(model);
        }
    }

    IPreference *m_preference;
    btVector3 m_lightVec;                  /* light vector for shadow maapping */
    btVector3 m_shadowMapAutoViewEyePoint; /* view point of shadow mapping */
#ifndef MMDAI_OPENGL_ES1
    GLdouble m_modelView2[16];
#endif
    btScalar m_modelView[16];
    btScalar m_modelViewInversed[16];
    btScalar m_projection[16];
    float m_shadowMapAutoViewRadius;       /* radius from view point */

    GLuint m_depthTextureID;
    GLuint m_fboID;
    bool m_shadowMapInitialized;           /* true if initialized */
};

} /* namespace */


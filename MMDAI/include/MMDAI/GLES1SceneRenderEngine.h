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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef MMDAI_GLES1SCENERENDERENGINE_H_
#define MMDAI_GLES1SCENERENDERENGINE_H_

/* headers */
#include <OpenGLES/ES1/gl.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <MMDAI/SceneRenderEngine.h>

class btConvexShape;

namespace MMDAI {

class BulletPhysics;
class Preference;
class PMDBone;
class PMDModel;
class PMDObject;
class PMDTexture;
class Stage;

struct PMDTextureNative {
    GLuint id;
};

struct PMDRenderCacheNative {
    GLuint id;
};

class GLES1SceneRenderEngine : public SceneRenderEngine {
public:
    GLES1SceneRenderEngine(Preference *preference);
    ~GLES1SceneRenderEngine();
    
    PMDModel *allocateModel();
    bool loadModel(PMDModel *model, PMDModelLoader *loader, BulletPhysics *bullet);
    void releaseModel(PMDModel *model);
    PMDMaterial **allocateMaterials(int size);
    void releaseMaterials(PMDMaterial **materials, int size);
    
    void renderRigidBodies(BulletPhysics *bullet);
    void renderBone(PMDBone *bone);
    void renderBones(PMDModel *model);
    void renderModel(PMDModel *model);
    void renderEdge(PMDModel *model);
    void renderShadow(PMDModel *model);
    
    PMDTextureNative *allocateTexture(const unsigned char *data,
                                      const int width,
                                      const int height,
                                      const int components);
    void releaseTexture(PMDTextureNative *native);
    
    void renderModelCached(PMDModel *model,
                           PMDRenderCacheNative **ptr);
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
                           PMDRenderCacheNative **ptr);
    void deleteCache(PMDRenderCacheNative **ptr);
    
    bool setup();
    void initializeShadowMap();
    void setShadowMapping();
    void prerender(PMDObject **objects,
                   int size);
    void render(PMDObject **objects,
                int size,
                Stage *stage);
    int pickModel(PMDObject **objects,
                  int size,
                  int x,
                  int y,
                  int width,
                  int height,
                  double scale,
                  int *allowDropPicked);
    void updateLighting();
    void updateProjectionMatrix(const int width,
                                const int height,
                                const double scale);
    void applyProjectionMatrix(const int width,
                               const int height,
                               const double scale);
    void applyModelViewMatrix();
    void updateModelViewMatrix(const btTransform &transMatrix,
                               const btTransform &transMatrixInv);
    void setShadowMapAutoView(const btVector3 &eyePoint,
                              const float radius);
    void setModelViewMatrix(const btScalar modelView[16]);
    void setProjectionMatrix(const btScalar projection[16]);
    
private:
    void drawCube();
    void drawSphere(int lats, int longs);
    void drawConvex(btConvexShape *shape);
    
    Preference *m_preference;
    btVector3 m_lightVec;                  /* light vector for shadow maapping */
    btVector3 m_shadowMapAutoViewEyePoint; /* view point of shadow mapping */
    btScalar m_rotMatrix[16];     /* current rotation + OpenGL rotation matrix */
    btScalar m_rotMatrixInv[16];  /* current rotation + inverse of OpenGL rotation matrix */
    btScalar m_newModelViewMatrix[16];
    btScalar m_newProjectionMatrix[16];
    float m_modelView[16];
    float m_shadowMapAutoViewRadius;       /* radius from view point */
    
    GLuint m_boxList;
    GLuint m_sphereList;
    GLuint m_depthTextureID;
    GLuint m_fboID;
    bool m_boxListEnabled;
    bool m_sphereListEnabled;
    bool m_enableShadowMapping;            /* true if shadow mapping */
    bool m_overrideModelViewMatrix;
    bool m_overrideProjectionMatrix;
    bool m_shadowMapInitialized;           /* true if initialized */
};

} /* namespace */

#endif


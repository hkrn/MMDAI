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

#ifndef MMDME_PMDMODEL_H_
#define MMDME_PMDMODEL_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/PMDBone.h"
#include "MMDME/PMDTexture.h"
#include "MMDME/PTree.h"

namespace MMDAI {

class BulletPhysics;
class PMDConstraint;
class PMDFace;
class PMDIK;
class PMDMaterial;
class PMDModelLoader;
class PMDRenderEngine;
class PMDRigidBody;

typedef struct {
    float u;
    float v;
} TexCoord;

class PMDModel
{
public:
    static const int  kNSystemTextureFiles = 10;
    static const float kMinBoneWeight;
    static const float kMinFaceWeight;
    static const float kEdgeColorR;
    static const float kEdgeColorG;
    static const float kEdgeColorB;
    static const float kEdgeColorA;

    PMDModel(PMDRenderEngine *engine);
    virtual ~PMDModel();

    bool load(PMDModelLoader *loader, BulletPhysics *bullet);
    PMDBone *getBone(const char *name);
    PMDFace *getFace(const char *name);
    int getChildBoneList(PMDBone **bone, uint16_t boneNum, PMDBone **childBoneList, uint16_t childBoneNumMax);
    void setPhysicsControl(bool flag);
    void release();
    void setEdgeThin(float thin);
    void setToonFlag(bool flag);
    bool getToonFlag() const;
    void setSelfShadowDrawing(bool flag);
    void setEdgeColor(float col[4]);
    void setGlobalAlpha(float alpha);
    PMDBone *getRootBone();
    PMDBone *getCenterBone() const;
    const char *getName() const;
    uint32_t getNumVertex() const;
    uint32_t getNumSurface() const;
    uint32_t getNumMaterial() const;
    uint16_t getNumBone() const;
    uint16_t getNumIK() const;
    uint16_t getNumFace() const;
    uint32_t getNumRigidBody() const;
    uint32_t getNumConstraint() const;
    float getMaxHeight() const;
    const char *getComment() const;
    void updateBone();
    void updateBoneFromSimulation();
    void updateFace();
    void updateSkin();
    void updateToon(btVector3 *light);
    void updateShadowColorTexCoord(float coef);
    float calculateBoundingSphereRange(btVector3 *cpos);
    void smearAllBonesToDefault(float rate);

    PMDBone *getBonesPtr() const;
    const btVector3 *getVerticesPtr() const;
    const btVector3 *getNormalsPtr() const;
    const TexCoord *getTexCoordsPtr() const;
    const btVector3 *getSkinnedVerticesPtr() const;
    const btVector3 *getSkinnedNormalsPtr() const;
    const TexCoord *getToonTexCoordsPtr() const;
    const TexCoord *getToonTexCoordsForSelfShadowPtr() const;
    const btVector3 *getEdgeVerticesPtr() const;
    const uint16_t *getSurfacesPtr() const;
    const uint16_t *getSurfacesForEdgePtr() const;
    const float getGlobalAlpha() const;
    const float *getEdgeColors() const;
    PMDMaterial *getMaterialAt(uint32_t i);
    PMDTexture *getToonTextureAt(uint32_t i);
    const uint32_t getNumSurfaceForEdge() const;
    const bool isSelfShadowEnabled() const;
    const bool hasSingleSphereMap() const;
    const bool hasMultipleSphereMap() const;

private:
    bool parse(PMDModelLoader *loader, BulletPhysics *bullet);
    void initialize();
    void clear();

    char *m_name;
    char *m_comment;
    uint32_t m_numVertex;
    btVector3 *m_vertexList;
    btVector3 *m_normalList;
    TexCoord *m_texCoordList;
    int16_t *m_bone1List;
    int16_t *m_bone2List;
    float *m_boneWeight1;
    bool *m_noEdgeFlag;
    uint32_t m_numSurface;
    uint16_t *m_surfaceList;
    uint32_t m_numMaterial;
    PMDMaterial **m_material;
    uint16_t m_numBone;
    PMDBone *m_boneList;
    uint16_t m_numIK;
    PMDIK *m_IKList;
    uint16_t m_numFace;
    PMDFace *m_faceList;
    uint32_t m_numRigidBody;
    PMDRigidBody *m_rigidBodyList;
    uint32_t m_numConstraint;
    PMDConstraint *m_constraintList;
    PMDTexture m_localToonTexture[kNSystemTextureFiles + 1];
    btTransform *m_boneSkinningTrans;
    btVector3 *m_skinnedVertexList;
    btVector3 *m_skinnedNormalList;
    TexCoord *m_toonTexCoordList;
    btVector3 *m_edgeVertexList;
    uint32_t m_numSurfaceForEdge;
    uint16_t *m_surfaceListForEdge;
    TexCoord *m_toonTexCoordListForShadowMap;
    PMDBone *m_centerBone;
    PMDFace *m_baseFace;
    bool m_hasSingleSphereMap;
    bool m_hasMultipleSphereMap;
    uint16_t m_numRotateBone;
    uint16_t *m_rotateBoneIDList;
    bool *m_IKSimulated;
    bool m_enableSimulation;
    float m_maxHeight;
    bool m_toon;
    float m_globalAlpha;
    float m_edgeOffset;
    bool m_selfShadowDrawing;
    float m_selfShadowDensityCoef;
    float m_edgeColor[4];

    BulletPhysics *m_bulletPhysics;
    PMDBone m_rootBone;
    PTree m_name2bone;
    PTree m_name2face;
    PMDRenderEngine *m_engine;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDModel);
};

} /* namespace */

#endif

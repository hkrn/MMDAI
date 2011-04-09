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
    void updateBone();
    void updateBoneFromSimulation();
    void updateFace();
    void updateSkin();
    void updateToon(btVector3 *light);
    void updateShadowColorTexCoord(float coef);
    float calculateBoundingSphereRange(btVector3 *cpos);
    void smearAllBonesToDefault(float rate);
    int getChildBoneList(PMDBone **bone, uint16_t boneNum, PMDBone **childBoneList, uint16_t childBoneNumMax);
    void setPhysicsControl(bool flag);
    void release();

    inline void setEdgeThin(const float value) {
        m_edgeOffset = value * 0.03f;
    }
    inline void setToonEnable(const bool value) {
        m_toon = value;
    }
    inline const bool isToonEnabled() const {
        return m_toon;
    }
    inline void setSelfShadowDrawing(const bool value) {
        m_selfShadowDrawing = value;
    }
    inline void setEdgeColor(const float col[4]) {
        for (int i = 0; i < 4; i++)
            m_edgeColor[i] = col[i];
    }
    inline void setGlobalAlpha(const float value) {
        m_globalAlpha = value;
    }
    inline PMDBone *getRootBone() {
        return &m_rootBone;
    }
    inline PMDBone *getCenterBone() {
        return m_centerBone;
    }
    inline const char *getName() const {
        return m_name;
    }
    inline const uint32_t countVertices() const {
        return m_numVertex;
    }
    inline const uint32_t countSurfaces() const {
        return m_numSurface;
    }
    inline const uint32_t countMaterials() const {
        return m_numMaterial;
    }
    inline const uint16_t countBones() const {
        return m_numBone;
    }
    inline const uint16_t countIKs() const {
        return m_numIK;
    }
    inline const uint16_t countFaces() const {
        return m_numFace;
    }
    inline const uint32_t countRigidBodies() const {
        return m_numRigidBody;
    }
    inline const uint32_t countConstraints() const {
        return m_numConstraint;
    }
    inline const float getMaxHeight() const {
        return m_maxHeight;
    }
    inline const char *getComment() const {
        return m_comment;
    }

    inline PMDBone *getBonesPtr() const {
        return m_boneList;
    }
    inline const btVector3 *getVerticesPtr() const {
        return m_vertexList;
    }
    inline const btVector3 *getNormalsPtr() const {
        return m_normalList;
    }
    inline const TexCoord *getTexCoordsPtr() const {
        return m_texCoordList;
    }
    inline const btVector3 *getSkinnedVerticesPtr() const {
        return m_skinnedVertexList;
    }
    inline const btVector3 *getSkinnedNormalsPtr() const {
        return m_skinnedNormalList;
    }
    inline const TexCoord *getToonTexCoordsPtr() const {
        return m_toonTexCoordList;
    }
    inline const TexCoord *getToonTexCoordsForSelfShadowPtr() const {
        return m_toonTexCoordListForShadowMap;
    }
    inline const btVector3 *getEdgeVerticesPtr() const {
        return m_edgeVertexList;
    }
    inline const uint16_t *getSurfacesPtr() const {
        return m_surfaceList;
    }
    inline const uint16_t *getSurfacesForEdgePtr() const {
        return m_surfaceListForEdge;
    }
    inline const float getGlobalAlpha() const {
        return m_globalAlpha;
    }
    inline const float *getEdgeColors() const {
        return m_edgeColor;
    }
    inline const uint32_t getNumSurfaceForEdge() const {
        return m_numSurfaceForEdge;
    }
    inline const bool isSelfShadowEnabled() const {
        return m_selfShadowDrawing;
    }
    inline const bool hasSingleSphereMap() const {
        return m_hasSingleSphereMap;
    }
    inline const bool hasMultipleSphereMap() const {
        return m_hasMultipleSphereMap;
    }
    inline PMDMaterial *getMaterialAt(uint32_t i) {
        if ( i >= m_numMaterial)
            return NULL;
        else
            return m_material[i];
    }
    inline PMDTexture *getToonTextureAt(uint32_t i) {
        if (i >= kNSystemTextureFiles + 1)
            return NULL;
        else
            return &m_localToonTexture[i];
    }
    inline const uint8_t countFaceDisplayNames() {
        return m_numFaceDisplayNames;
    }
    inline const int getFaceDisplayNameAt(uint32_t index) {
        if (index >= m_numFaceDisplayNames) {
            return -1;
        }
        else {
            return m_faceDisplayNames[index];
        }
    }
    inline const uint8_t countBoneFrameNames() {
        return m_numBoneFrameNames;
    }
    inline const char *getBoneFrameNameAt(uint32_t index) {
        if (index >= m_numBoneFrameNames) {
            return NULL;
        }
        else {
            return m_boneFrameNames[index];
        }
    }
    inline const uint32_t countBoneDisplayNames() {
        return m_numBoneDisplayNames;
    }
    inline const int getBoneDisplayIndexAt(uint32_t index) {
        if (index >= m_numBoneDisplayNames) {
            return -1;
        }
        else {
            return m_boneDisplayIndices[index];
        }
    }
    inline const int getBoneDisplayNameAt(uint32_t index) {
        if (index >= m_numBoneDisplayNames) {
            return -1;
        }
        else {
            return m_boneDisplayNames[index];
        }
    }
    inline PMDBone *getBoneAt(uint32_t index) {
        if (index >= m_numBone) {
            return NULL;
        }
        else {
            return &m_boneList[index];
        }
    }
    inline PMDFace *getFaceAt(uint32_t index) {
        if (index >= m_numFace) {
            return NULL;
        }
        else {
            return &m_faceList[index];
        }
    }

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

    uint8_t m_numFaceDisplayNames;
    uint16_t *m_faceDisplayNames;
    uint8_t m_numBoneFrameNames;
    char **m_boneFrameNames;
    uint32_t m_numBoneDisplayNames;
    uint16_t *m_boneDisplayIndices;
    uint8_t *m_boneDisplayNames;

    BulletPhysics *m_bulletPhysics;
    PMDBone m_rootBone;
    PTree m_name2bone;
    PTree m_name2face;
    PMDRenderEngine *m_engine;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDModel);
};

} /* namespace */

#endif

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

#ifndef VPVL_PMDMODEL_H_
#define VPVL_PMDMODEL_H_

#include "vpvl/Bone.h"
#include "vpvl/Constraint.h"
#include "vpvl/Face.h"
#include "vpvl/IK.h"
#include "vpvl/Material.h"
#include "vpvl/RigidBody.h"
#include "vpvl/Vertex.h"
#include "LinearMath/btHashMap.h"

namespace vpvl
{

struct PMDModelDataInfo
{
    const uint8_t *basePtr;
    const uint8_t *namePtr;
    const uint8_t *commentPtr;
    const uint8_t *verticesPtr;
    size_t verticesCount;
    const uint8_t *indicesPtr;
    size_t indicesCount;
    const uint8_t *materialsPtr;
    size_t materialsCount;
    const uint8_t *bonesPtr;
    size_t bonesCount;
    const uint8_t *IKsPtr;
    size_t IKsCount;
    const uint8_t *facesPtr;
    size_t facesCount;
    const uint8_t *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    const uint8_t *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    const uint8_t *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    const uint8_t *englishNamePtr;
    const uint8_t *englishCommentPtr;
    const uint8_t *englishBoneNamesPtr;
    const uint8_t *englishFaceNamesPtr;
    const uint8_t *englishBoneFramesPtr;
    const uint8_t *toonTextureNamesPtr;
    const uint8_t *rigidBodiesPtr;
    size_t rigidBodiesCount;
    const uint8_t *constraintsPtr;
    size_t constranitsCount;
};

struct SkinVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texureCoord;
};

class VMDMotion;

typedef struct PMDModelPrivate PMDModelPrivate;

class PMDModel
{
public:
    PMDModel(const uint8_t *data, size_t size);
    ~PMDModel();

    static const uint32_t kBoundingSpherePoints = 1000;
    static const uint32_t kBoundingSpherePointsMax = 20;
    static const uint32_t kBoundingSpherePointsMin = 5;
    static const uint32_t kSystemTextureMax = 11;
    static const float kMinBoneWeight;
    static const float kMinFaceWeight;

    void addMotion(VMDMotion *motion);
    void removeMotion(VMDMotion *motion);
    void updateRootBone();
    void updateMotion(float frameAt);
    void updateSkins();
    float boundingSphereRange(btVector3 &center);
    void smearAllBonesToDefault(float rate);

    bool preparse();
    bool load();

    const uint8_t *name() const {
        return m_name;
    }
    const uint8_t *comment() const {
        return m_comment;
    }
    const uint8_t *englishName() const {
        return m_englishName;
    }
    const uint8_t *englishComment() const {
        return m_englishComment;
    }
    const VertexList &vertices() const {
        return m_vertices;
    }
    const IndexList &indices() const {
        return m_indices;
    }
    const MaterialList &materials() const {
        return m_materials;
    }
    const BoneList &bones() const {
        return m_bones;
    }
    const IKList &IKs() const {
        return m_IKs;
    }
    const FaceList &faces() const {
        return m_faces;
    }
    const RigidBodyList &rigidBodies() const {
        return m_rigidBodies;
    }
    const ConstraintList &constraints() const {
        return m_constraints;
    }
    BoneList *mutableBones() {
        return &m_bones;
    }
    const uint8_t *toonTexture(uint32_t index) const {
        if (index >= kSystemTextureMax)
            return NULL;
        return m_textures[index];
    }
    const Bone &rootBone() const {
        return m_rootBone;
    }
    Bone *mutableRootBone() {
        return &m_rootBone;
    }
    float edgeOffset() const {
        return m_edgeOffset;
    }
    bool isSimulationEnabled() const {
        return m_enableSimulation;
    }
    const btVector3 &lightDirection() const {
        return m_lightDirection;
    }
    const PMDModelDataInfo &result() const {
        return m_result;
    }
    PMDModelPrivate *privateData() const {
        return m_private;
    }
    Bone *findBone(const uint8_t *name) const {
        Bone **ptr = const_cast<Bone **>(m_name2bone.find(btHashString(reinterpret_cast<const char *>(name))));
        return ptr ? *ptr : 0;
    }
    Face *findFace(const uint8_t *name) const {
        Face **ptr = const_cast<Face **>(m_name2face.find(btHashString(reinterpret_cast<const char *>(name))));
        return ptr ? *ptr : 0;
    }
    const uint8_t *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }

    size_t stride() const {
        return sizeof(SkinVertex);
    }
    const void *verticesPointer() const {
        return &m_skinnedVertices[0].position;
    }
    const void *normalsPointer() const {
        return &m_skinnedVertices[0].normal;
    }
    const void *textureCoordsPointer() const {
        return &m_skinnedVertices[0].texureCoord;
    }
    const void *toonTextureCoordsPointer() const {
        return &m_toonTextureCoords[0];
    }
    const void *edgeVerticesPointer() const {
        return &m_edgeVertices[0];
    }
    const uint16_t *indicesPointer() const {
        return m_indicesPointer;
    }
    const uint16_t *edgeIndicesPointer() const {
        return m_edgeIndicesPointer;
    }
    uint32_t edgeIndicesCount() const {
        return m_edgeIndicesCount;
    }

    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }
    void setComment(const uint8_t *value) {
        copyBytesSafe(m_comment, value, sizeof(m_comment));
    }
    void setEnglishName(const uint8_t *value) {
        copyBytesSafe(m_englishName, value, sizeof(m_englishName));
    }
    void setEnglishComment(const uint8_t *value) {
        copyBytesSafe(m_englishComment, value, sizeof(m_englishComment));
    }
    void setToonTextures(const uint8_t *ptr) {
        uint8_t *p = const_cast<uint8_t *>(ptr);
        for (int i = 0; i < 10; i++) {
            copyBytesSafe(m_textures[i], p, sizeof(m_textures[i]));
            p += sizeof(m_textures[i]);
        }
    }
    void setLightDirection(const btVector3 &value) {
        m_lightDirection = value.normalized();
    }
    void setEdgeOffset(float value) {
        m_edgeOffset = value * 0.03f;
    }
    void setEnableSimulation(bool value) {
        m_enableSimulation = value;
    }
    void setPrivateData(PMDModelPrivate *value) {
        m_private = value;
    }

private:
    void parseHeader();
    void parseVertices();
    void parseIndices();
    void parseMatrials();
    void parseBones();
    void parseIKs();
    void parseFaces();
    void parseFaceDisplayNames();
    void parseBoneDisplayNames();
    void parseEnglishDisplayNames();
    void parseToonTextureNames();
    void parseRigidBodies();
    void parseConstraints();
    void prepare();
    void updateAllBones();
    void updateBoneFromSimulation();
    void updateAllFaces();
    void updateShadowTextureCoords(float coef);
    void updateSkinVertices();
    void updateToon(const btVector3 &lightDirection);
    void updateIndices();

    uint8_t m_name[20];
    uint8_t m_comment[256];
    uint8_t m_englishName[20];
    uint8_t m_englishComment[256];
    uint8_t m_textures[10][100];
    VertexList m_vertices;
    IndexList m_indices;
    MaterialList m_materials;
    BoneList m_bones;
    IKList m_IKs;
    FaceList m_faces;
    RigidBodyList m_rigidBodies;
    ConstraintList m_constraints;
    Bone m_rootBone;
    Face *m_baseFace;
    PMDModelDataInfo m_result;
    btHashMap<btHashString, Bone *> m_name2bone;
    btHashMap<btHashString, Face *> m_name2face;
    btAlignedObjectArray<VMDMotion *> m_motions;
    btAlignedObjectArray<btTransform> m_skinningTransform;
    btAlignedObjectArray<btVector3> m_edgeVertices;
    btAlignedObjectArray<btVector3> m_toonTextureCoords;
    btAlignedObjectArray<btVector3> m_shadowTextureCoords;
    BoneList m_rotatedBones;
    btAlignedObjectArray<bool> m_isIKSimulated;
    SkinVertex *m_skinnedVertices;
    PMDModelPrivate *m_private;
    uint16_t *m_indicesPointer;
    uint16_t *m_edgeIndicesPointer;
    uint32_t m_edgeIndicesCount;
    btVector3 m_lightDirection;
    const size_t m_size;
    const uint8_t *m_data;
    uint32_t m_boundingSphereStep;
    float m_edgeOffset;
    float m_selfShadowDensityCoef;
    bool m_enableSimulation;
};

}

#endif

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#include "vpvl/vpvl.h"
#include "LinearMath/btHashMap.h"

namespace vpvl
{

struct PMDModelDataInfo
{
    const char *basePtr;
    const char *namePtr;
    const char *commentPtr;
    const char *verticesPtr;
    size_t verticesCount;
    const char *indicesPtr;
    size_t indicesCount;
    const char *materialsPtr;
    size_t materialsCount;
    const char *bonesPtr;
    size_t bonesCount;
    const char *IKsPtr;
    size_t IKsCount;
    const char *facesPtr;
    size_t facesCount;
    const char *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    const char *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    const char *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    const char *englishNamePtr;
    const char *englishCommentPtr;
    const char *toonTextureNamesPtr;
    const char *rigidBodiesPtr;
    size_t rigidBodiesCount;
    const char *constraintsPtr;
    size_t constranitsCount;
};

struct SkinVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texureCoord;
};

typedef struct PMDModelPrivate PMDModelPrivate;

class PMDModel
{
public:
    PMDModel(const char *data, size_t size);
    ~PMDModel();

    static const uint32_t kBoundingSpherePoints = 1000;
    static const uint32_t kBoundingSpherePointsMax = 20;
    static const uint32_t kBoundingSpherePointsMin = 5;
    static const uint32_t kSystemTextureMax = 11;
    static const float kMinBoneWeight;
    static const float kMinFaceWeight;

    void updateRootBone();
    void updateMotion();
    void updateSkins();
    float boundingSphereRange(btVector3 &center);
    void smearAllBonesToDefault(float rate);

    bool preparse();
    bool load();

    const char *name() const {
        return m_name;
    }
    const char *comment() const {
        return m_comment;
    }
    const char *englishName() const {
        return m_englishName;
    }
    const char *englishComment() const {
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
    const char *toonTexture(uint32_t index) const {
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
    Bone *findBone(const char *name) const {
        return *m_name2bone.find(btHashString(name));
    }
    Face *findFace(const char *name) const {
        return *m_name2face.find(btHashString(name));
    }
    const char *data() const {
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

    void setName(const char *value) {
        stringCopySafe(m_name, value, sizeof(m_name));
    }
    void setComment(const char *value) {
        stringCopySafe(m_comment, value, sizeof(m_comment));
    }
    void setEnglishName(const char *value) {
        stringCopySafe(m_englishName, value, sizeof(m_englishName));
    }
    void setEnglishComment(const char *value) {
        stringCopySafe(m_englishComment, value, sizeof(m_englishComment));
    }
    void setToonTextures(const char *ptr) {
        char *p = const_cast<char *>(ptr);
        for (int i = 0; i < 10; i++) {
            stringCopySafe(m_textures[i], p, sizeof(m_textures[i]));
            p += 100;
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

    char m_name[20];
    char m_comment[256];
    char m_englishName[20];
    char m_englishComment[256];
    char m_textures[10][100];
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
    btAlignedObjectArray<btTransform> m_skinningTransform;
    btAlignedObjectArray<btVector3> m_edgeVertices;
    btAlignedObjectArray<btVector3> m_toonTextureCoords;
    btAlignedObjectArray<btVector3> m_shadowTextureCoords;
    btAlignedObjectArray<uint16_t> m_rotatedBones;
    btAlignedObjectArray<bool> m_isIKSimulated;
    SkinVertex *m_skinnedVertices;
    PMDModelPrivate *m_private;
    uint16_t *m_indicesPointer;
    uint16_t *m_edgeIndicesPointer;
    uint32_t m_edgeIndicesCount;
    btVector3 m_lightDirection;
    const size_t m_size;
    const char *m_data;
    uint32_t m_boundingSphereStep;
    float m_edgeOffset;
    float m_selfShadowDensityCoef;
    bool m_enableSimulation;
};

}

#endif

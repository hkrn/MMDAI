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

class btDiscreteDynamicsWorld;

namespace vpvl
{

class VMDMotion;
typedef struct PMDModelUserData PMDModelUserData;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * PMDModel class represents a Polygon Model Data object, 3D model object.
 */

class VPVL_EXPORT PMDModel
{
public:
    typedef struct SkinVertex SkinVertex;
    typedef struct State State;

    /**
      * Type of parsing errors.
      */
    enum Error
    {
        kNoError,
        kInvalidHeaderError,
        kInvalidSignatureError,
        kInvalidVersionError,
        kVerticesSizeError,
        kVerticesError,
        kIndicesSizeError,
        kIndicesError,
        kMaterialsSizeError,
        kMaterialsError,
        kBonesSizeError,
        kBonesError,
        kIKsSizeError,
        kIKsError,
        kFacesSizeError,
        kFacesError,
        kFacesForDisplaySizeError,
        kFacesForDisplayError,
        kBoneFrameNamesSizeError,
        kBoneFrameNamesError,
        kBoneDisplayNamesSizeError,
        kBoneDisplayNamesError,
        kEnglishNamesError,
        kExtraTextureNamesError,
        kRigidBodiesSizeError,
        kRigidBodiesError,
        kConstraintsSizeError,
        kConstraintsError,
        kMaxErrors
    };

    /**
      * Type of stride to get stride length.
      */
    enum StrideType
    {
        kVerticesStride,
        kEdgeVerticesStride,
        kNormalsStride,
        kTextureCoordsStride,
        kToonTextureStride,
        kIndicesStride,
        kEdgeIndicesStride
    };

    struct DataInfo
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
        const uint8_t *facesForUIPtr;
        size_t facesForUICount;
        const uint8_t *boneCategoryNamesPtr;
        size_t boneCategoryNamesCount;
        const uint8_t *bonesForUIPtr;
        size_t bonesForUICount;
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

    PMDModel();
    ~PMDModel();

    static const uint32_t kBoundingSpherePoints = 1000;
    static const uint32_t kBoundingSpherePointsMax = 20;
    static const uint32_t kBoundingSpherePointsMin = 5;
    static const uint32_t kSystemTextureMax = 11;
    static const uint32_t kNameSize = 20;
    static const uint32_t kDescriptionSize = 256;
    static const uint32_t kBoneCategoryNameSize = 50;
    static const float kMinBoneWeight;
    static const float kMinFaceWeight;

    void addMotion(VMDMotion *motion);
    void joinWorld(btDiscreteDynamicsWorld *world);
    void leaveWorld(btDiscreteDynamicsWorld *world);
    void removeMotion(VMDMotion *motion);
    void resetMotion();
    bool isMotionReached(float atEnd);
    void seekMotion(float frameIndex);
    void updateRootBone();
    void advanceMotion(float deltaFrame);
    void updateSkins();
    void updateImmediate();
    float boundingSphereRange(btVector3 &center);
    void resetAllBones();
    void resetAllFaces();
    void smearAllBonesToDefault(float rate);
    void smearAllFacesToDefault(float rate);
    void discardState(State *&state) const;
    State *saveState() const;
    bool restoreState(State *state);

    size_t stride(StrideType type) const;
    const void *verticesPointer() const;
    const void *normalsPointer() const;
    const void *textureCoordsPointer() const;
    const void *toonTextureCoordsPointer() const;
    const void *edgeVerticesPointer() const;

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    bool load(const uint8_t *data, size_t size);

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
    const FaceList &facesForUI() const {
        return m_facesForUI;
    }
    const Array<uint8_t *> &boneCategoryNames() const {
        return m_boneCategoryNames;
    }
    const Array<BoneList *> &bonesForUI() const {
        return m_bonesForUI;
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
    const btVector3 &lightPosition() const {
        return m_lightPosition;
    }
    PMDModelUserData *userData() const {
        return m_userData;
    }
    Bone *findBone(const uint8_t *name) const {
        const HashString key(reinterpret_cast<const char *>(name));
        Bone **ptr = const_cast<Bone **>(m_name2bone.find(key));
        return ptr ? *ptr : 0;
    }
    Face *findFace(const uint8_t *name) const {
        const HashString key(reinterpret_cast<const char *>(name));
        Face **ptr = const_cast<Face **>(m_name2face.find(key));
        return ptr ? *ptr : 0;
    }
    Error error() const {
        return m_error;
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
    void setLightPosition(const btVector3 &value) {
        m_lightPosition = value.normalized();
    }
    void setEdgeOffset(float value) {
        m_edgeOffset = value * 0.03f;
    }
    void setEnableSimulation(bool value) {
        m_enableSimulation = value;
    }
    void setUserData(PMDModelUserData *value) {
        m_userData = value;
    }

private:
    void parseHeader(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseMatrials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseIKs(const DataInfo &info);
    void parseFaces(const DataInfo &info);
    void parseFacesForUI(const DataInfo &info);
    void parseBoneCategoryNames(const DataInfo &info);
    void parseBonesForUI(const DataInfo &info);
    void parseEnglishDisplayNames(const DataInfo &info);
    void parseToonTextureNames(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseConstraints(const DataInfo &info);
    void prepare();
    void release();
    void sortBones();
    void updateAllBones();
    void updateBoneFromSimulation();
    void updateAllFaces();
    void updateShadowTextureCoords(float coef);
    void updateSkinVertices();
    void updateToon(const btVector3 &lightPosition);
    void updateIndices();

    uint8_t m_name[kNameSize];
    uint8_t m_comment[kDescriptionSize];
    uint8_t m_englishName[kNameSize];
    uint8_t m_englishComment[kDescriptionSize];
    uint8_t m_textures[10][100];
    VertexList m_vertices;
    IndexList m_indices;
    MaterialList m_materials;
    BoneList m_bones;
    IKList m_IKs;
    FaceList m_faces;
    FaceList m_facesForUI;
    RigidBodyList m_rigidBodies;
    ConstraintList m_constraints;
    Bone m_rootBone;
    Face *m_baseFace;
    Hash<HashString, Bone *> m_name2bone;
    Hash<HashString, Face *> m_name2face;
    Array<VMDMotion *> m_motions;
    Array<uint8_t *> m_boneCategoryNames;
    Array<btTransform> m_skinningTransform;
    Array<btVector3> m_edgeVertices;
    Array<btVector3> m_toonTextureCoords;
    Array<btVector3> m_shadowTextureCoords;
    Array<BoneList *> m_bonesForUI;
    BoneList m_rotatedBones;
    Bone **m_orderedBones;
    Array<bool> m_isIKSimulated;
    SkinVertex *m_skinnedVertices;
    btDiscreteDynamicsWorld *m_world;
    PMDModelUserData *m_userData;
    uint16_t *m_indicesPointer;
    uint16_t *m_edgeIndicesPointer;
    uint32_t m_edgeIndicesCount;
    btVector3 m_lightPosition;
    Error m_error;
    uint32_t m_boundingSphereStep;
    float m_edgeOffset;
    float m_selfShadowDensityCoef;
    bool m_enableSimulation;

    VPVL_DISABLE_COPY_AND_ASSIGN(PMDModel)
};

}

#endif

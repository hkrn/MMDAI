/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_PMX_MODEL_H_
#define VPVL2_PMX_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IString.h"

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Model class represents a morph of a Polygon Model Extended object.
 */

class Bone;
class Label;
class Joint;
class Material;
class Morph;
class RigidBody;
class Vertex;

class VPVL2_API Model : public IModel
{
public:
    struct SkinnedVertex;

    enum StrideType {
        kVertexStride,
        kNormalStride,
        kTexCoordStride,
        kEdgeSizeStride,
        kToonCoordStride,
        kEdgeVertexStride,
        kVertexIndexStride,
        kBoneIndexStride,
        kBoneWeightStride,
        kUVA1Stride,
        kUVA2Stride,
        kUVA3Stride,
        kUVA4Stride,
        kIndexStride
    };

    struct DataInfo
    {
        IEncoding *encoding;
        IString::Codec codec;
        ErrorType error;
        uint8_t *basePtr;
        uint8_t *namePtr;
        size_t additionalUVSize;
        size_t vertexIndexSize;
        size_t textureIndexSize;
        size_t materialIndexSize;
        size_t boneIndexSize;
        size_t morphIndexSize;
        size_t rigidBodyIndexSize;
        size_t nameSize;
        uint8_t *englishNamePtr;
        size_t englishNameSize;
        uint8_t *commentPtr;
        size_t commentSize;
        uint8_t *englishCommentPtr;
        size_t englishCommentSize;
        uint8_t *verticesPtr;
        size_t verticesCount;
        uint8_t *indicesPtr;
        size_t indicesCount;
        uint8_t *texturesPtr;
        size_t texturesCount;
        uint8_t *materialsPtr;
        size_t materialsCount;
        uint8_t *bonesPtr;
        size_t bonesCount;
        uint8_t *morphsPtr;
        size_t morphsCount;
        uint8_t *labelsPtr;
        size_t labelsCount;
        uint8_t *rigidBodiesPtr;
        size_t rigidBodiesCount;
        uint8_t *jointsPtr;
        size_t jointsCount;
        uint8_t *endPtr;
    };

    /**
     * Constructor
     */
    Model(IEncoding *encoding);
    ~Model();

    static size_t strideOffset(StrideType type);
    static size_t strideSize(StrideType type);

    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;

    void resetVertices();
    void resetMotionState();
    void performUpdate(const Vector3 &cameraPosition, const Vector3 &lightDirection);
    void joinWorld(btDiscreteDynamicsWorld *world);
    void leaveWorld(btDiscreteDynamicsWorld *world);
    IBone *findBone(const IString *value) const;
    IMorph *findMorph(const IString *value) const;
    int count(ObjectType value) const;
    void getBones(Array<IBone *> &value) const;
    void getMorphs(Array<IMorph *> &value) const;
    void getLabels(Array<ILabel *> &value) const;
    void getBoundingBox(Vector3 &min, Vector3 &max) const;
    void getBoundingSphere(Vector3 &center, Scalar &radius) const;
    IndexType indexType() const;

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    void setVisible(bool value);

    const void *vertexPtr() const;
    const void *indicesPtr() const;
    Scalar edgeScaleFactor(const Vector3 &cameraPosition) const;

    Type type() const { return kPMX; }
    const Array<Vertex *> &vertices() const { return m_vertices; }
    const Array<int> &indices() const { return m_indices; }
    const Array<IString *> &textures() const { return m_textures; }
    const Array<Material *> &materials() const { return m_materials; }
    const Array<Bone *> &bones() const { return m_bones; }
    const Array<Morph *> &morphs() const { return m_morphs; }
    const Array<Label *> &labels() const { return m_labels;  }
    const Array<RigidBody *> &rigidBodies() const { return m_rigidBodies; }
    const Array<Joint *> &joints() const { return m_joints; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *comment() const { return m_comment; }
    const IString *englishComment() const { return m_englishComment; }
    ErrorType error() const { return m_info.error; }
    bool isVisible() const { return m_visible && !btFuzzyZero(m_opacity); }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);

    const Vector3 &position() const { return m_position; }
    const Quaternion &rotation() const { return m_rotation; }
    const Scalar &opacity() const { return m_opacity; }
    const Scalar &scaleFactor() const { return m_scaleFactor; }
    const Vector3 &edgeColor() const { return kZeroV3; }
    const Scalar &edgeWidth() const { return m_edgeWidth; }
    IModel *parentModel() const { return 0; }
    IBone *parentBone() const { return 0; }
    void setPosition(const Vector3 &value) { m_position = value; }
    void setRotation(const Quaternion &value) { m_rotation = value; }
    void setOpacity(const Scalar &value) { m_opacity = value; }
    void setScaleFactor(const Scalar &value) { m_scaleFactor = value; }
    void setEdgeColor(const Vector3 & /* value */) {}
    void setEdgeWidth(const Scalar &value) { m_edgeWidth = value; }
    void setParentModel(IModel * /* value */) {}
    void setParentBone(IBone * /* value */) {}

    typedef btAlignedObjectArray<int> BoneIndices;
    typedef btAlignedObjectArray<BoneIndices> MeshBoneIndices;
    typedef btAlignedObjectArray<Transform> MeshLocalTransforms;
    typedef Array<Scalar *> MeshMatrices;
    struct SkinningMeshes {
        MeshBoneIndices bones;
        MeshLocalTransforms transforms;
        MeshMatrices matrices;
        BoneIndices bdef1;
        BoneIndices bdef2;
        BoneIndices bdef4;
        BoneIndices sdef;
        ~SkinningMeshes() { matrices.releaseArrayAll(); }
    };
    void getSkinningMesh(SkinningMeshes &meshes) const;
    void updateSkinningMesh(SkinningMeshes &meshes) const;
    void setSkinningEnable(bool value);

private:
    struct IndexBuffer;

    void release();
    void parseNamesAndComments(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseTextures(const DataInfo &info);
    void parseMaterials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseMorphs(const DataInfo &info);
    void parseLabels(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseJoints(const DataInfo &info);

    btDiscreteDynamicsWorld *m_worldRef;
    IEncoding *m_encodingRef;
    Array<Vertex *> m_vertices;
    Array<int> m_indices;
    Array<IString *> m_textures;
    Array<Material *> m_materials;
    Array<Bone *> m_bones;
    Array<Bone *> m_BPSOrderedBones;
    Array<Bone *> m_APSOrderedBones;
    Array<Morph *> m_morphs;
    Array<Label *> m_labels;
    Array<RigidBody *> m_rigidBodies;
    Array<Joint *> m_joints;
    Hash<HashString, IBone *> m_name2boneRefs;
    Hash<HashString, IMorph *> m_name2morphRefs;
    SkinnedVertex *m_skinnedVertices;
    IndexBuffer *m_indexBuffer;
    IString *m_name;
    IString *m_englishName;
    IString *m_comment;
    IString *m_englishComment;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    Scalar m_edgeWidth;
    DataInfo m_info;
    bool m_visible;
    bool m_enableSkinning;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Model)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif


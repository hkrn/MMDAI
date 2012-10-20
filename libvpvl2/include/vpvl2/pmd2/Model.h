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

#ifndef VPVL2_PMD2_MODEL_H_
#define VPVL2_PMD2_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IModel.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd2
{

class Bone;
class Joint;
class Label;
class Material;
class Morph;
class RigidBody;
class Vertex;

class VPVL2_API Model : public IModel
{
public:
    static const int kNameSize = 20;
    static const int kCommentSize = 256;
    static const int kCustomToonTextureNameSize = 100;
    static const int kMaxCustomToonTextures = 10;
    static const uint8_t *const kFallbackToonTextureName;

    struct DataInfo {
        IEncoding *encoding;
        ErrorType error;
        uint8_t *basePtr;
        uint8_t *namePtr;
        uint8_t *commentPtr;
        uint8_t *verticesPtr;
        size_t verticesCount;
        uint8_t *indicesPtr;
        size_t indicesCount;
        uint8_t *materialsPtr;
        size_t materialsCount;
        uint8_t *bonesPtr;
        size_t bonesCount;
        uint8_t *IKConstraintsPtr;
        size_t IKConstraintsCount;
        uint8_t *morphsPtr;
        size_t morphsCount;
        uint8_t *morphLabelsPtr;
        size_t morphLabelsCount;
        uint8_t *boneCategoryNamesPtr;
        size_t boneCategoryNamesCount;
        uint8_t *boneLabelsPtr;
        size_t boneLabelsCount;
        uint8_t *englishNamePtr;
        uint8_t *englishCommentPtr;
        uint8_t *englishBoneNamesPtr;
        uint8_t *englishFaceNamesPtr;
        uint8_t *englishBoneFramesPtr;
        uint8_t *customToonTextureNamesPtr;
        uint8_t *rigidBodiesPtr;
        size_t rigidBodiesCount;
        uint8_t *jointsPtr;
        size_t jointsCount;
    };

    Model(IEncoding *encodingRef);
    ~Model();

    Type type() const { return kPMD; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *comment() const { return m_comment; }
    const IString *englishComment() const { return m_englishComment; }
    bool isVisible() const { return m_visible && !btFuzzyZero(m_opacity); }
    ErrorType error() const;
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;
    void resetVertices();
    void resetMotionState();
    void performUpdate();
    void joinWorld(btDiscreteDynamicsWorld *world);
    void leaveWorld(btDiscreteDynamicsWorld *world);
    IBone *findBone(const IString *value) const;
    IMorph *findMorph(const IString *value) const;
    int count(ObjectType value) const;
    void getBoneRefs(Array<IBone *> &value) const;
    void getLabelRefs(Array<ILabel *> &value) const;
    void getMaterialRefs(Array<IMaterial *> &value) const;
    void getMorphRefs(Array<IMorph *> &value) const;
    void getVertexRefs(Array<IVertex *> &value) const;
    Scalar edgeScaleFactor(const Vector3 &cameraPosition) const;
    const Vector3 &position() const { return m_position; }
    const Quaternion &rotation() const { return m_rotation; }
    const Scalar &opacity() const { return m_opacity; }
    const Scalar &scaleFactor() const { return m_scaleFactor; }
    const Vector3 &edgeColor() const { return m_edgeColor; }
    const Scalar &edgeWidth() const { return m_edgeWidth; }
    IModel *parentModel() const { return 0; }
    IBone *parentBone() const { return 0; }
    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);
    void setPosition(const Vector3 &value);
    void setRotation(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setEdgeColor(const Vector3 &value);
    void setEdgeWidth(const Scalar &value);
    void setParentModel(IModel * /* value */) {}
    void setParentBone(IBone * /* value */) {}

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    void setVisible(bool value);

    const Array<Vertex *> &vertices() const { return m_vertices; }
    const Array<int> &indices() const { return m_indices; }
    const Array<Material *> &materials() const { return m_materials; }
    const Array<Bone *> &bones() const { return m_bones; }
    const Array<Morph *> &morphs() const { return m_morphs; }
    const Array<Label *> &labels() const { return m_labels; }
    const Array<RigidBody *> &rigidBodies() const { return m_rigidBodies; }
    const Array<Joint *> &joints() const { return m_joints; }

    void getIndexBuffer(IIndexBuffer *&indexBuffer) const;
    void getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const;
    void getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer,
                                const IIndexBuffer *indexBuffer) const;
    void getMatrixBuffer(IMatrixBuffer *&matrixBuffer,
                         IDynamicVertexBuffer *dynamicBuffer,
                         const IIndexBuffer *indexBuffer) const;

private:
    void release();
    void parseNamesAndComments(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseMaterials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseIKConstraints(const DataInfo &info);
    void parseMorphs(const DataInfo &info);
    void parseLabels(const DataInfo &info);
    void parseCustomToonTextures(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseJoints(const DataInfo &info);

    btDiscreteDynamicsWorld *m_worldRef;
    IEncoding *m_encodingRef;
    IString *m_name;
    IString *m_englishName;
    IString *m_comment;
    IString *m_englishComment;
    Array<Vertex *> m_vertices;
    Array<int> m_indices;
    Array<Material *> m_materials;
    Array<Bone *> m_bones;
    Array<Morph *> m_morphs;
    Array<Label *> m_labels;
    Array<RigidBody *> m_rigidBodies;
    Array<Joint *> m_joints;
    Array<IString *> m_customToonTextures;
    Array<Bone *> m_sortedBones;
    Hash<HashString, IBone *> m_name2boneRefs;
    Hash<HashString, IMorph *> m_name2morphRefs;
    DataInfo m_info;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    Vector3 m_edgeColor;
    Scalar m_edgeWidth;
    bool m_visible;
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif

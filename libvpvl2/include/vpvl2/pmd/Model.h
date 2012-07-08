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

#ifndef VPVL2_PMD_MODEL_H_
#define VPVL2_PMD_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IModel.h"

#include "vpvl/PMDModel.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd
{

class VPVL2_API Model : public IModel
{
public:
    Model(IEncoding *encoding);
    ~Model();

    Type type() const { return kPMD; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *comment() const { return m_comment; }
    const IString *englishComment() const { return m_englishComment; }
    bool isVisible() const { return m_model.isVisible() && !btFuzzyZero(m_opacity); }
    Error error() const { return kNoError; }
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;
    void resetVertices();
    void performUpdate(const Vector3 &cameraPosition, const Vector3 &lightDirection);
    void joinWorld(btDiscreteDynamicsWorld *world);
    void leaveWorld(btDiscreteDynamicsWorld *world);
    IBone *findBone(const IString *value) const;
    IMorph *findMorph(const IString *value) const;
    int count(Object value) const;
    void getBones(Array<IBone *> &value) const { value.copy(m_bones); }
    void getMorphs(Array<IMorph *> &value) const { value.copy(m_morphs); }
    void getLabels(Array<ILabel *> &value) const { value.copy(m_labels); }
    void getBoundingBox(Vector3 &min, Vector3 &max) const;
    void getBoundingSphere(Vector3 &center, Scalar &radius) const;
    Scalar edgeScaleFactor(const Vector3 &cameraPosition) const;
    const Vector3 &position() const { return m_model.positionOffset(); }
    const Quaternion &rotation() const { return m_model.rotationOffset(); }
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

    vpvl::PMDModel *ptr() { return &m_model; }

    typedef btAlignedObjectArray<int> BoneIndices;
    typedef btAlignedObjectArray<Vector4> VertexBoneIndicesAndWeights;
    typedef btAlignedObjectArray<BoneIndices> MeshBoneIndices;
    typedef btAlignedObjectArray<VertexBoneIndicesAndWeights> MeshVertexBoneIndicesAndWeights;
    typedef btAlignedObjectArray<Transform> MeshLocalTransforms;
    typedef Array<Scalar *> MeshMatrices;
    struct SkinningMeshes {
        MeshBoneIndices bones;
        MeshLocalTransforms transforms;
        MeshMatrices matrices;
        ~SkinningMeshes() { matrices.releaseArrayAll(); }
    };
    void getSkinningMeshes(SkinningMeshes &meshes) const;
    void updateSkinningMeshes(SkinningMeshes &meshes) const;
    void overrideEdgeVerticesOffset();
    void setSkinnningEnable(bool value);

private:
    IEncoding *m_encoding;
    IString *m_name;
    IString *m_englishName;
    IString *m_comment;
    IString *m_englishComment;
    vpvl::PMDModel m_model;
    Array<IBone *> m_bones;
    Array<IMorph *> m_morphs;
    Array<ILabel *> m_labels;
    Hash<HashString, IBone *> m_name2bones;
    Hash<HashString, IMorph *> m_name2morphs;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    Vector3 m_edgeColor;
    Scalar m_edgeWidth;
    bool m_enableSkinning;
};

}
}

#endif

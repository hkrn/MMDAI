/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#pragma once
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

class Bone;

class VPVL2_API Model : public IModel
{
public:
    Model(IEncoding *encoding);
    ~Model();

    Type type() const { return kPMDModel; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *comment() const { return m_comment; }
    const IString *englishComment() const { return m_englishComment; }
    bool isVisible() const { return m_model.isVisible() && !btFuzzyZero(m_opacity); }
    bool isPhysicsEnabled() const { return m_enablePhysics; }
    ErrorType error() const { return kNoError; }
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data, size_t &written) const;
    size_t estimateSize() const;
    void resetMotionState(btDiscreteDynamicsWorld *worldRef);
    void performUpdate();
    void joinWorld(btDiscreteDynamicsWorld *worldRef);
    void leaveWorld(btDiscreteDynamicsWorld *worldRef);
    IBone *findBoneRef(const IString *value) const;
    IMorph *findMorphRef(const IString *value) const;
    int count(ObjectType value) const;
    void getBoneRefs(Array<IBone *> &value) const { value.copy(m_bones); }
    void getJointRefs(Array<IJoint *> & /* value */) const {}
    void getLabelRefs(Array<ILabel *> &value) const { value.copy(m_labels); }
    void getMaterialRefs(Array<IMaterial *> &value) const { value.copy(m_materials); }
    void getMorphRefs(Array<IMorph *> &value) const { value.copy(m_morphs); }
    void getRigidBodyRefs(Array<IRigidBody *> & /* value */) const {}
    void getTextureRefs(Array<const IString *> & /* value */) const {}
    void getVertexRefs(Array<IVertex *> &value) const { value.copy(m_vertices); }
    void getIndices(Array<int> &value) const;
    void getBoundingBox(Vector3 &min, Vector3 &max) const;
    void getBoundingSphere(Vector3 &center, Scalar &radius) const;
    IModel::IndexBuffer::Type indexType() const { return IModel::IndexBuffer::kIndex16; }
    IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 &cameraPosition) const;
    Vector3 worldPosition() const { return m_position; }
    Quaternion worldRotation() const { return m_rotation; }
    Scalar opacity() const { return m_opacity; }
    Scalar scaleFactor() const { return m_scaleFactor; }
    Vector3 edgeColor() const { return m_edgeColor; }
    Scalar edgeWidth() const { return m_edgeWidth; }
    Scene *parentSceneRef() const { return m_parentSceneRef; }
    IModel *parentModelRef() const { return m_parentModelRef; }
    IBone *parentBoneRef() const { return m_parentBoneRef; }
    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);
    void setWorldPosition(const Vector3 &value);
    void setWorldRotation(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setEdgeColor(const Vector3 &value);
    void setEdgeWidth(const Scalar &value);
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel *value);
    void setParentBoneRef(IBone *value);
    void setVisible(bool value);
    void setPhysicsEnable(bool value);

    void getIndexBuffer(IndexBuffer *&indexBuffer) const;
    void getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const;
    void getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer, const IndexBuffer *indexBuffer) const;
    void getMatrixBuffer(MatrixBuffer *&matrixBuffer, DynamicVertexBuffer *dynamicBuffer, const IndexBuffer *indexBuffer) const;
    void setAabb(const Vector3 &min, const Vector3 &max);
    void getAabb(Vector3 &min, Vector3 &max) const;
    void setSkinnningEnable(bool value);

    float32_t version() const;
    void setVersion(float32_t value);
    IBone *createBone();
    IJoint *createJoint();
    ILabel *createLabel();
    IMaterial *createMaterial();
    IMorph *createMorph();
    IRigidBody *createRigidBody();
    IVertex *createVertex();
    IBone *findBoneRefAt(int value) const;
    IJoint *findJointRefAt(int value) const;
    ILabel *findLabelRefAt(int value) const;
    IMaterial *findMaterialRefAt(int value) const;
    IMorph *findMorphRefAt(int value) const;
    IRigidBody *findRigidBodyRefAt(int value) const;
    IVertex *findVertexRefAt(int value) const;
    void setIndices(const Array<int> &value);
    void addBone(IBone *value);
    void addJoint(IJoint *value);
    void addLabel(ILabel *value);
    void addMaterial(IMaterial *value);
    void addMorph(IMorph *value);
    void addRigidBody(IRigidBody *value);
    void addVertex(IVertex *value);
    void removeBone(IBone *value);
    void removeJoint(IJoint *value);
    void removeLabel(ILabel *value);
    void removeMaterial(IMaterial *value);
    void removeMorph(IMorph *value);
    void removeRigidBody(IRigidBody *value);
    void removeVertex(IVertex *value);

    vpvl::PMDModel *reference() const { return &m_model; }
    const Array<IBone *> &bones() const { return m_bones; }
    const Array<ILabel *> &labels() const { return m_labels; }
    const Array<IMaterial *> &materials() const { return m_materials; }
    const Array<IMorph *> &morphs() const { return m_morphs; }
    const Array<IVertex *> &vertices() const { return m_vertices; }

private:
    void loadBones(Hash<HashPtr, Bone *> &bone2bone);
    void loadIKEffectors(const Hash<HashPtr, Bone *> &bone2bone);
    void loadLabels(const Hash<HashPtr, Bone *> &bone2bone);
    void loadMaterials();
    void loadMorphs();
    void loadVertices();

    mutable vpvl::PMDModel m_model;
    IEncoding *m_encodingRef;
    IString *m_name;
    IString *m_englishName;
    IString *m_comment;
    IString *m_englishComment;
    Scene *m_parentSceneRef;
    IModel *m_parentModelRef;
    IBone *m_parentBoneRef;
    PointerArray<IBone> m_bones;
    PointerArray<ILabel> m_labels;
    PointerArray<IMaterial> m_materials;
    PointerArray<IMorph> m_morphs;
    PointerArray<IVertex> m_vertices;
    PointerArray<vpvl::Bone> m_createdBones;
    PointerArray<vpvl::Material> m_createdMaterials;
    PointerArray<vpvl::Face> m_createdMorphs;
    PointerArray<vpvl::Vertex> m_createdVertices;
    Hash<HashString, IBone *> m_name2boneRefs;
    Hash<HashString, IMorph *> m_name2morphRefs;
    Vector3 m_aabbMax;
    Vector3 m_aabbMin;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    Vector3 m_edgeColor;
    Scalar m_edgeWidth;
    bool m_enableSkinning;
    bool m_enablePhysics;
};

}
}

#endif

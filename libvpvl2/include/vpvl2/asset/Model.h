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
#ifndef VPVL2_ASSET_MODEL_H_
#define VPVL2_ASSET_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IString.h"

#ifdef VPVL2_LINK_ASSIMP
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <assimp.hpp>
#include <aiPostProcess.h>
#include <aiScene.h>
#pragma clang diagnostic pop
#endif

class btDiscreteDynamicsWorld;

namespace vpvl2
{
namespace asset
{

class VPVL2_API Model : public IModel
{
public:
    Model(IEncoding *encoding);
    ~Model();

    Type type() const { return kAssetModel; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_name; }
    const IString *comment() const { return m_name; }
    const IString *englishComment() const { return m_name; }
    bool isVisible() const { return m_visible && !btFuzzyZero(opacity()); }
    bool isPhysicsEnabled() const { return false; }
    ErrorType error() const { return kNoError; }
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t * /* data */, size_t & /* written */) const {}
    size_t estimateSize() const { return 0; }
    void joinWorld(btDiscreteDynamicsWorld * /* world */) {}
    void leaveWorld(btDiscreteDynamicsWorld * /* world */) {}
    void resetAllVerticesTransform() {}
    void resetMotionState(btDiscreteDynamicsWorld * /* worldRef */) {}
    void performUpdate() {}
    IBone *findBoneRef(const IString *value) const;
    IMorph *findMorphRef(const IString *value) const;
    int count(ObjectType value) const;
    void getBoneRefs(Array<IBone *> &value) const;
    void getJointRefs(Array<IJoint *> &value) const;
    void getLabelRefs(Array<ILabel *> &value) const;
    void getMaterialRefs(Array<IMaterial *> &value) const;
    void getMorphRefs(Array<IMorph *> &value) const;
    void getRigidBodyRefs(Array<IRigidBody *> &value) const;
    void getVertexRefs(Array<IVertex *> &value) const;
    void getBoundingBox(Vector3 &min, Vector3 &max) const;
    void getIndices(Array<int> &value) const;
    IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 & /* position */) const { return 0; }
    Vector3 worldPosition() const { return m_position; }
    Quaternion worldRotation() const { return m_rotation; }
    Scalar opacity() const { return m_opacity; }
    Scalar scaleFactor() const { return m_scaleFactor; }
    Vector3 edgeColor() const { return kZeroV3; }
    Scalar edgeWidth() const { static Scalar kZeroWidth = 0; return kZeroWidth; }
    Scene *parentSceneRef() const { return m_parentSceneRef; }
    IModel *parentModelRef() const { return m_parentModelRef; }
    IBone *parentBoneRef() const { return m_parentBoneRef; }
    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);
    void setWorldPosition(const Vector3 &value);
    void setWorldPositionInternal(const Vector3 &value);
    void setWorldRotation(const Quaternion &value);
    void setWorldRotationInternal(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setScaleFactorInternal(const Scalar &value);
    void setEdgeColor(const Vector3 & /* value */) {}
    void setEdgeWidth(const Scalar & /* value */) {}
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel *value);
    void setParentBoneRef(IBone *value);
    void setVisible(bool value);
    void setPhysicsEnable(bool /* value */) {}

    void getIndexBuffer(IndexBuffer *&indexBuffer) const { indexBuffer = 0; }
    void getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const { staticBuffer = 0; }
    void getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer,
                                const IndexBuffer * /* indexBuffer */) const { dynamicBuffer = 0; }
    void getMatrixBuffer(MatrixBuffer *&matrixBuffer,
                         DynamicVertexBuffer * /* dynamicBuffer */,
                         const IndexBuffer * /* indexBuffer */) const { matrixBuffer = 0; }
    void setAabb(const Vector3 &min, const Vector3 &max);
    void getAabb(Vector3 &min, Vector3 &max) const;

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

#ifdef VPVL2_LINK_ASSIMP
    const aiScene *aiScenePtr() const { return m_scene; }
#endif

private:
#ifdef VPVL2_LINK_ASSIMP
    void setIndicesRecurse(const aiScene *scene, const aiNode *node);
    void setMaterialRefsRecurse(const aiScene *scene, const aiNode *node);
    void setVertexRefsRecurse(const aiScene *scene, const aiNode *node);
    void getBoundingBoxRecurse(const aiScene *scene, const aiNode *node, Vector3 &min, Vector3 &max) const;
    Assimp::Importer m_importer;
    const aiScene *m_scene;
#endif

    IEncoding *m_encodingRef;
    IString *m_name;
    IString *m_comment;
    Scene *m_parentSceneRef;
    IModel *m_parentModelRef;
    IBone *m_parentBoneRef;
    IBone *m_rootBoneRef;
    IBone *m_scaleBoneRef;
    IMorph *m_opacityMorphRef;
    mutable PointerArray<IBone> m_bones;
    mutable PointerArray<ILabel> m_labels;
    mutable PointerArray<IMaterial> m_materials;
    mutable PointerArray<IMorph> m_morphs;
    mutable PointerArray<IVertex> m_vertices;
    mutable Array<uint32_t> m_indices;
    Hash<HashString, IBone *> m_name2boneRefs;
    Hash<HashString, IMorph *> m_name2morphRefs;
    Vector3 m_aabbMax;
    Vector3 m_aabbMin;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    bool m_visible;
};

} /* namespace asset */
} /* namespace vpvl2 */

#endif

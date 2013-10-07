/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_ASSET_MODEL_H_
#define VPVL2_ASSET_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IString.h"

#if defined(VPVL2_LINK_ASSIMP3) || defined(VPVL2_LINK_ASSIMP)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#if defined(VPVL2_LINK_ASSIMP3)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#elif defined(VPVL2_LINK_ASSIMP)
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#endif
#pragma clang diagnostic pop
#endif

class btDiscreteDynamicsWorld;

namespace vpvl2
{
namespace asset
{

class VPVL2_API Model VPVL2_DECL_FINAL : public IModel
{
public:
    Model(IEncoding *encoding);
    ~Model();

    void addEventListenerRef(PropertyEventListener * /* value */) {}
    void removeEventListenerRef(PropertyEventListener * /* value */) {}
    void getEventListenerRefs(Array<PropertyEventListener *> &value) { value.clear(); }
    Type type() const { return kAssetModel; }
    const IString *name(IEncoding::LanguageType /* type */) const { return m_name; }
    const IString *comment(IEncoding::LanguageType /* type */) const { return m_name; }
    bool isVisible() const { return m_visible && !btFuzzyZero(opacity()); }
    bool isPhysicsEnabled() const { return false; }
    ErrorType error() const { return kNoError; }
    bool load(const uint8 *data, vsize size);
    void save(uint8 * /* data */, vsize & /* written */) const {}
    vsize estimateSize() const { return 0; }
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
    void getTextureRefs(Array<const IString *> &value) const;
    void getVertexRefs(Array<IVertex *> &value) const;
    void getBoundingBox(Vector3 &min, Vector3 &max) const;
    void getIndices(Array<int> &value) const;
    IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 & /* position */) const { return 0; }
    Vector3 worldTranslation() const { return m_position; }
    Quaternion worldOrientation() const { return m_rotation; }
    Scalar opacity() const { return m_opacity; }
    Scalar scaleFactor() const { return m_scaleFactor; }
    Color edgeColor() const { return kZeroC; }
    IVertex::EdgeSizePrecision edgeWidth() const { static IVertex::EdgeSizePrecision kZeroWidth = 0; return kZeroWidth; }
    Scene *parentSceneRef() const { return m_parentSceneRef; }
    IModel *parentModelRef() const { return m_parentModelRef; }
    IBone *parentBoneRef() const { return m_parentBoneRef; }
    void setName(const IString *value, IEncoding::LanguageType type);
    void setComment(const IString *value, IEncoding::LanguageType type);
    void setWorldTranslation(const Vector3 &value);
    void setWorldPositionInternal(const Vector3 &value);
    void setWorldOrientation(const Quaternion &value);
    void setWorldRotationInternal(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setScaleFactorInternal(const Scalar &value);
    void setEdgeColor(const Color & /* value */) {}
    void setEdgeWidth(const IVertex::EdgeSizePrecision & /* value */) {}
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

    float32 version() const;
    void setVersion(float32 value);
    int maxUVCount() const;
    void setMaxUVCount(int value);
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

#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
    const aiScene *aiScenePtr() const { return m_scene; }
#endif

private:
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
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
    mutable Array<uint32> m_indices;
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

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

class VPVL2_API Model VPVL2_DECL_FINAL : public IModel
{
public:
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
        float32 version;
        uint8 *basePtr;
        uint8 *namePtr;
        vsize additionalUVSize;
        vsize vertexIndexSize;
        vsize textureIndexSize;
        vsize materialIndexSize;
        vsize boneIndexSize;
        vsize morphIndexSize;
        vsize rigidBodyIndexSize;
        int nameSize;
        uint8 *englishNamePtr;
        int englishNameSize;
        uint8 *commentPtr;
        int commentSize;
        uint8 *englishCommentPtr;
        int englishCommentSize;
        uint8 *verticesPtr;
        vsize verticesCount;
        uint8 *indicesPtr;
        vsize indicesCount;
        uint8 *texturesPtr;
        vsize texturesCount;
        uint8 *materialsPtr;
        vsize materialsCount;
        uint8 *bonesPtr;
        vsize bonesCount;
        uint8 *morphsPtr;
        vsize morphsCount;
        uint8 *labelsPtr;
        vsize labelsCount;
        uint8 *rigidBodiesPtr;
        vsize rigidBodiesCount;
        uint8 *jointsPtr;
        vsize jointsCount;
        uint8 *endPtr;
    };

    /**
     * Constructor
     */
    Model(IEncoding *encoding);
    ~Model();

    bool load(const uint8 *data, vsize size);
    void save(uint8 *data, vsize &written) const;
    vsize estimateSize() const;

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);
    void joinWorld(btDiscreteDynamicsWorld *worldRef);
    void leaveWorld(btDiscreteDynamicsWorld *worldRef);
    void resetAllVerticesTransform();
    void resetMotionState(btDiscreteDynamicsWorld *worldRef);
    void performUpdate();
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
    void getIndices(Array<int> &value) const;

    bool preparse(const uint8 *data, vsize size, DataInfo &info);
    void setVisible(bool value);

    const void *vertexPtr() const;
    const void *indicesPtr() const;
    IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 &cameraPosition) const;

    Type type() const;
    const Array<Vertex *> &vertices() const;
    const Array<int> &indices() const;
    const Hash<HashString, IString *> &textures() const;
    const Array<Material *> &materials() const;
    const Array<Bone *> &bones() const;
    const Array<Morph *> &morphs() const;
    const Array<Label *> &labels() const;
    const Array<RigidBody *> &rigidBodies() const;
    const Array<Joint *> &joints() const;
    const IString *name(IEncoding::LanguageType type) const;
    const IString *comment(IEncoding::LanguageType type) const;
    ErrorType error() const;
    bool isVisible() const;
    bool isPhysicsEnabled() const;
    Vector3 worldTranslation() const;
    Quaternion worldOrientation() const;
    Scalar opacity() const;
    Scalar scaleFactor() const;
    Color edgeColor() const;
    IVertex::EdgeSizePrecision edgeWidth() const;
    Scene *parentSceneRef() const;
    IModel *parentModelRef() const;
    IBone *parentBoneRef() const;

    void setName(const IString *value, IEncoding::LanguageType type);
    void setComment(const IString *value, IEncoding::LanguageType type);
    void setWorldTranslation(const Vector3 &value);
    void setWorldOrientation(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setEdgeColor(const Color & /* value */);
    void setEdgeWidth(const IVertex::EdgeSizePrecision &value);
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel *value);
    void setParentBoneRef(IBone *value);
    void setPhysicsEnable(bool value);

    static void updateLocalTransform(Array<Bone *> &bones);
    void getIndexBuffer(IndexBuffer *&indexBuffer) const;
    void getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const;
    void getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer,
                                const IndexBuffer *indexBuffer) const;
    void getMatrixBuffer(MatrixBuffer *&matrixBuffer,
                         DynamicVertexBuffer *dynamicBuffer,
                         const IndexBuffer *indexBuffer) const;
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
    int addTexture(const IString *value);

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Model)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif


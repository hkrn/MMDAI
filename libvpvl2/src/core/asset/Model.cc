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

#include "vpvl2/vpvl2.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/internal/util.h"

namespace {

#ifdef VPVL2_LINK_ASSIMP
using namespace vpvl2;

class RootBone : public IBone {
public:
    RootBone(IModel *modelRef, const IEncoding *encodingRef)
        : m_encodingRef(encodingRef),
          m_modelRef(modelRef),
          m_worldTransform(Transform::getIdentity())
    {
        m_worldTransform.setOrigin(modelRef->position());
        m_worldTransform.setRotation(modelRef->rotation());
    }
    ~RootBone() {
        m_encodingRef = 0;
        m_modelRef = 0;
        m_worldTransform.setIdentity();
    }

    const IString *name() const { return m_encodingRef->stringConstant(IEncoding::kRootBoneAsset); }
    int index() const { return 0; }
    IBone *parentBone() const { return 0; }
    IBone *targetBone() const { return 0; }
    const Transform &worldTransform() const {
        return m_worldTransform;
    }
    const Transform &localTransform() const {
        return m_worldTransform;
    }
    void getLocalTransform(Transform &world2LocalTransform) const {
        world2LocalTransform = m_worldTransform;
    }
    void setLocalTransform(const Transform & /* value */) {}
    const Vector3 &origin() const { return kZeroV3; }
    const Vector3 destinationOrigin() const { return kZeroV3; }
    const Vector3 &localPosition() const { return m_modelRef->position(); }
    const Quaternion &rotation() const { return m_modelRef->rotation(); }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalPosition(const Vector3 &value) {
        m_modelRef->setPosition(value);
        m_worldTransform.setOrigin(value);
    }
    void setRotation(const Quaternion &value) {
        m_modelRef->setRotation(value);
        m_worldTransform.setRotation(value);
    }
    bool isMovable() const { return true; }
    bool isRotateable() const { return true; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return true; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return true; }
    bool hasLocalAxes() const { return false; }
    const Vector3 &fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}

private:
    const IEncoding *m_encodingRef;
    IModel *m_modelRef;
    Transform m_worldTransform;
};

class ScaleBone : public IBone {
public:
    ScaleBone(IModel *modelRef, const IEncoding *encodingRef)
        : m_encodingRef(encodingRef),
          m_modelRef(modelRef),
          m_position(kZeroV3),
          m_rotation(Quaternion::getIdentity())
    {
        const Scalar &scaleFactor = modelRef->scaleFactor();
        m_position.setValue(scaleFactor, scaleFactor, scaleFactor);
        m_rotation.setValue(scaleFactor, scaleFactor, scaleFactor);
    }
    ~ScaleBone() {
        m_encodingRef = 0;
        m_modelRef = 0;
        m_position.setZero();
        m_rotation.setValue(0, 0, 0, 1);
    }

    const IString *name() const { return m_encodingRef->stringConstant(IEncoding::kScaleBoneAsset); }
    int index() const { return -1; }
    IBone *parentBone() const { return 0; }
    IBone *targetBone() const { return 0; }
    const Transform &worldTransform() const {
        return Transform::getIdentity();
    }
    const Transform &localTransform() const {
        return Transform::getIdentity();
    }
    void getLocalTransform(Transform &world2LocalTransform) const {
        world2LocalTransform = Transform::getIdentity();
    }
    void setLocalTransform(const Transform & /* value */) {}
    const Vector3 &origin() const { return kZeroV3; }
    const Vector3 destinationOrigin() const { return kZeroV3; }
    const Vector3 &localPosition() const { return m_position; }
    const Quaternion &rotation() const { return m_rotation; }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalPosition(const Vector3 &value) { m_position = value; }
    void setRotation(const Quaternion &value) { m_rotation = value; }
    bool isMovable() const { return false; }
    bool isRotateable() const { return false; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return false; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return false; }
    bool hasLocalAxes() const { return false; }
    const Vector3 &fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}

private:
    const IEncoding *m_encodingRef;
    IModel *m_modelRef;
    Vector3 m_position;
    Quaternion m_rotation;
};

class Label : public ILabel {
public:
    Label(const Array<IBone *> &bones, const IEncoding *encodingRef)
        : m_name(0)
    {
        static const uint8_t name[] = "Root";
        m_name = encodingRef->toString(name, sizeof(name), IString::kUTF8);
        m_bones.copy(bones);
    }
    ~Label() {
        delete m_name;
        m_name = 0;
    }

    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_name; }
    bool isSpecial() const { return true; }
    int count() const { return m_bones.count(); }
    IBone *bone(int index) const { return m_bones[index]; }
    IMorph *morph(int /*index*/) const { return 0; }

private:
    const IString *m_name;
    Array<IBone *> m_bones;
};

class OpacityMorph : public IMorph {
public:
    OpacityMorph(IModel *modelRef, const IEncoding *encodingRef)
        : m_encodingRef(encodingRef),
          m_modelRef(modelRef),
          m_opacity(modelRef->opacity())
    {
    }
    ~OpacityMorph() {
        m_encodingRef = 0;
        m_modelRef = 0;
        m_opacity = 0;
    }

    const IString *name() const { return m_encodingRef->stringConstant(IEncoding::kOpacityMorphAsset); }
    int index() const { return 0; }
    Category category() const { return IMorph::kOther; }
    Type type() const { return IMorph::kMaterial; }
    bool hasParent() const { return false; }
    const WeightPrecision &weight() const { return m_opacity; }
    void setWeight(const WeightPrecision &value) {
        m_modelRef->setOpacity(value);
        m_opacity = value;
    }

private:
    const IEncoding *m_encodingRef;
    IModel *m_modelRef;
    WeightPrecision m_opacity;
};

class Vertex : public IVertex {
public:
    Vertex(const Vector3 &origin, const Vector3 &normal, const Vector3 &texcoord, int index)
        : m_origin(origin),
          m_normal(normal),
          m_texcoord(texcoord),
          m_index(index)
    {
    }
    ~Vertex() {
        m_origin.setZero();
        m_normal.setZero();
        m_texcoord.setZero();
        m_index = 0;
    }

    void performSkinning(Vector3 &/*position*/, Vector3 &/*normal*/) const {}
    void reset() {}
    const Vector3 &origin() const { return m_origin; }
    const Vector3 &normal() const { return m_normal; }
    const Vector3 &textureCoord() const { return m_texcoord; }
    const Vector4 &uv(int index) const { return kZeroV4; }
    const Vector3 &delta() const { return kZeroV3; }
    Type type() const { return IVertex::kBdef1; }
    float edgeSize() const { return 0; }
    float weight(int index) const { return 0; }
    IBone *bone(int index) const { return 0; }
    int index() const { return m_index; }
    void setOrigin(const Vector3 &/*value*/) {}
    void setNormal(const Vector3 &/*value*/) {}
    void setTextureCoord(const Vector3 &/*value*/) {}
    void setUV(int /*index*/, const Vector4 &/*value*/) {}
    void setType(Type /*value*/) {}
    void setEdgeSize(float /*value*/) {}
    void setWeight(int /*index*/, float /*weight*/) {}
    void setBone(int /*index*/, IBone */*value*/) {}

private:
    Vector3 m_origin;
    Vector3 m_normal;
    Vector3 m_texcoord;
    int m_index;
};
#endif

}

namespace vpvl2
{
namespace asset
{

Model::Model(IEncoding *encoding)
    : m_encodingRef(encoding),
      m_name(0),
      m_comment(0),
      m_parentModelRef(0),
      m_parentBoneRef(0),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(10),
      m_visible(false)
{
}

Model::~Model()
{
    delete m_comment;
    m_comment = 0;
    delete m_name;
    m_name = 0;
    m_parentBoneRef = 0;
    m_parentModelRef = 0;
    m_encodingRef = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_bones.releaseAll();
    m_labels.releaseAll();
    m_morphs.releaseAll();
    m_vertices.releaseAll();
    m_opacity = 0;
    m_scaleFactor = 0;
    m_visible = false;
}

bool Model::load(const uint8_t *data, size_t size)
{
#ifdef VPVL2_LINK_ASSIMP
    int flags = aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs;
    m_scene = m_importer.ReadFileFromMemory(data, size, flags);
    m_bones.add(new RootBone(this, m_encodingRef));
    m_bones.add(new ScaleBone(this, m_encodingRef));
    m_labels.add(new Label(m_bones, m_encodingRef));
    m_morphs.add(new OpacityMorph(this, m_encodingRef));
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = m_bones[i];
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
    }
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = m_morphs[i];
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
    }
    return m_scene != 0;
#else
    return false;
#endif
}

IBone *Model::findBone(const IString *value) const
{
    IBone **bone = const_cast<IBone **>(m_name2boneRefs.find(value->toHashString()));
    return bone ? *bone : 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    IMorph **morph = const_cast<IMorph **>(m_name2morphRefs.find(value->toHashString()));
    return morph ? *morph : 0;
}

int Model::count(ObjectType value) const
{
    switch (value) {
    case kBone:
        return m_bones.count();
    case kIK:
        return 0;
    case kIndex:
        return 0;
    case kJoint:
        return 0;
    case kMaterial:
        return 0;
    case kMorph:
        return m_morphs.count();
    case kRigidBody:
        return 0;
    case kVertex:
        return m_vertices.count();
    default:
        return 0;
    }
}

void Model::getBoneRefs(Array<IBone *> &value) const
{
    value.copy(m_bones);
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    value.copy(m_labels);
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    value.copy(m_morphs);
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
#ifdef VPVL2_LINK_ASSIMP
    if (m_vertices.count() == 0) {
        getVertexRefsRecurse(m_scene, m_scene->mRootNode, m_vertices);
    }
    value.copy(m_vertices);
#endif
}

void Model::getBoundingBox(Vector3 &min, Vector3 &max) const
{
    min.setZero();
    max.setZero();
#ifdef VPVL2_LINK_ASSIMP
    getBoundingBoxRecurse(m_scene, m_scene->mRootNode, min, max);
#endif
}

void Model::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Model::setEnglishName(const IString *value)
{
    setName(value);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_comment);
}

void Model::setEnglishComment(const IString *value)
{
    setComment(value);
}

void Model::setPosition(const Vector3 &value)
{
    m_position = value;
}

void Model::setRotation(const Quaternion &value)
{
    m_rotation = value;
}

void Model::setOpacity(const Scalar &value)
{
    m_opacity = value;
}

void Model::setScaleFactor(const Scalar &value)
{
    m_scaleFactor = value;
}

void Model::setParentModel(IModel *value)
{
    m_parentModelRef = value;
}

void Model::setParentBone(IBone *value)
{
    m_parentBoneRef = value;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

#ifdef VPVL2_LINK_ASSIMP
void Model::getVertexRefsRecurse(const aiScene *scene, const aiNode *node, Array<IVertex *> &vertices) const
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *meshVertices = mesh->mVertices;
        const aiVector3D *meshNormals = mesh->HasNormals() ? mesh->mNormals : 0;
        const aiVector3D *meshTextureCoords = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0] : 0;
        const unsigned int nvertices = mesh->mNumVertices;
        for (unsigned int j = 0; j < nvertices; j++) {
            const aiVector3D &v = meshVertices[j];
            const aiVector3D &n = meshNormals ? meshVertices[j] : aiVector3D();
            const aiVector3D &t = meshTextureCoords ? meshTextureCoords[j] : aiVector3D();
            vertices.add(new Vertex(Vector3(v.x, v.y, v.z), Vector3(n.x, n.y, n.z), Vector3(t.x, t.y, t.z), j));
        }
    }
}

void Model::getBoundingBoxRecurse(const aiScene *scene, const aiNode *node, Vector3 &min, Vector3 &max) const
{
    const unsigned int nmeshes = node->mNumMeshes;
    const Scalar &scale = scaleFactor();
    Vector3 position;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const unsigned int nvertices = mesh->mNumVertices;
        for (unsigned int j = 0; j < nvertices; j++) {
            const aiVector3D &vertex = vertices[i] * scale;
            position.setValue(vertex.x, vertex.y, vertex.z);
            min.setMin(position);
            max.setMax(position);
        }
    }
}
#endif

} /* namespace asset */
} /* namespace vpvl2 */

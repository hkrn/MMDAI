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
    bool hasFixedAxes() const { return false; }
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
          m_position(kZeroV3)
    {
        const Scalar &scaleFactor = modelRef->scaleFactor();
        m_position.setValue(scaleFactor, scaleFactor, scaleFactor);
    }
    ~ScaleBone() {
        m_encodingRef = 0;
        m_modelRef = 0;
        m_position.setZero();
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
    const Quaternion &rotation() const { return Quaternion::getIdentity(); }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalPosition(const Vector3 &value) {
        m_position = value;
        m_position.setMax(kMaxValue);
        const Scalar &scaleFactor = (m_position.x() + m_position.y() + m_position.z()) / 3.0;
        m_modelRef->setScaleFactor(scaleFactor);
    }
    void setRotation(const Quaternion & /* value */) {}
    bool isMovable() const { return true; }
    bool isRotateable() const { return false; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return true; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return false; }
    bool hasLocalAxes() const { return false; }
    const Vector3 &fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}

private:
    static const Vector3 kMaxValue;
    const IEncoding *m_encodingRef;
    IModel *m_modelRef;
    Vector3 m_position;
};
const Vector3 ScaleBone::kMaxValue = Vector3(0.01, 0.01, 0.01);

class Label : public ILabel {
public:
    Label(const Array<IBone *> &bones, const IEncoding *encodingRef)
        : m_name(0)
    {
        static const uint8_t name[] = "Root";
        m_name = encodingRef->toString(name, sizeof(name) - 1, IString::kUTF8);
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
    IString *m_name;
    Array<IBone *> m_bones;
};

class Material : public IMaterial {
public:
    Material(const aiMaterial *materialRef, IEncoding *encodingRef, int nindices, int index)
        : m_materialRef(materialRef),
          m_encodingRef(encodingRef),
          m_sphereTextureRenderMode(kNone),
          m_nindices(nindices),
          m_index(index)
    {
        aiColor4D color;
        aiGetMaterialColor(m_materialRef, AI_MATKEY_COLOR_AMBIENT, &color);
        m_ambient.setValue(color.r, color.g, color.b, 1);
        aiGetMaterialColor(m_materialRef, AI_MATKEY_COLOR_DIFFUSE, &color);
        m_diffuse.setValue(color.r, color.g, color.b, color.a);
        aiGetMaterialColor(m_materialRef, AI_MATKEY_COLOR_SPECULAR, &color);
        m_specular.setValue(color.r, color.g, color.b, 1);
        float shininess;
        aiGetMaterialFloat(m_materialRef, AI_MATKEY_SHININESS, &shininess);
        m_shininess = shininess;
        setMaterialTextures();
    }
    ~Material() {
        delete m_mainTexture;
        m_mainTexture =0 ;
        delete m_sphereTexture;
        m_sphereTexture = 0;
        m_materialRef = 0;
        m_encodingRef = 0;
        m_ambient.setZero();
        m_diffuse.setZero();
        m_specular.setZero();
        m_shininess = 0;
        m_nindices = 0;
        m_index = 0;
    }

    const IString *name() const { return 0; }
    const IString *englishName() const { return 0; }
    const IString *userDataArea() const { return 0; }
    const IString *mainTexture() const { return m_mainTexture; }
    const IString *sphereTexture() const { return m_sphereTexture; }
    const IString *toonTexture() const { return 0; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return m_sphereTextureRenderMode; }
    const Color &ambient() const { return m_ambient; }
    const Color &diffuse() const { return m_diffuse; }
    const Color &specular() const { return m_specular; }
    const Color &edgeColor() const { return kZeroC; }
    const Color &mainTextureBlend() const { return kWhiteColor; }
    const Color &sphereTextureBlend() const { return kWhiteColor; }
    const Color &toonTextureBlend() const { return kWhiteColor; }
    float shininess() const { return m_shininess; }
    float edgeSize() const { return 1; }
    int index() const { return m_index; }
    int textureIndex() const { return -1; }
    int sphereTextureIndex() const { return -1; }
    int toonTextureIndex() const { return -1; }
    int indices() const { return m_nindices; }
    bool isSharedToonTextureUsed() const { return false; }
    bool isCullFaceDisabled() const { return !btFuzzyZero(m_diffuse.w() - 1); }
    bool hasShadow() const { return false; }
    bool isShadowMapDrawn() const { return !btFuzzyZero(m_diffuse.x() - 0.98f); }
    bool isSelfShadowDrawn() const { return isShadowMapDrawn(); }
    bool isEdgeDrawn() const { return false; }

    void setName(const IString * /* value */) {}
    void setEnglishName(const IString * /* value */) {}
    void setUserDataArea(const IString * /* value */) {}
    void setMainTexture(const IString */*value*/) {}
    void setSphereTexture(const IString */*value*/) {}
    void setToonTexture(const IString */*value*/) {}
    void setSphereTextureRenderMode(SphereTextureRenderMode /*value*/) {}
    void setAmbient(const Color &value) { m_ambient = value; }
    void setDiffuse(const Color &value) { m_diffuse = value; }
    void setSpecular(const Color &value) { m_specular = value; }
    void setEdgeColor(const Color & /* value */) {}
    void setShininess(float value) { m_shininess = value; }
    void setEdgeSize(float /* value */) {}
    void setMainTextureIndex(int /* value */) {}
    void setSphereTextureIndex(int /* value */) {}
    void setToonTextureIndex(int /* value */) {}
    void setIndices(int /* value */) {}
    void setFlags(int /* value */) {}

private:
    void setMaterialTextures() {
        aiString texturePath;
        if (m_materialRef->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS) {
            const uint8_t *path = reinterpret_cast<const uint8_t *>(texturePath.data);
            const IString *separator = m_encodingRef->stringConstant(IEncoding::kAsterisk);
            const IString *sph = m_encodingRef->stringConstant(IEncoding::kSPHExtension);
            const IString *spa = m_encodingRef->stringConstant(IEncoding::kSPAExtension);
            IString *texture = m_encodingRef->toString(path, texturePath.length, IString::kShiftJIS);
            if (texture->contains(separator)) {
                Array<IString *> tokens;
                texture->split(separator, 2, tokens);
                delete texture;
                IString *mainTexture = tokens[0];
                if (mainTexture->endsWith(sph)) {
                    m_sphereTexture = mainTexture;
                    m_sphereTextureRenderMode = kMultTexture;
                }
                else {
                    m_mainTexture = mainTexture;
                }
                if (tokens.count() == 2) {
                    IString *subTexture = tokens[1];
                    if (subTexture->endsWith(sph)) {
                        m_sphereTexture = subTexture;
                        m_sphereTextureRenderMode = kMultTexture;
                    }
                    else if (subTexture->endsWith(spa)) {
                        m_sphereTexture = subTexture;
                        m_sphereTextureRenderMode = kAddTexture;
                    }
                }
            }
            else if (texture->endsWith(sph)) {
                m_sphereTexture = texture;
                m_sphereTextureRenderMode = kMultTexture;
            }
        }
    }

    static const Color kWhiteColor;
    const aiMaterial *m_materialRef;
    IEncoding *m_encodingRef;
    IString *m_mainTexture;
    IString *m_sphereTexture;
    SphereTextureRenderMode m_sphereTextureRenderMode;
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    float m_shininess;
    int m_nindices;
    int m_index;
};
const Color Material::kWhiteColor = Color(1, 1, 1, 1);

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
      m_aabbMax(kZeroV3),
      m_aabbMin(kZeroV3),
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

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
#ifdef VPVL2_LINK_ASSIMP
    if (m_materials.count() == 0) {
        setMaterialRefsRecurse(m_scene, m_scene->mRootNode);
    }
    value.copy(m_materials);
#endif
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    value.copy(m_morphs);
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
#ifdef VPVL2_LINK_ASSIMP
    if (m_vertices.count() == 0) {
        setVertexRefsRecurse(m_scene, m_scene->mRootNode);
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

void Model::setAabb(const Vector3 &min, const Vector3 &max)
{
    m_aabbMin = min;
    m_aabbMax = max;
}

void Model::getAabb(Vector3 &min, Vector3 &max) const
{
    min = m_aabbMin;
    max = m_aabbMax;
}

#ifdef VPVL2_LINK_ASSIMP
void Model::setIndicesRecurse(const aiScene *scene, const aiNode *node)
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiFace *faces = mesh->mFaces;
        const unsigned int nfaces = mesh->mNumFaces;
        for (unsigned int j = 0; j < nfaces; j++) {
            const struct aiFace &face = faces[j];
            const unsigned int *indices = face.mIndices;
            const unsigned int nindices = face.mNumIndices;
            for (unsigned int k = 0; k < nindices; k++) {
                m_indices.add(indices[k]);
            }
        }
    }
}

void Model::setMaterialRefsRecurse(const aiScene *scene, const aiNode *node) const
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        const struct aiFace *faces = mesh->mFaces;
        const unsigned int nfaces = mesh->mNumFaces;
        unsigned int nindices = 0;
        for (unsigned int j = 0; j < nfaces; j++) {
            nindices += faces[j].mNumIndices;
        }
        m_materials.add(new Material(material, m_encodingRef, nindices, i));
    }
}

void Model::setVertexRefsRecurse(const aiScene *scene, const aiNode *node) const
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
            m_vertices.add(new Vertex(Vector3(v.x, v.y, v.z), Vector3(n.x, n.y, n.z), Vector3(t.x, t.y, t.z), j));
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

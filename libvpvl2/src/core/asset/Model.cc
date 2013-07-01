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

#include "vpvl2/vpvl2.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/internal/ModelHelper.h"

namespace {

#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
using namespace vpvl2;

class RootBone : public IBone {
public:
    RootBone(asset::Model *modelRef, const IEncoding *encodingRef)
        : m_encodingRef(encodingRef),
          m_modelRef(modelRef),
          m_worldTransform(Transform::getIdentity())
    {
        m_worldTransform.setOrigin(modelRef->worldPosition());
        m_worldTransform.setRotation(modelRef->worldRotation());
    }
    ~RootBone() {
        m_encodingRef = 0;
        m_modelRef = 0;
        m_worldTransform.setIdentity();
    }

    const IString *name(IEncoding::LanguageType /* type */) const {
        return m_encodingRef->stringConstant(IEncoding::kRootBone);
    }
    int index() const { return 0; }
    IModel *parentModelRef() const { return m_modelRef; }
    IBone *parentBoneRef() const { return 0; }
    IBone *effectorBoneRef() const { return 0; }
    Transform worldTransform() const {
        return m_worldTransform;
    }
    Transform localTransform() const {
        return m_worldTransform;
    }
    void getLocalTransform(Transform &world2LocalTransform) const {
        world2LocalTransform = m_worldTransform;
    }
    void setLocalTransform(const Transform & /* value */) {}
    Vector3 origin() const { return kZeroV3; }
    Vector3 destinationOrigin() const { return kZeroV3; }
    Vector3 localTranslation() const { return m_modelRef->worldPosition(); }
    Quaternion localRotation() const { return m_modelRef->worldRotation(); }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalTranslation(const Vector3 &value) {
        m_modelRef->setWorldPositionInternal(value);
        m_worldTransform.setOrigin(value);
    }
    void setLocalRotation(const Quaternion &value) {
        m_modelRef->setWorldRotationInternal(value);
        m_worldTransform.setRotation(value);
    }
    bool isMovable() const { return true; }
    bool isRotateable() const { return true; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return true; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return false; }
    bool hasLocalAxes() const { return false; }
    Vector3 fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}
    bool isInverseKinematicsEnabled() const { return false; }

private:
    const IEncoding *m_encodingRef;
    asset::Model *m_modelRef;
    Transform m_worldTransform;
};

class ScaleBone : public IBone {
public:
    ScaleBone(asset::Model *modelRef, const IEncoding *encodingRef)
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

    const IString *name(IEncoding::LanguageType /* type */) const {
        return m_encodingRef->stringConstant(IEncoding::kScaleBoneAsset);
    }
    int index() const { return -1; }
    IModel *parentModelRef() const { return m_modelRef; }
    IBone *parentBoneRef() const { return 0; }
    IBone *effectorBoneRef() const { return 0; }
    Transform worldTransform() const {
        return Transform::getIdentity();
    }
    Transform localTransform() const {
        return Transform::getIdentity();
    }
    void getLocalTransform(Transform &world2LocalTransform) const {
        world2LocalTransform = Transform::getIdentity();
    }
    void setLocalTransform(const Transform & /* value */) {}
    Vector3 origin() const { return kZeroV3; }
    Vector3 destinationOrigin() const { return kZeroV3; }
    Vector3 localTranslation() const { return m_position; }
    Quaternion localRotation() const { return Quaternion::getIdentity(); }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalTranslation(const Vector3 &value) {
        m_position = value;
        m_position.setMax(kMaxValue);
        const Scalar &scaleFactor = (m_position.x() + m_position.y() + m_position.z()) / 3.0f;
        m_modelRef->setScaleFactorInternal(scaleFactor);
    }
    void setLocalRotation(const Quaternion & /* value */) {}
    bool isMovable() const { return true; }
    bool isRotateable() const { return false; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return true; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return false; }
    bool hasLocalAxes() const { return false; }
    Vector3 fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}
    bool isInverseKinematicsEnabled() const { return false; }

private:
    static const Vector3 kMaxValue;
    const IEncoding *m_encodingRef;
    asset::Model *m_modelRef;
    Vector3 m_position;
};
const Vector3 ScaleBone::kMaxValue = Vector3(0.01f, 0.01f, 0.01f);

class Label : public ILabel {
public:
    Label(asset::Model *modelRef, const Array<IBone *> &bones, const IEncoding *encodingRef)
        : m_modelRef(modelRef),
          m_name(0)
    {
        static const uint8 name[] = "Root";
        m_name = encodingRef->toString(name, sizeof(name) - 1, IString::kUTF8);
        m_bones.copy(bones);
    }
    ~Label() {
        delete m_name;
        m_name = 0;
        m_modelRef = 0;
    }

    const IString *name(IEncoding::LanguageType /* type */) const {
        return m_name;
    }
    const IString *englishName() const { return m_name; }
    IModel *parentModelRef() const { return m_modelRef; }
    bool isSpecial() const { return true; }
    int count() const { return m_bones.count(); }
    IBone *boneRef(int index) const { return m_bones[index]; }
    IMorph *morphRef(int /*index*/) const { return 0; }
    int index() const { return -1; }

private:
    asset::Model *m_modelRef;
    IString *m_name;
    Array<IBone *> m_bones;
};

class Material : public IMaterial {
public:
    Material(asset::Model *modelRef, const aiMaterial *materialRef, IEncoding *encodingRef, int nindices, int index)
        : m_materialRef(materialRef),
          m_modelRef(modelRef),
          m_encodingRef(encodingRef),
          m_mainTexture(0),
          m_sphereTexture(0),
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
        m_modelRef = 0;
        m_materialRef = 0;
        m_encodingRef = 0;
        m_ambient.setZero();
        m_diffuse.setZero();
        m_specular.setZero();
        m_shininess = 0;
        m_nindices = 0;
        m_index = 0;
    }

    IModel *parentModelRef() const { return m_modelRef; }
    const IString *name(IEncoding::LanguageType /* type */) const { return 0; }
    const IString *englishName() const { return 0; }
    const IString *userDataArea() const { return 0; }
    const IString *mainTexture() const { return m_mainTexture; }
    const IString *sphereTexture() const { return m_sphereTexture; }
    const IString *toonTexture() const { return 0; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return m_sphereTextureRenderMode; }
    Color ambient() const { return m_ambient; }
    Color diffuse() const { return m_diffuse; }
    Color specular() const { return m_specular; }
    Color edgeColor() const { return kZeroC; }
    Color mainTextureBlend() const { return kWhiteColor; }
    Color sphereTextureBlend() const { return kWhiteColor; }
    Color toonTextureBlend() const { return kWhiteColor; }
    IndexRange indexRange() const { return IndexRange(); }
    float shininess() const { return m_shininess; }
    IVertex::EdgeSizePrecision edgeSize() const { return 1; }
    int index() const { return m_index; }
    int textureIndex() const { return -1; }
    int sphereTextureIndex() const { return -1; }
    int toonTextureIndex() const { return -1; }
    int sizeofIndices() const { return m_nindices; }
    bool isSharedToonTextureUsed() const { return false; }
    bool isCullingDisabled() const { return !btFuzzyZero(m_diffuse.w() - 1); }
    bool hasShadow() const { return false; }
    bool hasShadowMap() const { return !btFuzzyZero(m_diffuse.x() - 0.98f); }
    bool isSelfShadowEnabled() const { return hasShadowMap(); }
    bool isEdgeEnabled() const { return false; }

    void setName(const IString * /* value */) {}
    void setEnglishName(const IString * /* value */) {}
    void setUserDataArea(const IString * /* value */) {}
    void setMainTexture(const IString * /* value */) {}
    void setSphereTexture(const IString * /* value */) {}
    void setToonTexture(const IString * /* value */) {}
    void setSphereTextureRenderMode(SphereTextureRenderMode /* value */) {}
    void setAmbient(const Color &value) { m_ambient = value; }
    void setDiffuse(const Color &value) { m_diffuse = value; }
    void setSpecular(const Color &value) { m_specular = value; }
    void setEdgeColor(const Color & /* value */) {}
    void setIndexRange(const IndexRange & /* value */) {}
    void setShininess(float value) { m_shininess = value; }
    void setEdgeSize(const IVertex::EdgeSizePrecision & /* value */) {}
    void setMainTextureIndex(int /* value */) {}
    void setSphereTextureIndex(int /* value */) {}
    void setToonTextureIndex(int /* value */) {}
    void setIndices(int /* value */) {}
    void setFlags(int /* value */) {}

private:
    void setMaterialTextures() {
        aiString texturePath;
        if (m_materialRef->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS) {
            const uint8 *path = reinterpret_cast<const uint8 *>(texturePath.data);
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
    asset::Model *m_modelRef;
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
    OpacityMorph(asset::Model *modelRef, const IEncoding *encodingRef)
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

    const IString *name(IEncoding::LanguageType /* type */) const {
        return m_encodingRef->stringConstant(IEncoding::kOpacityMorphAsset);
    }
    int index() const { return 0; }
    IModel *parentModelRef() const { return m_modelRef; }
    Category category() const { return IMorph::kOther; }
    Type type() const { return IMorph::kMaterialMorph; }
    bool hasParent() const { return false; }
    WeightPrecision weight() const { return m_opacity; }
    void setWeight(const WeightPrecision &value) {
        m_modelRef->setOpacity(Scalar(value));
        m_opacity = value;
    }

private:
    const IEncoding *m_encodingRef;
    asset::Model *m_modelRef;
    WeightPrecision m_opacity;
};

class Vertex : public IVertex {
public:
    Vertex(asset::Model *modelRef, const Vector3 &origin, const Vector3 &normal, const Vector3 &texcoord, int index)
        : m_modelRef(modelRef),
          m_origin(origin),
          m_normal(normal),
          m_texcoord(texcoord),
          m_index(index)
    {
    }
    ~Vertex() {
        m_origin.setZero();
        m_normal.setZero();
        m_texcoord.setZero();
        m_modelRef = 0;
        m_index = 0;
    }

    IModel *parentModelRef() const { return m_modelRef; }
    void performSkinning(Vector3 &/*position*/, Vector3 &/*normal*/) const {}
    void reset() {}
    Vector3 origin() const { return m_origin; }
    Vector3 normal() const { return m_normal; }
    Vector3 textureCoord() const { return m_texcoord; }
    Vector4 uv(int /* index */) const { return kZeroV4; }
    Vector3 delta() const { return kZeroV3; }
    Type type() const { return IVertex::kBdef1; }
    EdgeSizePrecision edgeSize() const { return 0; }
    WeightPrecision weight(int /* index */) const { return 0; }
    IBone *boneRef(int /* index */) const { return 0; }
    IMaterial *materialRef() const { return 0; }
    int index() const { return m_index; }
    void setOrigin(const Vector3 & /* value */) {}
    void setNormal(const Vector3 & /* value */) {}
    void setTextureCoord(const Vector3 & /* value */) {}
    void setUV(int /* index */, const Vector4 & /* value */) {}
    void setType(Type /* value */) {}
    void setEdgeSize(const EdgeSizePrecision & /* value */) {}
    void setWeight(int /* index */, const WeightPrecision & /* weight */) {}
    void setBoneRef(int /* index */, IBone * /* value */) {}
    void setMaterialRef(IMaterial * /* material */) {}

private:
    asset::Model *m_modelRef;
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
    :
      #if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
      m_scene(0),
      #endif
      m_encodingRef(encoding),
      m_name(0),
      m_comment(0),
      m_parentSceneRef(0),
      m_parentModelRef(0),
      m_parentBoneRef(0),
      m_rootBoneRef(0),
      m_scaleBoneRef(0),
      m_opacityMorphRef(0),
      m_aabbMax(kZeroV3),
      m_aabbMin(kZeroV3),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(10),
      m_visible(false)
{
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
    m_rootBoneRef = m_bones.append(new RootBone(this, m_encodingRef));
    m_scaleBoneRef = m_bones.append(new ScaleBone(this, m_encodingRef));
    m_labels.append(new Label(this, m_bones, m_encodingRef));
    m_opacityMorphRef = m_morphs.append(new OpacityMorph(this, m_encodingRef));
#endif
}

Model::~Model()
{
    m_rootBoneRef = 0;
    m_scaleBoneRef = 0;
    m_opacityMorphRef = 0;
    m_bones.releaseAll();
    m_labels.releaseAll();
    m_materials.releaseAll();
    m_morphs.releaseAll();
    m_vertices.releaseAll();
    delete m_comment;
    m_comment = 0;
    delete m_name;
    m_name = 0;
    m_parentSceneRef = 0;
    m_parentBoneRef = 0;
    m_parentModelRef = 0;
    m_encodingRef = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_opacity = 0;
    m_scaleFactor = 0;
    m_visible = false;
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
    m_scene = 0;
#endif
}

bool Model::load(const uint8 *data, vsize size)
{
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
    int flags = aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs;
    m_scene = m_importer.ReadFileFromMemory(data, size, flags, ".x");
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = m_bones[i];
        m_name2boneRefs.insert(bone->name(IEncoding::kDefaultLanguage)->toHashString(), bone);
    }
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = m_morphs[i];
        m_name2morphRefs.insert(morph->name(IEncoding::kDefaultLanguage)->toHashString(), morph);
    }
    if (m_scene) {
        setMaterialRefsRecurse(m_scene, m_scene->mRootNode);
        setVertexRefsRecurse(m_scene, m_scene->mRootNode);
        return true;
    }
#else
    (void) data;
    (void) size;
#endif
    return false;
}

IBone *Model::findBoneRef(const IString *value) const
{
    IBone *const *bone = m_name2boneRefs.find(value->toHashString());
    return bone ? *bone : 0;
}

IMorph *Model::findMorphRef(const IString *value) const
{
    IMorph *const *morph = m_name2morphRefs.find(value->toHashString());
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
    case kSoftBody:
        return 0;
    case kTexture:
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

void Model::getJointRefs(Array<IJoint *> & /* value */) const
{
    /* do nothing */
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    value.copy(m_labels);
}

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
    value.copy(m_materials);
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    value.copy(m_morphs);
}

void Model::getRigidBodyRefs(Array<IRigidBody *> & /* value */) const
{
    /* do nothing */
}

void Model::getTextureRefs(Array<const IString *> & /* value */) const
{
    /* do nothing */
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
    value.copy(m_vertices);
}

void Model::getIndices(Array<int> & /* value */) const
{
}

void Model::getBoundingBox(Vector3 &min, Vector3 &max) const
{
    min.setZero();
    max.setZero();
#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
    getBoundingBoxRecurse(m_scene, m_scene->mRootNode, min, max);
#endif
}

void Model::setName(const IString *value, IEncoding::LanguageType /* type */)
{
    internal::setString(value, m_name);
}

void Model::setComment(const IString *value, IEncoding::LanguageType /* type */)
{
    internal::setString(value, m_comment);
}

void Model::setWorldPosition(const Vector3 &value)
{
    m_rootBoneRef->setLocalTranslation(value);
}

void Model::setWorldPositionInternal(const Vector3 &value)
{
    m_position = value;
}

void Model::setWorldRotation(const Quaternion &value)
{
    m_rootBoneRef->setLocalRotation(value);
}

void Model::setWorldRotationInternal(const Quaternion &value)
{
    m_rotation = value;
}

void Model::setOpacity(const Scalar &value)
{
    m_opacity = value;
}

void Model::setScaleFactor(const Scalar &value)
{
    m_scaleBoneRef->setLocalTranslation(Vector3(value, value, value));
}

void Model::setScaleFactorInternal(const Scalar &value)
{
    m_scaleFactor = value;
}

void Model::setParentSceneRef(Scene *value)
{
    m_parentSceneRef = value;
}

void Model::setParentModelRef(IModel *value)
{
    if (!internal::ModelHelper::hasModelLoopChain(value, this)) {
        m_parentModelRef = value;
    }
}

void Model::setParentBoneRef(IBone *value)
{
    if (!internal::ModelHelper::hasBoneLoopChain(value, m_parentModelRef)) {
        m_parentBoneRef = value;
    }
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

float32 Model::version() const
{
    return 1.0f;
}

void Model::setVersion(float32 /* value */)
{
    /* do nothing */
}

IBone *Model::createBone()
{
    return 0;
}

IJoint *Model::createJoint()
{
    return 0;
}

ILabel *Model::createLabel()
{
    return 0;
}

IMaterial *Model::createMaterial()
{
    return 0;
}

IMorph *Model::createMorph()
{
    return 0;
}

IRigidBody *Model::createRigidBody()
{
    return 0;
}

IVertex *Model::createVertex()
{
    return 0;
}

IBone *Model::findBoneRefAt(int /* value */) const
{
    return 0;
}

IJoint *Model::findJointRefAt(int /* value */) const
{
    return 0;
}

ILabel *Model::findLabelRefAt(int /* value */) const
{
    return 0;
}

IMaterial *Model::findMaterialRefAt(int /* value */) const
{
    return 0;
}

IMorph *Model::findMorphRefAt(int /* value */) const
{
    return 0;
}

IRigidBody *Model::findRigidBodyRefAt(int /* value */) const
{
    return 0;
}

IVertex *Model::findVertexRefAt(int /* value */) const
{
    return 0;
}

void Model::setIndices(const Array<int> & /* value */)
{
    /* do nothing */
}

void Model::addBone(IBone * /* value */)
{
    /* do nothing */
}

void Model::addJoint(IJoint * /* value */)
{
    /* do nothing */
}

void Model::addLabel(ILabel * /* value */)
{
    /* do nothing */
}

void Model::addMaterial(IMaterial * /* value */)
{
    /* do nothing */
}

void Model::addMorph(IMorph * /* value */)
{
    /* do nothing */
}

void Model::addRigidBody(IRigidBody * /* value */)
{
    /* do nothing */
}

void Model::addVertex(IVertex * /* value */)
{
    /* do nothing */
}

void Model::removeBone(IBone * /* value */)
{
    /* do nothing */
}

void Model::removeJoint(IJoint * /* value */)
{
    /* do nothing */
}

void Model::removeLabel(ILabel * /* value */)
{
    /* do nothing */
}

void Model::removeMaterial(IMaterial * /* value */)
{
    /* do nothing */
}

void Model::removeMorph(IMorph * /* value */)
{
    /* do nothing */
}

void Model::removeRigidBody(IRigidBody * /* value */)
{
    /* do nothing */
}

void Model::removeVertex(IVertex * /* value */)
{
    /* do nothing */
}

#if defined(VPVL2_LINK_ASSIMP) || defined(VPVL2_LINK_ASSIMP3)
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
                m_indices.append(indices[k]);
            }
        }
    }
}

void Model::setMaterialRefsRecurse(const aiScene *scene, const aiNode *node)
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
        m_materials.append(new Material(this, material, m_encodingRef, nindices, i));
    }
}

void Model::setVertexRefsRecurse(const aiScene *scene, const aiNode *node)
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
            m_vertices.append(new Vertex(this, Vector3(v.x, v.y, v.z), Vector3(n.x, n.y, n.z), Vector3(t.x, t.y, t.z), j));
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

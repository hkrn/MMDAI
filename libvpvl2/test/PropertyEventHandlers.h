#include <vpvl2/vpvl2.h>

using namespace vpvl2;

class MockBonePropertyEventListener : public IBone::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IBone *bone));
    MOCK_METHOD2(localTranslationWillChange, void(const Vector3 &value, IBone *bone));
    MOCK_METHOD2(localOrientationWillChange, void(const Quaternion &value, IBone *bone));
    MOCK_METHOD2(inverseKinematicsEnableWillChange, void(bool value, IBone *bone));
};

class MockJointPropertyEventListner : public IJoint::PropertyEventListener {
public:
    MOCK_METHOD2(rigidBody1RefWillChange, void(IRigidBody *value, IJoint *joint));
    MOCK_METHOD2(rigidBody2RefWillChange, void(IRigidBody *value, IJoint *joint));
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IJoint *joint));
    MOCK_METHOD2(positionWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(rotationWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(positionLowerLimitWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(positionUpperLimitWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(rotationLowerLimitWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(rotationUpperLimitWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(positionStiffnessWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(rotationStiffnessWillChange, void(const Vector3 &value, IJoint *joint));
    MOCK_METHOD2(typeWillChange, void(IJoint::Type value, IJoint *joint));
};

class MockLabelPropertyEventListener : public ILabel::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, ILabel *label));
};

class MockMaterialPropertyEventListener : public IMaterial::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IMaterial *material));
    MOCK_METHOD2(userDataAreaWillChange, void(const IString *value, IMaterial *material));
    MOCK_METHOD2(mainTextureWillChange, void(const IString *value, IMaterial *material));
    MOCK_METHOD2(sphereTextureWillChange, void(const IString *value, IMaterial *material));
    MOCK_METHOD2(toonTextureWillChange, void(const IString *value, IMaterial *material));
    MOCK_METHOD2(sphereTextureRenderModeWillChange, void(IMaterial::SphereTextureRenderMode value, IMaterial *material));
    MOCK_METHOD2(ambientWillChange, void(const Color &value, IMaterial *material));
    MOCK_METHOD2(diffuseWillChange, void(const Color &value, IMaterial *material));
    MOCK_METHOD2(specularWillChange, void(const Color &value, IMaterial *material));
    MOCK_METHOD2(edgeColorWillChange, void(const Color &value, IMaterial *material));
    MOCK_METHOD2(indexRangeWillChange, void(const IMaterial::IndexRange &value, IMaterial *material));
    MOCK_METHOD2(shininessWillChange, void(float32 value, IMaterial *material));
    MOCK_METHOD2(edgeSizeWillChange, void(const IVertex::EdgeSizePrecision &value, IMaterial *material));
    MOCK_METHOD2(mainTextureIndexWillChange, void(int value, IMaterial *material));
    MOCK_METHOD2(sphereTextureIndexWillChange, void(int value, IMaterial *material));
    MOCK_METHOD2(toonTextureIndexWillChange, void(int value, IMaterial *material));
    MOCK_METHOD2(flagsWillChange, void(int value, IMaterial *material));
};

class MockModelPropertyEventListener : public IModel::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IModel *model));
    MOCK_METHOD3(commentWillChange, void(const IString *value, IEncoding::LanguageType type, IModel *model));
    MOCK_METHOD2(worldTranslationWillChange, void(const Vector3 &value, IModel *model));
    MOCK_METHOD2(worldOrientationWillChange, void(const Quaternion &value, IModel *model));
    MOCK_METHOD2(opacityWillChange, void(const Scalar &value, IModel *model));
    MOCK_METHOD2(scaleFactorWillChange, void(const Scalar &value, IModel *model));
    MOCK_METHOD2(edgeColorWillChange, void(const Vector3 &value, IModel *model));
    MOCK_METHOD2(edgeWidthWillChange, void(const IVertex::EdgeSizePrecision &value, IModel *model));
    MOCK_METHOD2(parentModelRefWillChange, void(IModel *value, IModel *model));
    MOCK_METHOD2(parentBoneRefWillChange, void(IBone *value, IModel *model));
    MOCK_METHOD2(visibleWillChange, void(bool value, IModel *model));
    MOCK_METHOD2(physicsEnableWillChange, void(bool value, IModel *model));
    MOCK_METHOD3(aabbWillChange, void(const Vector3 &min, const Vector3 &max, IModel *model));
    MOCK_METHOD2(versionWillChange, void(float32 value, IModel *model));
};

class MockMorphPropertyEventListener : public IMorph::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IMorph *morph));
    MOCK_METHOD2(weightWillChange, void(const IMorph::WeightPrecision &value, IMorph *morph));
};

class MockRigidBodyPropertyEventListener : public IRigidBody::PropertyEventListener {
public:
    MOCK_METHOD3(nameWillChange, void(const IString *value, IEncoding::LanguageType type, IRigidBody *rigidBody));
    MOCK_METHOD2(boneRefWillChange, void(IBone *value, IRigidBody *rigidBody));
    MOCK_METHOD2(angularDampingWillChange, void(float32 value, IRigidBody *rigidBody));
    MOCK_METHOD2(collisionGroupIDWillChange, void(uint8 value, IRigidBody *rigidBody));
    MOCK_METHOD2(collisionMaskWillChange, void(uint16 value, IRigidBody *rigidBody));
    MOCK_METHOD2(frictionWillChange, void(float32 value, IRigidBody *rigidBody));
    MOCK_METHOD2(linearDampingWillChange, void(float32 value, IRigidBody *rigidBody));
    MOCK_METHOD2(massWillChange, void(float32 value, IRigidBody *rigidBody));
    MOCK_METHOD2(positionWillChange, void(const Vector3 &value, IRigidBody *rigidBody));
    MOCK_METHOD2(restitutionWillChange, void(float32 value, IRigidBody *rigidBody));
    MOCK_METHOD2(rotationWillChange, void(const Vector3 &value, IRigidBody *rigidBody));
    MOCK_METHOD2(shapeTypeWillChange, void(IRigidBody::ShapeType value, IRigidBody *rigidBody));
    MOCK_METHOD2(sizeWillChange, void(const Vector3 &value, IRigidBody *rigidBody));
    MOCK_METHOD2(objectTypeWillChange, void(IRigidBody::ObjectType value, IRigidBody *rigidBody));
};

class MockVertexPropertyEventListener : public IVertex::PropertyEventListener {
public:
    MOCK_METHOD2(originWillChange, void(const Vector3 &value, IVertex *vertex));
    MOCK_METHOD2(normalWillChange, void(const Vector3 &value, IVertex *vertex));
    MOCK_METHOD2(textureCoordWillChange, void(const Vector3 &value, IVertex *vertex));
    MOCK_METHOD3(originUVWillChange, void(int index, const Vector4 &value, IVertex *vertex));
    MOCK_METHOD3(morphUVWillChange, void(int index, const Vector4 &value, IVertex *vertex));
    MOCK_METHOD2(typeWillChange, void(IVertex::Type value, IVertex *vertex));
    MOCK_METHOD2(edgeSizeWillChange, void(const IVertex::EdgeSizePrecision &value, IVertex *vertex));
    MOCK_METHOD3(weightWillChange, void(int index, const IVertex::WeightPrecision &weight, IVertex *vertex));
    MOCK_METHOD3(boneRefWillChange, void(int index, IBone *value, IVertex *vertex));
    MOCK_METHOD2(materialRefWillChange, void(IMaterial *value, IVertex *vertex));
};

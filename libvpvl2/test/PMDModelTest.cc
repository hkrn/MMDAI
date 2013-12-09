#include "Common.h"
#include "PropertyEventHandlers.h"

#include <btBulletDynamicsCommon.h>

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"

#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Vertex.h"
using namespace vpvl2::pmd;
#else
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Joint.h"
#include "vpvl2/pmd2/Material.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/RigidBody.h"
#include "vpvl2/pmd2/Vertex.h"
using namespace vpvl2::pmd2;
#endif

#include "mock/Bone.h"
#include "mock/Label.h"
#include "mock/Material.h"
#include "mock/Morph.h"
#include "mock/Vertex.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

namespace {

class PMDLanguageTest : public TestWithParam<IEncoding::LanguageType> {};

}

#ifdef VPVL2_LINK_VPVL

TEST(VertexTest, PerformSkinningBdef2WeightZeroPMDCompat)
{
    MockIBone bone1, bone2;
    //Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    //EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(0);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(4.025, 5.05, 6.075), position));
    ASSERT_TRUE(CompareVector(Vector3(0.1, 0.125, 0.15), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightOnePMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    //Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    //EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(100);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.075, 2.15, 3.225), position));
    ASSERT_TRUE(CompareVector(Vector3(0.3, 0.375, 0.45), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightHalfPMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(50);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    const Vector3 &v2 = (Vector3(1.075, 2.15, 3.225) + Vector3(4.025, 5.05, 6.075)) * 0.5;
    const Vector3 &n2 = (Vector3(0.1, 0.125, 0.15) + Vector3(0.3, 0.375, 0.45)) * 0.5;
    ASSERT_TRUE(CompareVector(v2, position));
    ASSERT_TRUE(CompareVector(n2, normal));
}

#else

TEST(PMDVertexTest, Boundary)
{
    Encoding encoding(0);
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0, &encoding));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight(Vertex::kMaxBones, 0.1);
    vertex.setBoneRef(-1, bone.get());
    vertex.setBoneRef(Vertex::kMaxBones, bone.get());
    ASSERT_EQ(vertex.boneRef(-1), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.boneRef(Vertex::kMaxBones), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.weight(-1), 0.0f);
    ASSERT_EQ(vertex.weight(Vertex::kMaxBones), 0.0f);
}

TEST(PMDVertexTest, NullRef)
{
    Encoding encoding(0);
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0, &encoding));
    std::unique_ptr<Material> material(new Material(0, &encoding));
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
    vertex.setBoneRef(0, bone.get());
    vertex.setBoneRef(0, 0);
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    vertex.setMaterialRef(material.get());
    vertex.setMaterialRef(0);
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
}

#endif

TEST(PMDPropertyEventListener, HandleBonePropertyEvents)
{
    Bone bone(0, 0);
    MockBonePropertyEventListener listener;
    TestHandleEvents<IBone::PropertyEventListener>(listener, bone);
    Vector3 v(1, 2, 3);
    Quaternion q(0.1, 0.2, 0.3);
    bool enableIK = !bone.isInverseKinematicsEnabled();
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kJapanese, &bone)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kEnglish, &bone)).WillOnce(Return());
    EXPECT_CALL(listener, inverseKinematicsEnableWillChange(enableIK, &bone)).WillOnce(Return());
    EXPECT_CALL(listener, localTranslationWillChange(v, &bone)).WillOnce(Return());
    EXPECT_CALL(listener, localOrientationWillChange(q, &bone)).WillOnce(Return());
    String japaneseName("Japanese"), englishName("English");
    bone.addEventListenerRef(&listener);
    bone.setName(&japaneseName, IEncoding::kJapanese);
    bone.setName(&japaneseName, IEncoding::kJapanese);
    bone.setName(0, IEncoding::kJapanese);
    bone.setName(0, IEncoding::kJapanese);
    bone.setName(&englishName, IEncoding::kEnglish);
    bone.setName(&englishName, IEncoding::kEnglish);
    bone.setName(0, IEncoding::kEnglish);
    bone.setName(0, IEncoding::kEnglish);
    bone.setLocalTranslation(v);
    bone.setLocalTranslation(v);
    bone.setLocalOrientation(q);
    bone.setLocalOrientation(q);
    bone.setInverseKinematicsEnable(enableIK);
    bone.setInverseKinematicsEnable(enableIK);
}

TEST(PMDPropertyEventListener, HandleJointPropertyEvents)
{
    Joint joint(0, 0);
    MockJointPropertyEventListner listener;
    TestHandleEvents<IJoint::PropertyEventListener>(listener, joint);
    Vector3 position(0.5, 1, 1.5), lowerV(1, 2, 3), upperV(4, 5, 6), stiffnessV(7, 8, 9),
            rotation(0.25, 0.5, 0.75), lowerQ(0.1, 0.2, 0.3), upperQ(0.4, 0.5, 0.6), stiffnessQ(0.7, 0.8, 0.9);
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kJapanese, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kEnglish, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionLowerLimitWillChange(lowerV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionStiffnessWillChange(stiffnessV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionUpperLimitWillChange(upperV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionWillChange(position, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationLowerLimitWillChange(lowerQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationStiffnessWillChange(stiffnessQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationUpperLimitWillChange(upperQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationWillChange(rotation, &joint)).WillOnce(Return());
    String japaneseName("Japanese"), englishName("English");
    joint.addEventListenerRef(&listener);
    joint.setName(&japaneseName, IEncoding::kJapanese);
    joint.setName(&japaneseName, IEncoding::kJapanese);
    joint.setName(0, IEncoding::kJapanese);
    joint.setName(0, IEncoding::kJapanese);
    joint.setName(&englishName, IEncoding::kEnglish);
    joint.setName(&englishName, IEncoding::kEnglish);
    joint.setName(0, IEncoding::kEnglish);
    joint.setName(0, IEncoding::kEnglish);
    joint.setPositionLowerLimit(lowerV);
    joint.setPositionLowerLimit(lowerV);
    joint.setPositionStiffness(stiffnessV);
    joint.setPositionStiffness(stiffnessV);
    joint.setPositionUpperLimit(upperV);
    joint.setPositionUpperLimit(upperV);
    joint.setPosition(position);
    joint.setPosition(position);
    joint.setRotationLowerLimit(lowerQ);
    joint.setRotationLowerLimit(lowerQ);
    joint.setRotationStiffness(stiffnessQ);
    joint.setRotationStiffness(stiffnessQ);
    joint.setRotationUpperLimit(upperQ);
    joint.setRotationUpperLimit(upperQ);
    joint.setRotation(rotation);
    joint.setRotation(rotation);
}

TEST(PMDPropertyEventListener, HandleMaterialPropertyEvents)
{
    Model model(0); /* not to crash at setting texture */
    Material material(&model, 0);
    MockMaterialPropertyEventListener listener;
    TestHandleEvents<IMaterial::PropertyEventListener>(listener, material);
    Color ambient(0.11, 0.12, 0.13, 1), diffuse(0.14, 0.15, 0.16, 1), edgeColor(0.17, 0.18, 0.19, 1), specular(0.20, 0.21, 0.22, 1);
    IVertex::EdgeSizePrecision edgeSize(0.1);
    IMaterial::IndexRange indexRange; indexRange.count = 3, indexRange.end = 4, indexRange.start = 1;
    IMaterial::SphereTextureRenderMode renderMode(IMaterial::kSubTexture);
    Scalar shininess(0.2);
    int flags = 16;
    /* name/flags/edgeSize/userData should not be called */
    EXPECT_CALL(listener, ambientWillChange(ambient, &material)).WillOnce(Return());
    EXPECT_CALL(listener, diffuseWillChange(diffuse, &material)).WillOnce(Return());
    EXPECT_CALL(listener, edgeColorWillChange(edgeColor, &material)).WillOnce(Return());
    EXPECT_CALL(listener, indexRangeWillChange(indexRange, &material)).WillOnce(Return());
    EXPECT_CALL(listener, mainTextureWillChange(_, &material)).WillOnce(Return());
    EXPECT_CALL(listener, shininessWillChange(shininess, &material)).WillOnce(Return());
    EXPECT_CALL(listener, specularWillChange(specular, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureRenderModeWillChange(renderMode, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureWillChange(_, &material)).WillOnce(Return());
    EXPECT_CALL(listener, toonTextureWillChange(_, &material)).WillOnce(Return());
    String mainTexture("MainTexture"), japaneseName("Japanese Name"), englishName("English Name"),
            sphereTexture("SphereTexture"), toonTexture("ToonTexture"), userDataArea("UserDataArea");
    material.addEventListenerRef(&listener);
    material.setAmbient(ambient);
    material.setAmbient(ambient);
    material.setDiffuse(diffuse);
    material.setDiffuse(diffuse);
    material.setEdgeColor(edgeColor);
    material.setEdgeColor(edgeColor);
    material.setEdgeSize(edgeSize);
    material.setEdgeSize(edgeSize);
    material.setFlags(flags);
    material.setFlags(flags);
    material.setIndexRange(indexRange);
    material.setIndexRange(indexRange);
    material.setMainTexture(&mainTexture);
    material.setMainTexture(&mainTexture);
    material.setName(&japaneseName, IEncoding::kJapanese);
    material.setName(&japaneseName, IEncoding::kJapanese);
    material.setName(0, IEncoding::kJapanese);
    material.setName(0, IEncoding::kJapanese);
    material.setName(&englishName, IEncoding::kEnglish);
    material.setName(&englishName, IEncoding::kEnglish);
    material.setName(0, IEncoding::kEnglish);
    material.setName(0, IEncoding::kEnglish);
    material.setShininess(shininess);
    material.setShininess(shininess);
    material.setSpecular(specular);
    material.setSpecular(specular);
    material.setSphereTexture(&sphereTexture);
    material.setSphereTexture(&sphereTexture);
    material.setSphereTextureRenderMode(renderMode);
    material.setSphereTextureRenderMode(renderMode);
    material.setToonTexture(&toonTexture);
    material.setToonTexture(&toonTexture);
    material.setUserDataArea(&userDataArea);
    material.setUserDataArea(&userDataArea);
}

TEST(PMDPropertyEventListener, HandleModelPropertyEvents)
{
    Model model(0), parentModel(0);
    MockModelPropertyEventListener listener;
    TestHandleEvents<IModel::PropertyEventListener>(listener, model);
    Color edgeColor(0.1, 0.2, 0.3, 1.0);
    Vector3 aabbMin(1, 2, 3), aabbMax(4, 5, 6), position(7, 8, 9);
    Quaternion rotation(0.4, 0.5, 0.6, 1);
    IVertex::EdgeSizePrecision edgeSize(0.4);
    Scalar opacity(0.5), scaleFactor(0.6), version(2.1);
    Bone parentBone(0, 0);
    bool physics = !model.isPhysicsEnabled(), visible = !model.isVisible();
    /* version should not be called */
    EXPECT_CALL(listener, aabbWillChange(aabbMin, aabbMax, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(_, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(_, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, edgeColorWillChange(edgeColor, &model)).WillOnce(Return());
    EXPECT_CALL(listener, edgeWidthWillChange(edgeSize, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, opacityWillChange(opacity, &model)).WillOnce(Return());
    EXPECT_CALL(listener, parentBoneRefWillChange(&parentBone, &model)).WillOnce(Return());
    EXPECT_CALL(listener, parentModelRefWillChange(&parentModel, &model)).WillOnce(Return());
    EXPECT_CALL(listener, physicsEnableWillChange(physics, &model)).WillOnce(Return());
    EXPECT_CALL(listener, scaleFactorWillChange(scaleFactor, &model)).WillOnce(Return());
    EXPECT_CALL(listener, visibleWillChange(visible, &model)).WillOnce(Return());
    EXPECT_CALL(listener, worldTranslationWillChange(position, &model)).WillOnce(Return());
    EXPECT_CALL(listener, worldOrientationWillChange(rotation, &model)).WillOnce(Return());
    String japaneseName("Japanese Name"), englishName("English Name"), japaneseComment("Japanese Comment"), englishComemnt("English Comment");
    model.addEventListenerRef(&listener);
    model.setAabb(aabbMin, aabbMax);
    model.setAabb(aabbMin, aabbMax);
    model.setComment(&japaneseComment, IEncoding::kJapanese);
    model.setComment(&japaneseComment, IEncoding::kJapanese);
    model.setComment(0, IEncoding::kJapanese);
    model.setComment(0, IEncoding::kJapanese);
    model.setComment(&englishComemnt, IEncoding::kEnglish);
    model.setComment(&englishComemnt, IEncoding::kEnglish);
    model.setComment(0, IEncoding::kEnglish);
    model.setComment(0, IEncoding::kEnglish);
    model.setEdgeColor(edgeColor);
    model.setEdgeColor(edgeColor);
    model.setEdgeWidth(edgeSize);
    model.setEdgeWidth(edgeSize);
    model.setName(&japaneseName, IEncoding::kJapanese);
    model.setName(&japaneseName, IEncoding::kJapanese);
    model.setName(0, IEncoding::kJapanese);
    model.setName(0, IEncoding::kJapanese);
    model.setName(&englishName, IEncoding::kEnglish);
    model.setName(&englishName, IEncoding::kEnglish);
    model.setName(0, IEncoding::kEnglish);
    model.setName(0, IEncoding::kEnglish);
    model.setOpacity(opacity);
    model.setOpacity(opacity);
    model.setParentBoneRef(&parentBone);
    model.setParentBoneRef(&parentBone);
    model.setParentModelRef(&parentModel);
    model.setParentModelRef(&parentModel);
    model.setPhysicsEnable(physics);
    model.setPhysicsEnable(physics);
    model.setScaleFactor(scaleFactor);
    model.setScaleFactor(scaleFactor);
    model.setVersion(version);
    model.setVersion(version);
    model.setVisible(visible);
    model.setVisible(visible);
    model.setWorldTranslation(position);
    model.setWorldTranslation(position);
    model.setWorldOrientation(rotation);
    model.setWorldOrientation(rotation);
}

TEST(PMDPropertyEventListener, HandleMorphPropertyEvents)
{
    Morph morph(0, 0);
    MockMorphPropertyEventListener listener;
    TestHandleEvents<IMorph::PropertyEventListener>(listener, morph);
    IMorph::WeightPrecision weight(0.42);
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kJapanese, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kEnglish, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, weightWillChange(weight, &morph)).WillOnce(Return());
    String japaneseName("Japanese"), englishName("English");
    morph.addEventListenerRef(&listener);
    morph.setName(&japaneseName, IEncoding::kJapanese);
    morph.setName(&japaneseName, IEncoding::kJapanese);
    morph.setName(0, IEncoding::kJapanese);
    morph.setName(0, IEncoding::kJapanese);
    morph.setName(&englishName, IEncoding::kEnglish);
    morph.setName(&englishName, IEncoding::kEnglish);
    morph.setName(0, IEncoding::kEnglish);
    morph.setName(0, IEncoding::kEnglish);
    morph.setWeight(weight);
    morph.setWeight(weight);
}

TEST(PMDPropertyEventListener, HandleRigidBodyPropertyEvents)
{
    RigidBody body(0, 0);
    MockRigidBodyPropertyEventListener listener;
    TestHandleEvents<IRigidBody::PropertyEventListener>(listener, body);
    float32 angularDamping(0.1), friction(0.2), linearDamping(0.3), mass(0.4), restitution(0.5);
    uint8 collisionGroupID(1);
    uint16_t collisionMask(2);
    Bone bone(0, 0);
    IRigidBody::ShapeType shapeType(IRigidBody::kCapsureShape);
    IRigidBody::ObjectType objectType(IRigidBody::kAlignedObject);
    Vector3 position(1, 2, 3), rotation(0.4, 0.5, 0.6), size(7, 8, 9);
    EXPECT_CALL(listener, angularDampingWillChange(angularDamping, &body)).WillOnce(Return());
    EXPECT_CALL(listener, boneRefWillChange(&bone, &body)).WillOnce(Return());
    EXPECT_CALL(listener, collisionGroupIDWillChange(collisionGroupID, &body)).WillOnce(Return());
    EXPECT_CALL(listener, collisionMaskWillChange(collisionMask, &body)).WillOnce(Return());
    EXPECT_CALL(listener, frictionWillChange(friction, &body)).WillOnce(Return());
    EXPECT_CALL(listener, linearDampingWillChange(linearDamping, &body)).WillOnce(Return());
    EXPECT_CALL(listener, massWillChange(mass, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kJapanese, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(_, IEncoding::kEnglish, &body)).WillOnce(Return());
    EXPECT_CALL(listener, positionWillChange(position, &body)).WillOnce(Return());
    EXPECT_CALL(listener, restitutionWillChange(restitution, &body)).WillOnce(Return());
    EXPECT_CALL(listener, rotationWillChange(rotation, &body)).WillOnce(Return());
    EXPECT_CALL(listener, shapeTypeWillChange(shapeType, &body)).WillOnce(Return());
    EXPECT_CALL(listener, sizeWillChange(size, &body)).WillOnce(Return());
    EXPECT_CALL(listener, objectTypeWillChange(objectType, &body)).WillOnce(Return());
    String japaneseName("Japanese Name"), englishName("English Name");
    body.addEventListenerRef(&listener);
    body.setAngularDamping(angularDamping);
    body.setBoneRef(&bone);
    body.setCollisionGroupID(collisionGroupID);
    body.setCollisionMask(collisionMask);
    body.setFriction(friction);
    body.setLinearDamping(linearDamping);
    body.setMass(mass);
    body.setName(&japaneseName, IEncoding::kJapanese);
    body.setName(&japaneseName, IEncoding::kJapanese);
    body.setName(0, IEncoding::kJapanese);
    body.setName(0, IEncoding::kJapanese);
    body.setName(&englishName, IEncoding::kEnglish);
    body.setName(&englishName, IEncoding::kEnglish);
    body.setName(0, IEncoding::kEnglish);
    body.setName(0, IEncoding::kEnglish);
    body.setPosition(position);
    body.setPosition(position);
    body.setRestitution(restitution);
    body.setRestitution(restitution);
    body.setRotation(rotation);
    body.setRotation(rotation);
    body.setShapeType(shapeType);
    body.setShapeType(shapeType);
    body.setSize(size);
    body.setSize(size);
    body.setObjectType(objectType);
    body.setObjectType(objectType);
}

TEST(PMDPropertyEventListener, HandleVertexPropertyEvents)
{
    Vertex vertex(0);
    MockVertexPropertyEventListener listener;
    TestHandleEvents<IVertex::PropertyEventListener>(listener, vertex);
    Bone bone(0, 0);
    Material material(0, 0);
    IVertex::EdgeSizePrecision edgeSize(0.42);
    IVertex::WeightPrecision weightSize(0.84);
    IVertex::Type type(IVertex::kBdef4);
    Vector3 origin(1, 2, 3), normal(origin.normalized()), texcoord(0.4, 0.5, 0.6);
    Vector4 uv(0.6, 0.7, 0.8, 0.9);
    /* type/UV should not be called */
    EXPECT_CALL(listener, boneRefWillChange(0, &bone, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, edgeSizeWillChange(edgeSize, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, materialRefWillChange(&material, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, normalWillChange(normal, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, originWillChange(origin, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, textureCoordWillChange(texcoord, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, weightWillChange(0, weightSize, &vertex)).WillOnce(Return());
    vertex.addEventListenerRef(&listener);
    vertex.setBoneRef(0, &bone);
    vertex.setBoneRef(0, &bone);
    vertex.setEdgeSize(edgeSize);
    vertex.setEdgeSize(edgeSize);
    vertex.setMaterialRef(&material);
    vertex.setMaterialRef(&material);
    vertex.setNormal(normal);
    vertex.setNormal(normal);
    vertex.setOrigin(origin);
    vertex.setOrigin(origin);
    vertex.setTextureCoord(texcoord);
    vertex.setTextureCoord(texcoord);
    vertex.setType(type);
    vertex.setType(type);
    vertex.setOriginUV(0, uv);
    vertex.setOriginUV(0, uv);
    vertex.setWeight(0, weightSize);
    vertex.setWeight(0, weightSize);
}

TEST(PMDModelTest, AddAndRemoveBone)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IBone> bone(model.createBone());
    ASSERT_EQ(-1, bone->index());
    model.addBone(0); /* should not be crashed */
    model.addBone(bone.get());
    model.addBone(bone.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.bones().count());
    ASSERT_EQ(bone.get(), model.findBoneRefAt(0));
    ASSERT_EQ(bone->index(), model.findBoneRefAt(0)->index());
    model.removeBone(0); /* should not be crashed */
    model.removeBone(bone.get());
    ASSERT_EQ(0, model.bones().count());
    ASSERT_EQ(-1, bone->index());
    MockIBone mockedBone;
    EXPECT_CALL(mockedBone, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedBone, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addBone(&mockedBone);
    ASSERT_EQ(0, model.bones().count());
}

TEST(PMDModelTest, AddAndRemoveLabel)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<ILabel> label(model.createLabel());
    ASSERT_EQ(-1, label->index());
    model.addLabel(0); /* should not be crashed */
    model.addLabel(label.get());
    model.addLabel(label.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.labels().count());
    ASSERT_EQ(label.get(), model.findLabelRefAt(0));
    ASSERT_EQ(label->index(), model.findLabelRefAt(0)->index());
    model.removeLabel(0); /* should not be crashed */
    model.removeLabel(label.get());
    ASSERT_EQ(0, model.labels().count());
    ASSERT_EQ(-1, label->index());
    MockILabel mockedLabel;
    EXPECT_CALL(mockedLabel, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedLabel, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addLabel(&mockedLabel);
    ASSERT_EQ(0, model.labels().count());
}

TEST(PMDModelTest, AddAndRemoveMaterial)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IMaterial> material(model.createMaterial());
    ASSERT_EQ(-1, material->index());
    model.addMaterial(0); /* should not be crashed */
    model.addMaterial(material.get());
    model.addMaterial(material.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.materials().count());
    ASSERT_EQ(material.get(), model.findMaterialRefAt(0));
    ASSERT_EQ(material->index(), model.findMaterialRefAt(0)->index());
    model.removeMaterial(0); /* should not be crashed */
    model.removeMaterial(material.get());
    ASSERT_EQ(0, model.materials().count());
    ASSERT_EQ(-1, material->index());
    MockIMaterial mockedMaterial;
    EXPECT_CALL(mockedMaterial, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMaterial, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMaterial(&mockedMaterial);
    ASSERT_EQ(0, model.materials().count());
}

TEST(PMDModelTest, AddAndRemoveMorph)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IMorph> morph(model.createMorph());
    ASSERT_EQ(-1, morph->index());
    model.addMorph(0); /* should not be crashed */
    model.addMorph(morph.get());
    model.addMorph(morph.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.morphs().count());
    ASSERT_EQ(morph.get(), model.findMorphRefAt(0));
    ASSERT_EQ(morph->index(), model.findMorphRefAt(0)->index());
    model.removeMorph(0); /* should not be crashed */
    model.removeMorph(morph.get());
    ASSERT_EQ(0, model.morphs().count());
    ASSERT_EQ(-1, morph->index());
    MockIMorph mockedMorph;
    EXPECT_CALL(mockedMorph, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMorph, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMorph(&mockedMorph);
    ASSERT_EQ(0, model.morphs().count());
}

TEST(PMDModelTest, AddAndRemoveVertex)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IVertex> vertex(model.createVertex());
    ASSERT_EQ(-1, vertex->index());
    model.addVertex(0); /* should not be crashed */
    model.addVertex(vertex.get());
    model.addVertex(vertex.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.vertices().count());
    ASSERT_EQ(vertex.get(), model.findVertexRefAt(0));
    ASSERT_EQ(vertex->index(), model.findVertexRefAt(0)->index());
    model.removeVertex(0); /* should not be crashed */
    model.removeVertex(vertex.get());
    ASSERT_EQ(0, model.vertices().count());
    ASSERT_EQ(-1, vertex->index());
    MockIVertex mockedVertex;
    EXPECT_CALL(mockedVertex, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedVertex, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addVertex(&mockedVertex);
    ASSERT_EQ(0, model.vertices().count());
}

TEST(PMDModelTest, UnknownLanguageTest)
{
    Encoding encoding(0);
    Model model(&encoding);
    String name("This is a name."), comment("This is a comment.");
    model.setName(&name, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.name(IEncoding::kUnknownLanguageType));
    model.setComment(&comment, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.comment(IEncoding::kUnknownLanguageType));
}

TEST(PMDModelTest, DefaultLanguageSameAsJapanese)
{
    Encoding encoding(0);
    Model model(&encoding);
    String name1("This is a Japanese name type 1."),
            name2("This is a Japanese name type 2."),
            comment1("This is a comment type 1."),
            comment2("This is a comment type 2.");
    model.setName(&name1, IEncoding::kJapanese);
    ASSERT_TRUE(model.name(IEncoding::kDefaultLanguage)->equals(&name1));
    model.setName(&name2, IEncoding::kDefaultLanguage);
    ASSERT_TRUE(model.name(IEncoding::kJapanese)->equals(&name2));
    model.setComment(&comment1, IEncoding::kJapanese);
    ASSERT_TRUE(model.comment(IEncoding::kDefaultLanguage)->equals(&comment1));
    model.setComment(&comment2, IEncoding::kDefaultLanguage);
    ASSERT_TRUE(model.comment(IEncoding::kJapanese)->equals(&comment2));
}

TEST_P(PMDLanguageTest, ReadWriteName)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a name.");
    model.setName(&s, language);
    ASSERT_TRUE(model.name(language)->equals(&s));
}

TEST_P(PMDLanguageTest, ReadWriteComment)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a comment.");
    model.setComment(&s, language);
    ASSERT_TRUE(model.comment(language)->equals(&s));
}

INSTANTIATE_TEST_CASE_P(PMDModelInstance, PMDLanguageTest, Values(IEncoding::kDefaultLanguage,
                                                                  IEncoding::kJapanese,
                                                                  IEncoding::kEnglish));

TEST(PMDModelTest, ParseRealPMD)
{
    QFile file("miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding::Dictionary dict;
        Encoding encoding(&dict);
        Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8 *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMDModel, model.type());
    }
    else {
        // skip
    }
}

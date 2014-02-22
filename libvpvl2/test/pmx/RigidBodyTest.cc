#include "Common.h"

TEST(PMXPropertyEventListener, HandleRigidBodyPropertyEvents)
{
    RigidBody body(0, 0);
    MockRigidBodyPropertyEventListener listener;
    TestHandleEvents<IRigidBody::PropertyEventListener>(listener, body);
    float32 angularDamping(0.1), friction(0.2), linearDamping(0.3), mass(0.4), restitution(0.5);
    uint8 collisionGroupID(1);
    uint16_t collisionMask(2);
    Bone bone(0);
    IRigidBody::ShapeType shapeType(IRigidBody::kCapsureShape);
    IRigidBody::ObjectType objectType(IRigidBody::kAlignedObject);
    String japaneseName("Japanese Name"), englishName("English Name");
    Vector3 position(1, 2, 3), rotation(0.4, 0.5, 0.6), size(7, 8, 9);
    EXPECT_CALL(listener, angularDampingWillChange(angularDamping, &body)).WillOnce(Return());
    EXPECT_CALL(listener, boneRefWillChange(&bone, &body)).WillOnce(Return());
    EXPECT_CALL(listener, collisionGroupIDWillChange(collisionGroupID, &body)).WillOnce(Return());
    EXPECT_CALL(listener, collisionMaskWillChange(collisionMask, &body)).WillOnce(Return());
    EXPECT_CALL(listener, frictionWillChange(friction, &body)).WillOnce(Return());
    EXPECT_CALL(listener, linearDampingWillChange(linearDamping, &body)).WillOnce(Return());
    EXPECT_CALL(listener, massWillChange(mass, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &body)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &body)).WillOnce(Return());
    EXPECT_CALL(listener, positionWillChange(position, &body)).WillOnce(Return());
    EXPECT_CALL(listener, restitutionWillChange(restitution, &body)).WillOnce(Return());
    EXPECT_CALL(listener, rotationWillChange(rotation, &body)).WillOnce(Return());
    EXPECT_CALL(listener, shapeTypeWillChange(shapeType, &body)).WillOnce(Return());
    EXPECT_CALL(listener, sizeWillChange(size, &body)).WillOnce(Return());
    EXPECT_CALL(listener, objectTypeWillChange(objectType, &body)).WillOnce(Return());
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

TEST(PMXModelTest, AddAndRemoveRigidBody)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IRigidBody> body(model.createRigidBody());
    ASSERT_EQ(-1, body->index());
    model.addRigidBody(0); /* should not be crashed */
    model.addRigidBody(body.get());
    model.addRigidBody(body.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.rigidBodies().count());
    ASSERT_EQ(body.get(), model.findRigidBodyRefAt(0));
    ASSERT_EQ(body->index(), model.findRigidBodyRefAt(0)->index());
    model.removeRigidBody(0); /* should not be crashed */
    model.removeRigidBody(body.get());
    ASSERT_EQ(0, model.rigidBodies().count());
    ASSERT_EQ(-1, body->index());
    MockIRigidBody mockedRigidBody;
    EXPECT_CALL(mockedRigidBody, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedRigidBody, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0))); /* should not be crashed */
    model.addRigidBody(&mockedRigidBody);
    ASSERT_EQ(0, model.rigidBodies().count());
}

TEST_P(PMXFragmentTest, ReadWriteRigidBody)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    RigidBody expected(0, &encoding), actual(0, &encoding);
    Bone bone(0);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    bone.setIndex(1);
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setBoneRef(&bone);
    expected.setAngularDamping(0.01);
    expected.setCollisionGroupID(1);
    expected.setCollisionMask(2);
    expected.setFriction(0.11);
    expected.setLinearDamping(0.21);
    expected.setMass(0.31);
    expected.setPosition(Vector3(0.41, 0.42, 0.43));
    expected.setRestitution(0.51);
    expected.setRotation(Vector3(0.61, 0.62, 0.63));
    expected.setShapeType(RigidBody::kCapsureShape);
    expected.setSize(Vector3(0.71, 0.72, 0.73));
    expected.setObjectType(RigidBody::kAlignedObject);
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareRigidBody(expected, actual, bone));
}

#include "Common.h"

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

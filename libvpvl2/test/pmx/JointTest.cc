#include "Common.h"

TEST_P(PMXFragmentTest, ReadWriteJoint)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    Joint expected(0), actual(0);
    RigidBody body(0, &encoding), body2(0, &encoding);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.rigidBodyIndexSize = indexSize;
    // construct joint
    body.setIndex(0);
    body2.setIndex(1);
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setRigidBody1Ref(&body);
    expected.setRigidBody2Ref(&body2);
    expected.setPosition(Vector3(0.01, 0.02, 0.03));
    expected.setRotation(Vector3(0.11, 0.12, 0.13));
    expected.setPositionLowerLimit(Vector3(0.21, 0.22, 0.23));
    expected.setRotationLowerLimit(Vector3(0.31, 0.32, 0.33));
    expected.setPositionUpperLimit(Vector3(0.41, 0.42, 0.43));
    expected.setRotationUpperLimit(Vector3(0.51, 0.52, 0.53));
    expected.setPositionStiffness(Vector3(0.61, 0.62, 0.63));
    expected.setRotationStiffness(Vector3(0.71, 0.72, 0.73));
    // write constructed joint and read it
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareJoint(expected, actual, body, body2));
}

TEST(PMXModelTest, AddAndRemoveJoint)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IJoint> joint(model.createJoint());
    ASSERT_EQ(-1, joint->index());
    model.addJoint(0); /* should not be crashed */
    model.addJoint(joint.get());
    model.addJoint(joint.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.joints().count());
    ASSERT_EQ(joint.get(), model.findJointRefAt(0));
    ASSERT_EQ(joint->index(), model.findJointRefAt(0)->index());
    model.removeJoint(0); /* should not be crashed */
    model.removeJoint(joint.get());
    ASSERT_EQ(0, model.joints().count());
    ASSERT_EQ(-1, joint->index());
    MockIJoint mockedJoint;
    EXPECT_CALL(mockedJoint, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedJoint, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addJoint(&mockedJoint);
    ASSERT_EQ(0, model.joints().count());
}

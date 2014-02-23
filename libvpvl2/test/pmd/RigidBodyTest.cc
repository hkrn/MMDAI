#include "Common.h"

TEST(PMDModelTest, RemoveRigidBodyReferences)
{
    Encoding encoding(0);
    Model model(&encoding);
    RigidBody rigidBody(&model, &encoding);
    model.addRigidBody(&rigidBody);
    Joint joint(&model, &encoding);
    joint.setRigidBody1Ref(&rigidBody);
    joint.setRigidBody2Ref(&rigidBody);
    model.addJoint(&joint);
    model.removeRigidBody(&rigidBody);
    model.removeJoint(&joint);
    ASSERT_EQ(static_cast<IRigidBody *>(0), joint.rigidBody1Ref());
    ASSERT_EQ(static_cast<IRigidBody *>(0), joint.rigidBody2Ref());
}

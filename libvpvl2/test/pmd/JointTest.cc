#include "Common.h"

TEST(PMDPropertyEventListener, HandleJointPropertyEvents)
{
    Joint joint(0, 0);
    MockJointPropertyEventListner listener;
    TestHandleEvents<IJoint::PropertyEventListener>(listener, joint);
    String japaneseName("Japanese"), englishName("English");
    Vector3 position(0.5, 1, 1.5), lowerV(1, 2, 3), upperV(4, 5, 6), stiffnessV(7, 8, 9),
            rotation(0.25, 0.5, 0.75), lowerQ(0.1, 0.2, 0.3), upperQ(0.4, 0.5, 0.6), stiffnessQ(0.7, 0.8, 0.9);
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionLowerLimitWillChange(lowerV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionStiffnessWillChange(stiffnessV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionUpperLimitWillChange(upperV, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, positionWillChange(position, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationLowerLimitWillChange(lowerQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationStiffnessWillChange(stiffnessQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationUpperLimitWillChange(upperQ, &joint)).WillOnce(Return());
    EXPECT_CALL(listener, rotationWillChange(rotation, &joint)).WillOnce(Return());
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

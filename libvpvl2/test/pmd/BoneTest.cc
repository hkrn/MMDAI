#include "Common.h"

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
    EXPECT_CALL(mockedBone, name(_)).WillRepeatedly(Return(static_cast<const IString *>(0)));  /* should not be crashed */
    EXPECT_CALL(mockedBone, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addBone(&mockedBone);
    ASSERT_EQ(0, model.bones().count());
}

TEST(PMDModelTest, RemoveBoneReferences)
{
    Encoding encoding(0);
    Model model(&encoding);
    Bone bone(&model, &encoding), childBone(&model, &encoding);
    String s("testBone");
    bone.setName(&s, IEncoding::kDefaultLanguage);
    model.addBone(&bone);
    ASSERT_EQ(&bone, model.findBoneRef(&s));
    childBone.setParentBoneRef(&bone);
    childBone.setParentInherentBoneRef(&bone);
    childBone.setDestinationOriginBoneRef(&bone);
    model.addBone(&childBone);
    Vertex vertex(&model);
    for (int i = 0; i < Vertex::kMaxBones; i++) {
        vertex.setBoneRef(i, &bone);
    }
    model.addVertex(&vertex);
    RigidBody body(&model, &encoding);
    body.setBoneRef(&bone);
    model.addRigidBody(&body);
    model.removeBone(&bone);
    model.removeBone(&childBone);
    model.removeRigidBody(&body);
    model.removeVertex(&vertex);
    ASSERT_EQ(Factory::sharedNullBoneRef(), body.boneRef());
    for (int i = 0; i < Vertex::kMaxBones; i++) {
        ASSERT_EQ(Factory::sharedNullBoneRef(), vertex.boneRef(i));
    }
    ASSERT_EQ(static_cast<IBone *>(0), childBone.parentBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), childBone.parentInherentBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), childBone.destinationOriginBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), model.findBoneRef(&s));
}

TEST_P(PMDLanguageTest, RenameBone)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IBone> bone(model.createBone());
    String oldName("OldBoneName"), newName("NewBoneName");
    IEncoding::LanguageType language = GetParam();
    bone->setName(&oldName, language);
    model.addBone(bone.get());
    ASSERT_EQ(bone.get(), model.findBoneRef(&oldName));
    ASSERT_EQ(0, model.findBoneRef(&newName));
    bone->setName(&newName, language);
    ASSERT_EQ(0, model.findBoneRef(&oldName));
    ASSERT_EQ(bone.get(), model.findBoneRef(&newName));
    bone.release();
}

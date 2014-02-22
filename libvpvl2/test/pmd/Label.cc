#include "Common.h"

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

TEST(PMDLabelTest, AddAndRemoveBone)
{
    Encoding encoding(0);
    Model model(&encoding);
    Label label(&model, &encoding, reinterpret_cast<const uint8 *>(""), Label::kBoneCategoryLabel);
    Bone bone(&model, &encoding);
    label.addBoneRef(&bone);
    ASSERT_EQ(1, label.count());
    ASSERT_EQ(&bone, label.boneRef(0));
    ASSERT_EQ(static_cast<IMorph *>(0), label.morphRef(0));
    label.removeBoneRef(&bone);
    ASSERT_EQ(0, label.count());
}

TEST(PMDLabelTest, AddAndRemoveMorph)
{
    Encoding encoding(0);
    Model model(&encoding);
    Label label(&model, &encoding, reinterpret_cast<const uint8 *>(""), Label::kMorphCategoryLabel);
    Morph morph(&model, &encoding);
    label.addMorphRef(&morph);
    ASSERT_EQ(1, label.count());
    ASSERT_EQ(static_cast<IBone *>(0), label.boneRef(0));
    ASSERT_EQ(&morph, label.morphRef(0));
    label.removeMorphRef(&morph);
    ASSERT_EQ(0, label.count());
}

TEST(PMDLabelTest, AddAndRemoveBoneReference)
{
    Encoding encoding(0);
    Model model(&encoding);
    Label label(&model, &encoding, reinterpret_cast<const uint8 *>(""), Label::kBoneCategoryLabel);
    std::unique_ptr<Bone> bone(new Bone(&model, &encoding));
    label.addBoneRef(bone.get());
    ASSERT_EQ(1, label.count());
    ASSERT_EQ(bone.get(), label.boneRef(0));
    ASSERT_EQ(static_cast<IMorph *>(0), label.morphRef(0));
    bone.reset();
    ASSERT_EQ(0, label.count());
}

TEST(PMDLabelTest, AddAndRemoveMorphReference)
{
    Encoding encoding(0);
    Model model(&encoding);
    Label label(&model, &encoding, reinterpret_cast<const uint8 *>(""), Label::kMorphCategoryLabel);
    std::unique_ptr<Morph> morph(new Morph(&model, &encoding));
    label.addMorphRef(morph.get());
    ASSERT_EQ(1, label.count());
    ASSERT_EQ(static_cast<IBone *>(0), label.boneRef(0));
    ASSERT_EQ(morph.get(), label.morphRef(0));
    morph.reset();
    ASSERT_EQ(0, label.count());
}


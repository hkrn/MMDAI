#include "Common.h"

TEST_P(PMXFragmentTest, ReadWriteBone)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Bone expected(&model), actual(&model), parent(&model), parentInherent(&model), effector(&model);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    // construct bone
    parent.setIndex(0);
    parentInherent.setIndex(1);
    effector.setIndex(2);
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setOrigin(Vector3(0.11, 0.12, 0.13));
    expected.setIndex(1);
    expected.setDestinationOrigin(Vector3(0.21, 0.22, 0.23));
    expected.setFixedAxis(Vector3(0.31, 0.32, 0.33));
    expected.setAxisX(Vector3(0.41, 0.42, 0.43));
    expected.setAxisZ(Vector3(0.51, 0.52, 0.53));
    expected.setExternalIndex(3);
    expected.setParentBoneRef(&parent);
    expected.setParentInherentBoneRef(&parentInherent);
    expected.setInherentCoefficient(0.61);
    expected.setRotateable(true);
    expected.setMovable(true);
    expected.setVisible(true);
    expected.setInteractive(true);
    expected.setInherentOrientationEnable(true);
    expected.setInherentTranslationEnable(true);
    expected.setFixedAxisEnable(true);
    expected.setLocalAxesEnable(true);
    expected.setTransformAfterPhysicsEnable(true);
    expected.setTransformedByExternalParentEnable(true);
    // write constructed bone and read it
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    // compare read bone
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareBone(expected, actual));
    Array<Bone *> bones;
    bones.append(&parent);
    bones.append(&parentInherent);
    bones.append(&effector);
    bones.append(&actual);
    Bone::loadBones(bones);
    ASSERT_EQ(&parent, actual.parentBoneRef());
    ASSERT_EQ(&parentInherent, actual.parentInherentBoneRef());
}

TEST_P(PMXLanguageTest, RenameBone)
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

TEST(PMXBoneTest, DefaultFlags)
{
    Bone bone(0);
    ASSERT_FALSE(bone.isMovable());
    ASSERT_FALSE(bone.isRotateable());
    ASSERT_FALSE(bone.isVisible());
    ASSERT_FALSE(bone.isInteractive());
    ASSERT_FALSE(bone.hasInverseKinematics());
    ASSERT_FALSE(bone.isInherentTranslationEnabled());
    ASSERT_FALSE(bone.isInherentOrientationEnabled());
    ASSERT_FALSE(bone.hasFixedAxes());
    ASSERT_FALSE(bone.hasLocalAxes());
    ASSERT_FALSE(bone.isTransformedAfterPhysicsSimulation());
    ASSERT_FALSE(bone.isTransformedByExternalParent());
}

TEST(PMXModelTest, AddAndRemoveBone)
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
    EXPECT_CALL(mockedBone, name(_)).Times(2).WillRepeatedly(Return(static_cast<const IString *>(0)));
    EXPECT_CALL(mockedBone, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addBone(&mockedBone);
    ASSERT_EQ(0, model.bones().count());
}

TEST(PMXModelTest, RemoveBoneReferences)
{
    Encoding encoding(0);
    Model model(&encoding);
    Bone bone(&model), childBone(&model);
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
    Morph parentBoneMorph(&model);
    Morph::Bone boneMorph;
    boneMorph.bone = &bone;
    parentBoneMorph.setType(IMorph::kBoneMorph);
    parentBoneMorph.addBoneMorph(&boneMorph);
    model.addMorph(&parentBoneMorph);
    model.removeBone(&bone);
    model.removeBone(&childBone);
    model.removeRigidBody(&body);
    model.removeVertex(&vertex);
    parentBoneMorph.removeBoneMorph(&boneMorph);
    model.removeMorph(&parentBoneMorph);
    ASSERT_EQ(Factory::sharedNullBoneRef(), body.boneRef());
    for (int i = 0; i < Vertex::kMaxBones; i++) {
        ASSERT_EQ(Factory::sharedNullBoneRef(), vertex.boneRef(i));
    }
    ASSERT_EQ(static_cast<IBone *>(0), boneMorph.bone);
    ASSERT_EQ(static_cast<IBone *>(0), childBone.parentBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), childBone.parentInherentBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), childBone.destinationOriginBoneRef());
    ASSERT_EQ(static_cast<IBone *>(0), model.findBoneRef(&s));
}

#include "Common.h"
#include "vpvl2/Factory.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

TEST(FactoryTest, CreateEmptyModels)
{
    Encoding encoding;
    Factory factory(&encoding);
    QScopedPointer<IModel> pmd(factory.createModel(IModel::kPMD));
    ASSERT_TRUE(dynamic_cast<pmd::Model *>(pmd.data()));
    QScopedPointer<IModel> pmx(factory.createModel(IModel::kPMX));
    ASSERT_TRUE(dynamic_cast<pmx::Model *>(pmx.data()));
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    ASSERT_TRUE(dynamic_cast<asset::Model *>(asset.data()));
}

TEST(FactoryTest, CreateEmptyMotions)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIModel model;
    QScopedPointer<IMotion> vmd(factory.createMotion(IMotion::kVMD, &model));
    ASSERT_TRUE(dynamic_cast<vmd::Motion *>(vmd.data()));
    QScopedPointer<IMotion> mvd(factory.createMotion(IMotion::kMVD, &model));
    ASSERT_TRUE(dynamic_cast<mvd::Motion *>(mvd.data()));
}

TEST(FactoryTest, CreateEmptyBoneKeyframes)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createBoneKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.createMotion(IMotion::kVMD, &model));
    QScopedPointer<IBoneKeyframe> vbk(factory.createBoneKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::BoneKeyframe *>(vbk.data()));
    QScopedPointer<IMotion> mvd(factory.createMotion(IMotion::kMVD, &model));
    QScopedPointer<IBoneKeyframe> mbk(factory.createBoneKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::BoneKeyframe *>(mbk.data()));
}

TEST(FactoryTest, CreateEmptyCameraKeyframes)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createCameraKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.createMotion(IMotion::kVMD, &model));
    QScopedPointer<ICameraKeyframe> vck(factory.createCameraKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::CameraKeyframe *>(vck.data()));
    QScopedPointer<IMotion> mvd(factory.createMotion(IMotion::kMVD, &model));
    QScopedPointer<ICameraKeyframe> mck(factory.createCameraKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::CameraKeyframe *>(mck.data()));
}

TEST(FactoryTest, CreateEmptyLightKeyframes)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createLightKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.createMotion(IMotion::kVMD, &model));
    QScopedPointer<ILightKeyframe> vlk(factory.createLightKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::LightKeyframe *>(vlk.data()));
    QScopedPointer<IMotion> mvd(factory.createMotion(IMotion::kMVD, &model));
    QScopedPointer<ILightKeyframe> mlk(factory.createLightKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::LightKeyframe *>(mlk.data()));
}

TEST(FactoryTest, CreateEmptyMorphKeyframes)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIModel model;
    ASSERT_FALSE(factory.createMorphKeyframe(0));
    QScopedPointer<IMotion> vmd(factory.createMotion(IMotion::kVMD, &model));
    QScopedPointer<IMorphKeyframe> vmk(factory.createMorphKeyframe(vmd.data()));
    ASSERT_TRUE(dynamic_cast<vmd::MorphKeyframe *>(vmk.data()));
    QScopedPointer<IMotion> mvd(factory.createMotion(IMotion::kMVD, &model));
    QScopedPointer<IMorphKeyframe> mmk(factory.createMorphKeyframe(mvd.data()));
    ASSERT_TRUE(dynamic_cast<mvd::MorphKeyframe *>(mmk.data()));
}

class MotionConversionTest : public TestWithParam< tuple<QString, IMotion::Type > > {};

ACTION_P(FindBone, bones)
{
    MockIBone *bone = new MockIBone();
    EXPECT_CALL(*bone, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    bones->append(bone);
    return bone;
}

ACTION_P(FindMorph, morphs)
{
    MockIMorph *morph = new MockIMorph();
    EXPECT_CALL(*morph, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    morphs->append(morph);
    return morph;
}

TEST_P(MotionConversionTest, ConvertModelMotion)
{
    QFile file("motion." + get<0>(GetParam()));
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Factory factory(&encoding);
        MockIModel model;
        QList<IBone *> bones;
        QList<IMorph *> morphs;
        EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(FindBone(&bones));
        EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(FindMorph(&morphs));
        bool ok;
        QScopedPointer<IMotion> source(factory.createMotion(data, size, &model, ok));
        ASSERT_TRUE(ok);
        IMotion::Type type = get<1>(GetParam());
        QScopedPointer<IMotion> dest(factory.convertMotion(source.data(), type));
        ASSERT_EQ(dest->type(), type);
        ASSERT_EQ(source->countKeyframes(IKeyframe::kBone), dest->countKeyframes(IKeyframe::kBone));
        ASSERT_EQ(source->countKeyframes(IKeyframe::kMorph), dest->countKeyframes(IKeyframe::kMorph));
        qDeleteAll(bones);
        qDeleteAll(morphs);
    }
}

TEST_P(MotionConversionTest, ConvertCameraMotion)
{
    QFile file("camera." + get<0>(GetParam()));
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Factory factory(&encoding);
        bool ok;
        QScopedPointer<IMotion> source(factory.createMotion(data, size, 0, ok));
        ASSERT_TRUE(ok);
        IMotion::Type type = get<1>(GetParam());
        QScopedPointer<IMotion> dest(factory.convertMotion(source.data(), type));
        ASSERT_EQ(dest->type(), type);
        ASSERT_EQ(source->countKeyframes(IKeyframe::kCamera), dest->countKeyframes(IKeyframe::kCamera));
    }
}

INSTANTIATE_TEST_CASE_P(FactoryInstance, MotionConversionTest,
                        Combine(Values("vmd", "mvd"), Values(IMotion::kVMD, IMotion::kMVD)));

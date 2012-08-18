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
#include "mock/Model.h"

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

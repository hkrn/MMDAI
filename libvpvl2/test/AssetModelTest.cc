#include "Common.h"

#include <btBulletDynamicsCommon.h>

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/asset/Model.h"

#include "mock/Bone.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::asset;
using namespace vpvl2::extensions::icu4c;

TEST(AssetModelTest, RootBonePosition)
{
    Vector3 expected1(1, 2, 3), expected2(4, 5, 6);
    Encoding::Dictionary dict;
    String s("rootBone");
    dict.insert(IEncoding::kRootBone, &s);
    Encoding encoding(&dict);
    asset::Model model(&encoding);
    model.load(0, 0); // getting root bone reference
    model.setWorldTranslation(expected1);
    ASSERT_TRUE(CompareVector(expected1, model.worldTranslation()));
    IBone *boneRef = model.findBoneRef(&s);
    ASSERT_TRUE(boneRef);
    boneRef->setLocalTranslation(expected2);
    ASSERT_TRUE(CompareVector(expected2, model.worldTranslation()));
}

TEST(AssetModelTest, RootBoneRotation)
{
    Quaternion expected1(0.1, 0.2, 0.3, 0.4), expected2(0.5, 0.6, 0.7, 0.8);
    Encoding::Dictionary dict;
    String s("rootBone");
    dict.insert(IEncoding::kRootBone, &s);
    Encoding encoding(&dict);
    asset::Model model(&encoding);
    model.load(0, 0); // getting root bone reference
    model.setWorldOrientation(expected1);
    ASSERT_TRUE(CompareVector(expected1, model.worldOrientation()));
    IBone *boneRef = model.findBoneRef(&s);
    ASSERT_TRUE(boneRef);
    boneRef->setLocalOrientation(expected2);
    ASSERT_TRUE(CompareVector(expected2, model.worldOrientation()));
}

TEST(AssetModelTet, ScaleBone)
{
    Encoding::Dictionary dict;
    String s("scaleBone");
    dict.insert(IEncoding::kScaleBoneAsset, &s);
    Encoding encoding(&dict);
    asset::Model model(&encoding);
    model.load(0, 0); // getting scale bone reference
    model.setScaleFactor(0.5);
    ASSERT_FLOAT_EQ(0.5, model.scaleFactor());
    IBone *boneRef = model.findBoneRef(&s);
    ASSERT_TRUE(boneRef);
    boneRef->setLocalTranslation(Vector3(0.1, 0.3, 0.5));
    ASSERT_FLOAT_EQ(0.3, model.scaleFactor());
}

TEST(AssetModelTest, OpacityMorph)
{
    Scalar expected1(0.1), expected2(0.2);
    Encoding::Dictionary dict;
    String s("opacity");
    dict.insert(IEncoding::kOpacityMorphAsset, &s);
    Encoding encoding(&dict);
    asset::Model model(&encoding);
    model.load(0, 0); // getting opacity morph reference
    model.setOpacity(expected1);
    ASSERT_FLOAT_EQ(expected1, model.opacity());
    IMorph *morphRef = model.findMorphRef(&s);
    ASSERT_TRUE(morphRef);
    morphRef->setWeight(expected2);
    ASSERT_FLOAT_EQ(expected2, model.opacity());
}

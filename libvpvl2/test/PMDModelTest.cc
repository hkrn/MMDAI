#include "Common.h"

#include <btBulletDynamicsCommon.h>

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"

#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Vertex.h"
using namespace vpvl2::pmd;
#else
#include "vpvl2/pmd2/Model.h"
using namespace vpvl2::pmd2;
#endif

#include "mock/Bone.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

#ifdef VPVL2_LINK_VPVL

TEST(VertexTest, PerformSkinningBdef2WeightZeroPMDCompat)
{
    MockIBone bone1, bone2;
    //Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    //EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(0);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(4.025, 5.05, 6.075), position));
    ASSERT_TRUE(CompareVector(Vector3(0.1, 0.125, 0.15), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightOnePMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    //Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    //EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(100);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.075, 2.15, 3.225), position));
    ASSERT_TRUE(CompareVector(Vector3(0.3, 0.375, 0.45), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightHalfPMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.append(&bone1);
    bones.append(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(50);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    const Vector3 &v2 = (Vector3(1.075, 2.15, 3.225) + Vector3(4.025, 5.05, 6.075)) * 0.5;
    const Vector3 &n2 = (Vector3(0.1, 0.125, 0.15) + Vector3(0.3, 0.375, 0.45)) * 0.5;
    ASSERT_TRUE(CompareVector(v2, position));
    ASSERT_TRUE(CompareVector(n2, normal));
}

#endif

TEST(PMDModelTest, ParseRealPMD)
{
    QFile file("miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding::Dictionary dict;
        Encoding encoding(&dict);
        Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMDModel, model.type());
    }
    else {
        // skip
    }
}

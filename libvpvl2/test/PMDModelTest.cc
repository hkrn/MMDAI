#include "Common.h"

#include <btBulletDynamicsCommon.h>

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"

#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Vertex.h"
using namespace vpvl2::pmd;
#else
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Material.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmd2/Vertex.h"
using namespace vpvl2::pmd2;
#endif

#include "mock/Bone.h"
#include "mock/Label.h"
#include "mock/Material.h"
#include "mock/Morph.h"
#include "mock/Vertex.h"

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

#else

TEST(PMDVertexTest, Boundary)
{
    Encoding encoding(0);
    Vertex vertex(0);
    QScopedPointer<Bone> bone(new Bone(0, &encoding));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight(Vertex::kMaxBones, 0.1);
    vertex.setBoneRef(-1, bone.data());
    vertex.setBoneRef(Vertex::kMaxBones, bone.data());
    ASSERT_EQ(vertex.boneRef(-1), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.boneRef(Vertex::kMaxBones), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.weight(-1), 0.0f);
    ASSERT_EQ(vertex.weight(Vertex::kMaxBones), 0.0f);
}

TEST(PMDVertexTest, NullRef)
{
    Encoding encoding(0);
    Vertex vertex(0);
    QScopedPointer<Bone> bone(new Bone(0, &encoding));
    QScopedPointer<Material> material(new Material(0, &encoding));
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
    vertex.setBoneRef(0, bone.data());
    vertex.setBoneRef(0, 0);
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    vertex.setMaterial(material.data());
    vertex.setMaterial(0);
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
}

#endif

TEST(PMDModelTest, AddAndRemoveBone)
{
    Encoding encoding(0);
    Model model(&encoding);
    QScopedPointer<IBone> bone(model.createBone());
    ASSERT_EQ(-1, bone->index());
    model.addBone(0); /* should not be crashed */
    model.addBone(bone.data());
    model.addBone(bone.data()); /* no effect because it's already added */
    ASSERT_EQ(1, model.bones().count());
    ASSERT_EQ(bone.data(), model.findBoneRefAt(0));
    ASSERT_EQ(bone->index(), model.findBoneRefAt(0)->index());
    model.removeBone(0); /* should not be crashed */
    model.removeBone(bone.data());
    ASSERT_EQ(0, model.bones().count());
    ASSERT_EQ(-1, bone->index());
    MockIBone mockedBone;
    EXPECT_CALL(mockedBone, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedBone, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addBone(&mockedBone);
    ASSERT_EQ(0, model.bones().count());
}

TEST(PMDModelTest, AddAndRemoveLabel)
{
    Encoding encoding(0);
    Model model(&encoding);
    QScopedPointer<ILabel> label(model.createLabel());
    ASSERT_EQ(-1, label->index());
    model.addLabel(0); /* should not be crashed */
    model.addLabel(label.data());
    model.addLabel(label.data()); /* no effect because it's already added */
    ASSERT_EQ(1, model.labels().count());
    ASSERT_EQ(label.data(), model.findLabelRefAt(0));
    ASSERT_EQ(label->index(), model.findLabelRefAt(0)->index());
    model.removeLabel(0); /* should not be crashed */
    model.removeLabel(label.data());
    ASSERT_EQ(0, model.labels().count());
    ASSERT_EQ(-1, label->index());
    MockILabel mockedLabel;
    EXPECT_CALL(mockedLabel, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedLabel, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addLabel(&mockedLabel);
    ASSERT_EQ(0, model.labels().count());
}

TEST(PMDModelTest, AddAndRemoveMaterial)
{
    Encoding encoding(0);
    Model model(&encoding);
    QScopedPointer<IMaterial> material(model.createMaterial());
    ASSERT_EQ(-1, material->index());
    model.addMaterial(0); /* should not be crashed */
    model.addMaterial(material.data());
    model.addMaterial(material.data()); /* no effect because it's already added */
    ASSERT_EQ(1, model.materials().count());
    ASSERT_EQ(material.data(), model.findMaterialRefAt(0));
    ASSERT_EQ(material->index(), model.findMaterialRefAt(0)->index());
    model.removeMaterial(0); /* should not be crashed */
    model.removeMaterial(material.data());
    ASSERT_EQ(0, model.materials().count());
    ASSERT_EQ(-1, material->index());
    MockIMaterial mockedMaterial;
    EXPECT_CALL(mockedMaterial, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMaterial, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMaterial(&mockedMaterial);
    ASSERT_EQ(0, model.materials().count());
}

TEST(PMDModelTest, AddAndRemoveMorph)
{
    Encoding encoding(0);
    Model model(&encoding);
    QScopedPointer<IMorph> morph(model.createMorph());
    ASSERT_EQ(-1, morph->index());
    model.addMorph(0); /* should not be crashed */
    model.addMorph(morph.data());
    model.addMorph(morph.data()); /* no effect because it's already added */
    ASSERT_EQ(1, model.morphs().count());
    ASSERT_EQ(morph.data(), model.findMorphRefAt(0));
    ASSERT_EQ(morph->index(), model.findMorphRefAt(0)->index());
    model.removeMorph(0); /* should not be crashed */
    model.removeMorph(morph.data());
    ASSERT_EQ(0, model.morphs().count());
    ASSERT_EQ(-1, morph->index());
    MockIMorph mockedMorph;
    EXPECT_CALL(mockedMorph, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMorph, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMorph(&mockedMorph);
    ASSERT_EQ(0, model.morphs().count());
}

TEST(PMDModelTest, AddAndRemoveVertex)
{
    Encoding encoding(0);
    Model model(&encoding);
    QScopedPointer<IVertex> vertex(model.createVertex());
    ASSERT_EQ(-1, vertex->index());
    model.addVertex(0); /* should not be crashed */
    model.addVertex(vertex.data());
    model.addVertex(vertex.data()); /* no effect because it's already added */
    ASSERT_EQ(1, model.vertices().count());
    ASSERT_EQ(vertex.data(), model.findVertexRefAt(0));
    ASSERT_EQ(vertex->index(), model.findVertexRefAt(0)->index());
    model.removeVertex(0); /* should not be crashed */
    model.removeVertex(vertex.data());
    ASSERT_EQ(0, model.vertices().count());
    ASSERT_EQ(-1, vertex->index());
    MockIVertex mockedVertex;
    EXPECT_CALL(mockedVertex, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedVertex, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addVertex(&mockedVertex);
    ASSERT_EQ(0, model.vertices().count());
}

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

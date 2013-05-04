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
    ASSERT_EQ(bone.data(), model.findBoneAt(0));
    ASSERT_EQ(bone->index(), model.findBoneAt(0)->index());
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
    ASSERT_EQ(label.data(), model.findLabelAt(0));
    ASSERT_EQ(label->index(), model.findLabelAt(0)->index());
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
    ASSERT_EQ(material.data(), model.findMaterialAt(0));
    ASSERT_EQ(material->index(), model.findMaterialAt(0)->index());
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
    ASSERT_EQ(morph.data(), model.findMorphAt(0));
    ASSERT_EQ(morph->index(), model.findMorphAt(0)->index());
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
    ASSERT_EQ(vertex.data(), model.findVertexAt(0));
    ASSERT_EQ(vertex->index(), model.findVertexAt(0)->index());
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

        QByteArray bytes2;
        bytes2.resize(model.estimateSize());;
        size_t written;
        model.save(reinterpret_cast<uint8_t *>(bytes2.data()), written);
        QFile file2(QDir::home().absoluteFilePath(QFileInfo(file.fileName()).fileName()));
        qDebug() << file2.fileName() << file.size() << model.estimateSize() << written;
        file2.open(QFile::WriteOnly);
        file2.write(bytes2);
        QFile file3(file2.fileName());
        file3.open(QFile::ReadOnly);
        const QByteArray &bytes3 = file3.readAll();
        pmd2::Model model2(&encoding);
        qDebug() << "result:" << model2.load(reinterpret_cast<const uint8_t *>(bytes3.constData()), bytes3.size())
                 << model2.error() << "estimated:" << model.estimateSize() << "actual:" << written;
    }
    else {
        // skip
    }
}

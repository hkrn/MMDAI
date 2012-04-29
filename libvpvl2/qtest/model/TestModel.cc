#include <QtTest/QtTest>
#include <btBulletDynamicsCommon.h>
#include <vpvl2/internal/util.h>

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

#include "../common.h"

using namespace vpvl2::pmx;

class TestModel : public QObject
{
    Q_OBJECT

public:
    static const char *kTestString;
    static const uint8_t *kName;
    static const uint8_t *kEnglishName;

    TestModel();

private:
    static void compare(const Vector3 &actual, const Vector3 &expected);
    static void compare(const Vector4 &actual, const Vector4 &expected);
    static void compare(const Quaternion &actual, const Quaternion &expected);
    static void setVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones);
    static void compareVertex(const Vertex &vertex, const Vertex &vertex2, const Array<Bone *> &bones);

    static void testReadWriteBone(size_t indexSize);
    static void testReadWriteJoint(size_t indexSize);
    static void testReadWriteMaterial(size_t indexSize);
    static void testReadWriteBoneMorph(size_t indexSize);
    static void testReadWriteGroupMorph(size_t indexSize);
    static void testReadWriteMaterialMorph(size_t indexSize);
    static void testReadWriteUVMorph(size_t indexSize, pmx::Morph::Type type);
    static void testReadWriteRigidBody(size_t indexSize);
    static void testReadWriteVertexMorph(size_t indexSize);
    static void testReadWriteVertexBdef1(size_t indexSize);
    static void testReadWriteVertexBdef2(size_t indexSize);
    static void testReadWriteVertexBdef4(size_t indexSize);
    static void testReadWriteVertexSdef(size_t indexSize);

private Q_SLOTS:
    void parseEmpty() const;
    void parseFile() const;
    void testBoneDefaultFlags() const;
    void testVertexBoundary() const;
    void testMaterialMorphMergeAmbient() const;
    void testMaterialMorphMergeDiffuse() const;
    void testMaterialMorphMergeSpecular() const;
    void testMaterialMorphMergeShininess() const;
    void testMaterialMorphMergeEdgeColor() const;
    void testMaterialMorphMergeEdgeSize() const;
    void testReadWriteBoneIndexSize1() const { testReadWriteBone(1); }
    void testReadWriteBoneIndexSize2() const { testReadWriteBone(2); }
    void testReadWriteBoneIndexSize4() const { testReadWriteBone(4); }
    void testReadWriteJointIndexSize1() const { testReadWriteJoint(1); }
    void testReadWriteJointIndexSize2() const { testReadWriteJoint(2); }
    void testReadWriteJointIndexSize4() const { testReadWriteJoint(4); }
    void testReadWriteMaterialIndexSize1() const { testReadWriteMaterial(1); }
    void testReadWriteMaterialIndexSize2() const { testReadWriteMaterial(2); }
    void testReadWriteMaterialIndexSize4() const { testReadWriteMaterial(4); }
    void testReadWriteBoneMorphIndexSize1() const { testReadWriteBoneMorph(1); }
    void testReadWriteBoneMorphIndexSize2() const { testReadWriteBoneMorph(2); }
    void testReadWriteBoneMorphIndexSize4() const { testReadWriteBoneMorph(4); }
    void testReadWriteGroupMorphIndexSize1() const { testReadWriteGroupMorph(1); }
    void testReadWriteGroupMorphIndexSize2() const { testReadWriteGroupMorph(2); }
    void testReadWriteGroupMorphIndexSize4() const { testReadWriteGroupMorph(4); }
    void testReadWriteMaterialMorphIndexSize1() const { testReadWriteMaterialMorph(1); }
    void testReadWriteMaterialMorphIndexSize2() const { testReadWriteMaterialMorph(2); }
    void testReadWriteMaterialMorphIndexSize4() const { testReadWriteMaterialMorph(4); }
    void testReadWriteRigidBodyIndexSize1() const { testReadWriteRigidBody(1); }
    void testReadWriteRigidBodyIndexSize2() const { testReadWriteRigidBody(2); }
    void testReadWriteRigidBodyIndexSize4() const { testReadWriteRigidBody(4); }
    void testReadWriteTexCoordMorphIndexSize1() const { testReadWriteUVMorph(1, pmx::Morph::kTexCoord); }
    void testReadWriteTexCoordMorphIndexSize2() const { testReadWriteUVMorph(2, pmx::Morph::kTexCoord); }
    void testReadWriteTexCoordMorphIndexSize4() const { testReadWriteUVMorph(4, pmx::Morph::kTexCoord); }
    void testReadWriteUVA1MorphIndexSize1() const { testReadWriteUVMorph(1, pmx::Morph::kUVA1); }
    void testReadWriteUVA1MorphIndexSize2() const { testReadWriteUVMorph(2, pmx::Morph::kUVA1); }
    void testReadWriteUVA1MorphIndexSize4() const { testReadWriteUVMorph(4, pmx::Morph::kUVA1); }
    void testReadWriteUVA2MorphIndexSize1() const { testReadWriteUVMorph(1, pmx::Morph::kUVA2); }
    void testReadWriteUVA2MorphIndexSize2() const { testReadWriteUVMorph(2, pmx::Morph::kUVA2); }
    void testReadWriteUVA2MorphIndexSize4() const { testReadWriteUVMorph(4, pmx::Morph::kUVA2); }
    void testReadWriteUVA3MorphIndexSize1() const { testReadWriteUVMorph(1, pmx::Morph::kUVA3); }
    void testReadWriteUVA3MorphIndexSize2() const { testReadWriteUVMorph(2, pmx::Morph::kUVA3); }
    void testReadWriteUVA3MorphIndexSize4() const { testReadWriteUVMorph(4, pmx::Morph::kUVA3); }
    void testReadWriteUVA4MorphIndexSize1() const { testReadWriteUVMorph(1, pmx::Morph::kUVA4); }
    void testReadWriteUVA4MorphIndexSize2() const { testReadWriteUVMorph(2, pmx::Morph::kUVA4); }
    void testReadWriteUVA4MorphIndexSize4() const { testReadWriteUVMorph(4, pmx::Morph::kUVA4); }
    void testReadWriteVertexMorphIndexSize1() const { testReadWriteVertexMorph(1); }
    void testReadWriteVertexMorphIndexSize2() const { testReadWriteVertexMorph(2); }
    void testReadWriteVertexMorphIndexSize4() const { testReadWriteVertexMorph(4); }
    void testReadWriteVertexBdef1IndexSize1() const { testReadWriteVertexBdef1(1); }
    void testReadWriteVertexBdef1IndexSize2() const { testReadWriteVertexBdef1(2); }
    void testReadWriteVertexBdef1IndexSize4() const { testReadWriteVertexBdef1(4); }
    void testReadWriteVertexBdef2IndexSize1() const { testReadWriteVertexBdef2(1); }
    void testReadWriteVertexBdef2IndexSize2() const { testReadWriteVertexBdef2(2); }
    void testReadWriteVertexBdef2IndexSize4() const { testReadWriteVertexBdef2(4); }
    void testReadWriteVertexBdef4IndexSize1() const { testReadWriteVertexBdef4(1); }
    void testReadWriteVertexBdef4IndexSize2() const { testReadWriteVertexBdef4(2); }
    void testReadWriteVertexBdef4IndexSize4() const { testReadWriteVertexBdef4(4); }
    void testReadWriteVertexSdefIndexSize1() const { testReadWriteVertexSdef(1); }
    void testReadWriteVertexSdefIndexSize2() const { testReadWriteVertexSdef(2); }
    void testReadWriteVertexSdefIndexSize4() const { testReadWriteVertexSdef(4); }
};

void TestModel::compare(const Vector3 &actual, const Vector3 &expected)
{
    QCOMPARE(actual.x(), expected.x());
    QCOMPARE(actual.y(), expected.y());
    QCOMPARE(actual.z(), expected.z());
}

void TestModel::compare(const Vector4 &actual, const Vector4 &expected)
{
    QCOMPARE(actual.x(), expected.x());
    QCOMPARE(actual.y(), expected.y());
    QCOMPARE(actual.z(), expected.z());
    QCOMPARE(actual.w(), expected.w());
}

void TestModel::compare(const Quaternion &actual, const Quaternion &expected)
{
    QCOMPARE(actual.x(), expected.x());
    QCOMPARE(actual.y(), expected.y());
    QCOMPARE(actual.z(), expected.z());
    QCOMPARE(actual.w(), expected.w());
}

void TestModel::setVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones)
{
    vertex.setOrigin(Vector3(0.01, 0.02, 0.03));
    vertex.setNormal(Vector3(0.11, 0.12, 0.13));
    vertex.setTexCoord(Vector3(0.21, 0.22, 0.0));
    vertex.setUV(0, Vector4(0.31, 0.32, 0.33, 0.34));
    vertex.setType(type);
    vertex.setEdgeSize(0.1);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vertex.setBone(i, bones[i]);
        vertex.setWeight(i, 0.2 + 0.1 * i);
    }
    vertex.setSdefC(Vector3(0.41, 0.42, 0.43));
    vertex.setSdefR0(Vector3(0.51, 0.52, 0.53));
    vertex.setSdefR1(Vector3(0.61, 0.62, 0.63));
}

void TestModel::compareVertex(const Vertex &vertex, const Vertex &vertex2, const Array<Bone *> &bones)
{
    compare(vertex2.origin(), vertex.origin());
    compare(vertex2.normal(), vertex.normal());
    compare(vertex2.texcoord(), vertex.texcoord());
    compare(vertex2.uv(0), vertex.uv(0));
    compare(kZeroV4, vertex.uv(1));
    QCOMPARE(vertex2.type(), vertex.type());
    QCOMPARE(vertex2.edgeSize(), vertex.edgeSize());
    if (vertex.type() == Vertex::kSdef) {
        compare(vertex2.sdefC(), vertex.sdefC());
        compare(vertex2.sdefR0(), vertex.sdefR0());
        compare(vertex2.sdefR1(), vertex.sdefR1());
    }
    else {
        compare(kZeroV3, vertex2.sdefC());
        compare(kZeroV3, vertex2.sdefR0());
        compare(kZeroV3, vertex2.sdefR1());
    }
    Array<Vertex *> vertices;
    vertices.add(const_cast<Vertex *>(&vertex));
    Vertex::loadVertices(vertices, bones);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        QCOMPARE(bones[i], vertex.bone(i));
        if (nbones == 4)
            QCOMPARE(0.2f + 0.1f * i, vertex2.weight(i));
    }
    if (nbones == 2)
        QCOMPARE(0.2f, vertex2.weight(0));
}

TestModel::TestModel()
{
}

void TestModel::parseEmpty() const
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    QVERIFY(!model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(model.error(), Model::kInvalidHeaderError);
}

void TestModel::parseFile() const
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    QFile file("miku.pmx");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        const size_t size = file.size();
        QVERIFY(model.preparse(reinterpret_cast<const uint8_t *>(data), size, info));
        model.load(reinterpret_cast<const uint8_t *>(data), size);
#if 0
        QTextCodec *codec = QTextCodec::codecForName("UTF-16");
        qDebug() << "Model#name()" << codec->toUnicode(model.name()->ptr(), model.name()->length());
        qDebug() << "Model#englishName()" << codec->toUnicode(model.englishName()->ptr(), model.englishName()->length());
        qDebug() << "Model#comment()" << codec->toUnicode(model.comment()->ptr(), model.comment()->length());
        qDebug() << "Model#englishComment()" << codec->toUnicode(model.englishComment()->ptr(), model.englishComment()->length());
        const Array<StaticString *> &textures = model.textures();
        for (int i = 0; i < textures.count(); i++) {
            StaticString *texture = textures[i];
            qDebug() << "Texture#name()" << codec->toUnicode(texture->ptr(), texture->length());
        }
        const Array<Bone *> &bones = model.bones();
        for (int i = 0; i < bones.count(); i++) {
            Bone *bone = bones[i];
            qDebug() << "Bone#name()" << codec->toUnicode(bone->name()->ptr(), bone->name()->length());
            qDebug() << "Bone#englishName()" << codec->toUnicode(bone->englishName()->ptr(), bone->englishName()->length());
        }
        const Array<Morph *> &morphs = model.morphs();
        for (int i = 0; i < morphs.count(); i++) {
            Morph *morph = morphs[i];
            qDebug() << "Bone#name()" << codec->toUnicode(morph->name()->ptr(), morph->name()->length());
            qDebug() << "Bone#englishName()" << codec->toUnicode(morph->englishName()->ptr(), morph->englishName()->length());
        }
        const Array<RigidBody *> &rigidBodies = model.rigidBodies();
        for (int i = 0; i < rigidBodies.count(); i++) {
            RigidBody *rigidBody = rigidBodies[i];
            qDebug() << "RigidBody#name()" << codec->toUnicode(rigidBody->name()->ptr(), rigidBody->name()->length());
            qDebug() << "RigidBody#englishName()" << codec->toUnicode(rigidBody->englishName()->ptr(), rigidBody->englishName()->length());
        }
        const Array<Joint *> &joints = model.joints();
        for (int i = 0; i < joints.count(); i++) {
            Joint *joint = joints[i];
            qDebug() << "Joint#name()" << codec->toUnicode(joint->name()->ptr(), joint->name()->length());
            qDebug() << "Joint#englishName()" << codec->toUnicode(joint->englishName()->ptr(), joint->englishName()->length());
        }
#endif
    }
    else {
        QSKIP("Require a model to test this", SkipSingle);
    }
}

void TestModel::testBoneDefaultFlags() const
{
    Bone bone;
    QVERIFY(!bone.isMovable());
    QVERIFY(!bone.isRotateable());
    QVERIFY(!bone.isVisible());
    QVERIFY(!bone.isInteractive());
    QVERIFY(!bone.hasInverseKinematics());
    QVERIFY(!bone.hasPositionInherence());
    QVERIFY(!bone.hasRotationInherence());
    QVERIFY(!bone.hasFixedAxes());
    QVERIFY(!bone.hasLocalAxes());
    QVERIFY(!bone.isTransformedAfterPhysicsSimulation());
    QVERIFY(!bone.isTransformedByExternalParent());
}

void TestModel::testVertexBoundary() const
{
    Vertex vertex;
    Bone *bone = new Bone();
    vertex.setUV(-1, Vector4(1, 1, 1, 1));
    vertex.setUV( 4, Vector4(1, 1, 1, 1));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight( 4, 0.1);
    vertex.setBone(-1, bone);
    vertex.setBone( 4, bone);
    QCOMPARE(0.0f, vertex.uv(-1).x());
    QCOMPARE(0.0f, vertex.uv(4).x());
    QCOMPARE(static_cast<IBone *>(0), vertex.bone(-1));
    QCOMPARE(static_cast<IBone *>(0), vertex.bone(4));
    QCOMPARE(0.0f, vertex.weight(-1));
    QCOMPARE(0.0f, vertex.weight(4));
    delete bone;
}

void TestModel::testMaterialMorphMergeAmbient() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.ambient.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setAmbient(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.6, 0.6, 0.6, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.4, 0.4, 0.4, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.2, 0.2, 0.2, 0.8), material.ambient());
    material.mergeMorph(&morph, 1.0);
    compare(Color(0.0, 0.0, 0.0, 0.8), material.ambient());
    material.resetMorph();
    // add (0.2)
    morph.ambient.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.85, 0.85, 0.85, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.9, 0.9, 0.9, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.ambient());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.0, 1.0, 1.0, 0.8), material.ambient());
    material.resetMorph();
    // add (0.6)
    morph.ambient.setValue(0.6, 0.6, 0.6);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.5);
    compare(Color(1.1, 1.1, 1.1, 0.8), material.ambient());
    material.mergeMorph(&morph, 0.75);
    compare(Color(1.25, 1.25, 1.25, 0.8), material.ambient());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.4, 1.4, 1.4, 0.8), material.ambient());
}

void TestModel::testMaterialMorphMergeDiffuse() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.diffuse.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setDiffuse(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.6, 0.6, 0.6, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.4, 0.4, 0.4, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.2, 0.2, 0.2, 0.8), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    compare(Color(0.0, 0.0, 0.0, 0.8), material.diffuse());
    material.resetMorph();
    // add (0.2)
    morph.diffuse.setValue(0.2, 0.2, 0.2, 1.0);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.85, 0.85, 0.85, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.9, 0.9, 0.9, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.0, 1.0, 1.0, 0.8), material.diffuse());
    material.resetMorph();
    // add (0.6)
    morph.diffuse.setValue(0.6, 0.6, 0.6, 1.0);
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    compare(Color(1.1, 1.1, 1.1, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    compare(Color(1.25, 1.25, 1.25, 0.8), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.4, 1.4, 1.4, 0.8), material.diffuse());
}

void TestModel::testMaterialMorphMergeSpecular() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.specular.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setSpecular(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.specular());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.6, 0.6, 0.6, 0.8), material.specular());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.4, 0.4, 0.4, 0.8), material.specular());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.2, 0.2, 0.2, 0.8), material.specular());
    material.mergeMorph(&morph, 1.0);
    compare(Color(0.0, 0.0, 0.0, 0.8), material.specular());
    material.resetMorph();
    // add (0.2)
    morph.specular.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.specular());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.85, 0.85, 0.85, 0.8), material.specular());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.9, 0.9, 0.9, 0.8), material.specular());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.specular());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.0, 1.0, 1.0, 0.8), material.specular());
    material.resetMorph();
    // add (0.6)
    morph.specular.setValue(0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.specular());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.specular());
    material.mergeMorph(&morph, 0.5);
    compare(Color(1.1, 1.1, 1.1, 0.8), material.specular());
    material.mergeMorph(&morph, 0.75);
    compare(Color(1.25, 1.25, 1.25, 0.8), material.specular());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.4, 1.4, 1.4, 0.8), material.specular());
}

void TestModel::testMaterialMorphMergeShininess() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.shininess = 1.0;
    morph.operation = 0;
    material.setShininess(0.8);
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.shininess());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.6f, material.shininess());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(0.4f, material.shininess());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(0.2f, material.shininess());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(0.0f, material.shininess());
    material.resetMorph();
    // add (0.2)
    morph.shininess = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.shininess());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.85f, material.shininess());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(0.9f, material.shininess());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(0.95f, material.shininess());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(1.0f, material.shininess());
    // add (0.6)
    morph.shininess = 0.6;
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.shininess());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.95f, material.shininess());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(1.1f, material.shininess());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(1.25f, material.shininess());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(1.4f, material.shininess());
}

void TestModel::testMaterialMorphMergeEdgeColor() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.edgeColor.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setEdgeColor(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.6, 0.6, 0.6, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.4, 0.4, 0.4, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.2, 0.2, 0.2, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    compare(Color(0.0, 0.0, 0.0, 0.8), material.edgeColor());
    material.resetMorph();
    // add (0.2)
    morph.edgeColor.setValue(0.2, 0.2, 0.2, 1.0);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.85, 0.85, 0.85, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    compare(Color(0.9, 0.9, 0.9, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.0, 1.0, 1.0, 0.8), material.edgeColor());
    material.resetMorph();
    // add (0.6)
    morph.edgeColor.setValue(0.6, 0.6, 0.6, 1.0);
    material.mergeMorph(&morph, 0.0);
    compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    compare(Color(0.95, 0.95, 0.95, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    compare(Color(1.1, 1.1, 1.1, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    compare(Color(1.25, 1.25, 1.25, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    compare(Color(1.4, 1.4, 1.4, 0.8), material.edgeColor());
}

void TestModel::testMaterialMorphMergeEdgeSize() const
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.edgeSize = 1.0;
    morph.operation = 0;
    material.setEdgeSize(0.8);
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.edgeSize());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.6f, material.edgeSize());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(0.4f, material.edgeSize());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(0.2f, material.edgeSize());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(0.0f, material.edgeSize());
    material.resetMorph();
    // add (0.2)
    morph.edgeSize = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.edgeSize());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.85f, material.edgeSize());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(0.9f, material.edgeSize());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(0.95f, material.edgeSize());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(1.0f, material.edgeSize());
    // add (0.6)
    morph.edgeSize = 0.6;
    material.mergeMorph(&morph, 0.0);
    QCOMPARE(0.8f, material.edgeSize());
    material.mergeMorph(&morph, 0.25);
    QCOMPARE(0.95f, material.edgeSize());
    material.mergeMorph(&morph, 0.5);
    QCOMPARE(1.1f, material.edgeSize());
    material.mergeMorph(&morph, 0.75);
    QCOMPARE(1.25f, material.edgeSize());
    material.mergeMorph(&morph, 1.0);
    QCOMPARE(1.4f, material.edgeSize());
}

void TestModel::testReadWriteBone(size_t indexSize)
{
    Encoding encoding;
    Bone bone, bone2, parent;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    parent.setIndex(0);
    bone.setName(&name);
    bone.setEnglishName(&englishName);
    bone.setDestinationOrigin(Vector3(0.01, 0.02, 0.03));
    bone.setOrigin(Vector3(0.11, 0.12, 0.13));
    bone.setIndex(1);
    bone.setDestinationOrigin(Vector3(0.21, 0.22, 0.23));
    bone.setFixedAxis(Vector3(0.31, 0.32, 0.33));
    bone.setAxisX(Vector3(0.41, 0.42, 0.43));
    bone.setAxisZ(Vector3(0.51, 0.52, 0.53));
    bone.setExternalIndex(3);
    bone.setParentBone(&parent);
    bone.setParentInherenceBone(&parent, 0.61);
    bone.setTargetBone(&parent, 3, 0.71);
    bone.setRotateable(true);
    bone.setMovable(true);
    bone.setVisible(true);
    bone.setOperatable(true);
    bone.setIKEnable(true);
    bone.setPositionInherenceEnable(true);
    bone.setRotationInherenceEnable(true);
    bone.setAxisFixedEnable(true);
    bone.setLocalAxisEnable(true);
    bone.setTransformedAfterPhysicsSimulationEnable(true);
    bone.setTransformedByExternalParentEnable(true);
    size_t size = bone.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    bone.write(data.data(), info);
    bone2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(bone2.name()->equals(bone.name()));
    QVERIFY(bone2.englishName()->equals(bone.englishName()));
    compare(bone2.origin(), bone.origin());
    compare(bone2.destinationOrigin(), bone.destinationOrigin());
    compare(bone2.axis(), bone.axis());
    compare(bone2.axisX(), bone.axisX());
    compare(bone2.axisZ(), bone.axisZ());
    QCOMPARE(bone2.layerIndex(), bone.layerIndex());
    QCOMPARE(bone2.externalIndex(), bone.externalIndex());
    QVERIFY(bone2.isRotateable());
    QVERIFY(bone2.isMovable());
    QVERIFY(bone2.isVisible());
    QVERIFY(bone2.isInteractive());
    QVERIFY(bone2.hasInverseKinematics());
    QVERIFY(bone2.hasPositionInherence());
    QVERIFY(bone2.hasRotationInherence());
    QVERIFY(bone2.hasFixedAxes());
    QVERIFY(bone2.hasLocalAxes());
    QVERIFY(bone2.isTransformedAfterPhysicsSimulation());
    QVERIFY(bone2.isTransformedByExternalParent());
    Array<Bone *> bones, apb, bpb;
    bones.add(&parent);
    bones.add(&bone2);
    Bone::loadBones(bones, bpb, apb);
    QCOMPARE(bone2.parentBone(), &parent);
    QCOMPARE(bone2.parentInherenceBone(), &parent);
    QCOMPARE(bone2.targetBone(), &parent);
}

void TestModel::testReadWriteJoint(size_t indexSize)
{
    Encoding encoding;
    Joint joint, joint2;
    RigidBody body, body2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.rigidBodyIndexSize = indexSize;
    body.setIndex(0);
    body2.setIndex(1);
    joint.setName(&name);
    joint.setEnglishName(&englishName);
    joint.setRigidBody1(&body);
    joint.setRigidBody2(&body2);
    joint.setPosition(Vector3(0.01, 0.02, 0.03));
    joint.setRotation(Vector3(0.11, 0.12, 0.13));
    joint.setPositionLowerLimit(Vector3(0.21, 0.22, 0.23));
    joint.setRotationLowerLimit(Vector3(0.31, 0.32, 0.33));
    joint.setPositionUpperLimit(Vector3(0.41, 0.42, 0.43));
    joint.setRotationUpperLimit(Vector3(0.51, 0.52, 0.53));
    joint.setPositionStiffness(Vector3(0.61, 0.62, 0.63));
    joint.setRotationStiffness(Vector3(0.71, 0.72, 0.73));
    size_t size = joint.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    joint.write(data.data(), info);
    joint2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(joint2.name()->equals(joint.name()));
    QVERIFY(joint2.englishName()->equals(joint.englishName()));
    QCOMPARE(joint2.position(), joint.position());
    QCOMPARE(joint2.rotation(), joint.rotation());
    QCOMPARE(joint2.positionLowerLimit(), joint.positionLowerLimit());
    QCOMPARE(joint2.rotationLowerLimit(), joint.rotationLowerLimit());
    QCOMPARE(joint2.positionUpperLimit(), joint.positionUpperLimit());
    QCOMPARE(joint2.rotationUpperLimit(), joint.rotationUpperLimit());
    QCOMPARE(joint2.positionStiffness(), joint.positionStiffness());
    QCOMPARE(joint2.rotationStiffness(), joint.rotationStiffness());
    QCOMPARE(joint2.rigidBodyIndex1(), body.index());
    QCOMPARE(joint2.rigidBodyIndex2(), body2.index());
}

void TestModel::testReadWriteMaterial(size_t indexSize)
{
    Encoding encoding;
    Material material, material2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.textureIndexSize = indexSize;
    material.setName(&name);
    material.setEnglishName(&englishName);
    material.setSphereTextureRenderMode(Material::kSubTexture);
    material.setAmbient(Color(0.01, 0.02, 0.03, 0.0));
    material.setDiffuse(Color(0.11, 0.12, 0.13, 0.14));
    material.setSpecular(Color(0.21, 0.22, 0.23, 0.0));
    material.setEdgeColor(Color(0.31, 0.32, 0.33, 0.34));
    material.setShininess(0.1);
    material.setEdgeSize(0.2);
    material.setMainTextureIndex(1);
    material.setSphereTextureIndex(2);
    material.setToonTextureIndex(3);
    material.setIndices(4);
    material.setFlags(5);
    size_t size = material.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    material.write(data.data(), info);
    material2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(material2.name()->equals(material.name()));
    QVERIFY(material2.englishName()->equals(material.englishName()));
    compare(material2.ambient(), material.ambient());
    compare(material2.diffuse(), material.diffuse());
    compare(material2.specular(), material.specular());
    compare(material2.edgeColor(), material.edgeColor());
    QCOMPARE(material2.sphereTextureRenderMode(), material.sphereTextureRenderMode());
    QCOMPARE(material2.shininess(), material.shininess());
    QCOMPARE(material2.edgeSize(), material.edgeSize());
    QCOMPARE(material2.textureIndex(), material.textureIndex());
    QCOMPARE(material2.sphereTextureIndex(), material.sphereTextureIndex());
    QCOMPARE(material2.toonTextureIndex(), material.toonTextureIndex());
    QCOMPARE(material2.indices(), material.indices());
}

void TestModel::testReadWriteBoneMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
    Morph::Bone bone1, bone2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.boneIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // bone morph1
    bone1.index = 0;
    bone1.position.setValue(0.11, 0.12, 0.13);
    bone1.rotation.setValue(0.21, 0.22, 0.23, 0.24);
    morph.addBoneMorph(bone1);
    // bone morph2
    bone2.index = 1;
    bone2.position.setValue(0.31, 0.32, 0.33);
    bone2.rotation.setValue(0.41, 0.42, 0.43, 0.44);
    morph.addBoneMorph(bone2);
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kEyeblow);
    morph.setType(pmx::Morph::kBone);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(morph2.name()->equals(morph.name()));
    QVERIFY(morph2.englishName()->equals(morph.englishName()));
    QCOMPARE(morph2.category(), morph.category());
    QCOMPARE(morph2.type(), morph.type());
    const Array<Morph::Bone> &bones = morph2.bones();
    QCOMPARE(2, bones.count());
    compare(bones[0].position, bone1.position);
    compare(bones[0].rotation, bone1.rotation);
    QCOMPARE(bones[0].index, bone1.index);
    compare(bones[1].position, bone2.position);
    compare(bones[1].rotation, bone2.rotation);
    QCOMPARE(bones[1].index, bone2.index);
}

void TestModel::testReadWriteGroupMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
    Morph::Group group1, group2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.morphIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // group morph1
    group1.index = 0;
    group1.weight = 0.1;
    morph.addGroupMorph(group1);
    // group morph2
    group2.index = 1;
    group2.weight = 0.2;
    morph.addGroupMorph(group2);
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kEye);
    morph.setType(pmx::Morph::kGroup);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(morph2.name()->equals(morph.name()));
    QVERIFY(morph2.englishName()->equals(morph.englishName()));
    QCOMPARE(morph2.category(), morph.category());
    QCOMPARE(morph2.type(), morph.type());
    const Array<Morph::Group> &groups = morph2.groups();
    QCOMPARE(2, groups.count());
    QCOMPARE(groups[0].weight, group1.weight);
    QCOMPARE(groups[0].index, group1.index);
    QCOMPARE(groups[1].weight, group2.weight);
    QCOMPARE(groups[1].index, group2.index);
}

void TestModel::testReadWriteMaterialMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
    Morph::Material material1, material2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.materialIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // material morph1
    material1.index = 0;
    material1.ambient.setValue(0.01, 0.02, 0.03);
    material1.diffuse.setValue(0.11, 0.12, 0.13, 0.14);
    material1.specular.setValue(0.21, 0.22, 0.23);
    material1.edgeColor.setValue(0.31, 0.32, 0.33, 0.34);
    material1.sphereTextureWeight.setValue(0.41, 0.42, 0.43, 0.44);
    material1.textureWeight.setValue(0.51, 0.52, 0.53, 0.54);
    material1.toonTextureWeight.setValue(0.61, 0.62, 0.63, 0.64);
    material1.edgeSize = 0.1;
    material1.shininess = 0.2;
    material1.operation = 1;
    // material morph2
    morph.addMaterialMorph(material1);
    material2.index = 1;
    material2.ambient.setValue(0.61, 0.62, 0.63);
    material2.diffuse.setValue(0.51, 0.52, 0.53, 0.54);
    material2.specular.setValue(0.41, 0.42, 0.43);
    material2.edgeColor.setValue(0.31, 0.32, 0.33, 0.34);
    material2.sphereTextureWeight.setValue(0.21, 0.22, 0.23, 0.24);
    material2.textureWeight.setValue(0.11, 0.12, 0.13, 0.14);
    material2.toonTextureWeight.setValue(0.01, 0.02, 0.03, 0.04);
    material2.edgeSize = 0.2;
    material2.shininess = 0.1;
    material2.operation = 2;
    morph.addMaterialMorph(material2);
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kLip);
    morph.setType(pmx::Morph::kMaterial);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(morph2.name()->equals(morph.name()));
    QVERIFY(morph2.englishName()->equals(morph.englishName()));
    QCOMPARE(morph2.category(), morph.category());
    QCOMPARE(morph2.type(), morph.type());
    const Array<Morph::Material> &materials = morph2.materials();
    QCOMPARE(2, materials.count());
    compare(materials[0].ambient, material1.ambient);
    compare(materials[0].diffuse, material1.diffuse);
    compare(materials[0].specular, material1.specular);
    compare(materials[0].edgeColor, material1.edgeColor);
    compare(materials[0].sphereTextureWeight, material1.sphereTextureWeight);
    compare(materials[0].textureWeight, material1.textureWeight);
    compare(materials[0].toonTextureWeight, material1.toonTextureWeight);
    QCOMPARE(materials[0].edgeSize, material1.edgeSize);
    QCOMPARE(materials[0].shininess, material1.shininess);
    QCOMPARE(materials[0].operation, material1.operation);
    QCOMPARE(materials[0].index, material1.index);
    compare(materials[1].ambient, material2.ambient);
    compare(materials[1].diffuse, material2.diffuse);
    compare(materials[1].specular, material2.specular);
    compare(materials[1].edgeColor, material2.edgeColor);
    compare(materials[1].sphereTextureWeight, material2.sphereTextureWeight);
    compare(materials[1].textureWeight, material2.textureWeight);
    compare(materials[1].toonTextureWeight, material2.toonTextureWeight);
    QCOMPARE(materials[1].edgeSize, material2.edgeSize);
    QCOMPARE(materials[1].shininess, material2.shininess);
    QCOMPARE(materials[1].operation, material2.operation);
    QCOMPARE(materials[1].index, material2.index);
}

void TestModel::testReadWriteRigidBody(size_t indexSize)
{
    Encoding encoding;
    RigidBody body, body2;
    Bone bone;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    bone.setIndex(1);
    body.setName(&name);
    body.setEnglishName(&englishName);
    body.setBone(&bone);
    body.setAngularDamping(0.01);
    body.setCollisionGroupID(1);
    body.setCollisionMask(2);
    body.setFriction(0.11);
    body.setLinearDamping(0.21);
    body.setMass(0.31);
    body.setPosition(Vector3(0.41, 0.42, 0.43));
    body.setRestitution(0.51);
    body.setRotation(Vector3(0.61, 0.62, 0.63));
    body.setShapeType(4);
    body.setSize(Vector3(0.71, 0.72, 0.73));
    body.setType(5);
    size_t size = body.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    body.write(data.data(), info);
    body2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(body2.name()->equals(body.name()));
    QVERIFY(body2.englishName()->equals(body.englishName()));
    QCOMPARE(body2.boneIndex(), bone.index());
    QCOMPARE(body2.angularDamping(), body.angularDamping());
    QCOMPARE(body2.collisionGroupID(), body.collisionGroupID());
    QCOMPARE(body2.collisionGroupMask(), body.collisionGroupMask());
    QCOMPARE(body2.friction(), body.friction());
    QCOMPARE(body2.linearDamping(), body.linearDamping());
    QCOMPARE(body2.mass(), body.mass());
    QCOMPARE(body2.position(), body.position());
    QCOMPARE(body2.restitution(), body.restitution());
    QCOMPARE(body2.rotation(), body.rotation());
    QCOMPARE(body2.size(), body.size());
    QCOMPARE(body2.boneIndex(), bone.index());
}

void TestModel::testReadWriteUVMorph(size_t indexSize, pmx::Morph::Type type)
{
    Encoding encoding;
    Morph morph, morph2;
    Morph::UV uv1, uv2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // UV morph1
    uv1.index = 0;
    uv1.position.setValue(0.1, 0.2, 0.3, 0.4);
    morph.addUVMorph(uv1);
    // UV morph2
    uv2.index = 1;
    uv2.position.setValue(0.5, 0.6, 0.7, 0.8);
    morph.addUVMorph(uv2);
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kOther);
    morph.setType(type);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(morph2.name()->equals(morph.name()));
    QVERIFY(morph2.englishName()->equals(morph.englishName()));
    QCOMPARE(morph2.category(), morph.category());
    QCOMPARE(morph2.type(), morph.type());
    const Array<Morph::UV> &uvs = morph2.uvs();
    QCOMPARE(2, uvs.count());
    compare(uvs[0].position, uv1.position);
    QCOMPARE(uvs[0].offset, type - pmx::Morph::kTexCoord);
    QCOMPARE(uvs[0].index, uv1.index);
    compare(uvs[1].position, uv2.position);
    QCOMPARE(uvs[1].offset, type - pmx::Morph::kTexCoord);
    QCOMPARE(uvs[1].index, uv2.index);
}

void TestModel::testReadWriteVertexMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
    Morph::Vertex vertex1, vertex2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // vertex morph1
    vertex1.index = 0;
    vertex1.position.setValue(0.1, 0.2, 0.3);
    morph.addVertexMorph(vertex1);
    // vertex morph2
    vertex2.index = 1;
    vertex2.position.setValue(0.4, 0.5, 0.6);
    morph.addVertexMorph(vertex2);
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kOther);
    morph.setType(pmx::Morph::kVertex);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    QCOMPARE(read, size);
    QVERIFY(morph2.name()->equals(morph.name()));
    QVERIFY(morph2.englishName()->equals(morph.englishName()));
    QCOMPARE(morph2.category(), morph.category());
    QCOMPARE(morph2.type(), morph.type());
    const Array<Morph::Vertex> &vertices = morph2.vertices();
    QCOMPARE(2, vertices.count());
    compare(vertices[0].position, vertex1.position);
    QCOMPARE(vertices[0].index, vertex1.index);
    compare(vertices[1].position, vertex2.position);
    QCOMPARE(vertices[1].index, vertex2.index);
}

void TestModel::testReadWriteVertexBdef1(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1;
    Model::DataInfo info;
    bones.add(&bone1);
    setVertex(vertex, Vertex::kBdef1, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    QCOMPARE(read, size);
    compareVertex(vertex, vertex2, bones);
}

void TestModel::testReadWriteVertexBdef2(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bones.add(&bone1);
    bones.add(&bone2);
    setVertex(vertex, Vertex::kBdef2, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    QCOMPARE(read, size);
    compareVertex(vertex, vertex2, bones);
}

void TestModel::testReadWriteVertexBdef4(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2, bone3, bone4;
    Model::DataInfo info;
    bones.add(&bone1);
    bones.add(&bone2);
    bones.add(&bone3);
    bones.add(&bone4);
    setVertex(vertex, Vertex::kBdef4, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, read);
    QCOMPARE(read, size);
    compareVertex(vertex, vertex2, bones);
    delete[] data;
}

void TestModel::testReadWriteVertexSdef(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bones.add(&bone1);
    bones.add(&bone2);
    setVertex(vertex, Vertex::kSdef, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    QCOMPARE(read, size);
    compareVertex(vertex, vertex2, bones);
}

QTEST_APPLESS_MAIN(TestModel)

#include "TestModel.moc"

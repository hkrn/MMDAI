#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <btBulletDynamicsCommon.h>

#include <vpvl/vpvl.h>
#include <vpvl/internal/util.h>

class TestPMDModel : public QObject
{
    Q_OBJECT

public:
    static const char *kTestString;

    TestPMDModel();

private Q_SLOTS:
    void parseEmpty();
    void parseFile();
    void parseBone();
    void parseConstraint();
    void parseFace();
    void parseIK();
    void parseMaterial();
    void parseRigidBody();
    void parseVertex();
};

namespace
{

static vpvl::Bone *CreateBone(int id)
{
    QByteArray bytes;
    bytes.resize(vpvl::Bone::stride());
    bytes.fill(0);
    vpvl::Bone *bone = new vpvl::Bone();
    bone->read(reinterpret_cast<const uint8_t *>(bytes.constData()), id);
    return bone;
}

static vpvl::RigidBody *CreateRigidBody(QByteArray &bytes, vpvl::BoneList *bones)
{
    bytes.clear();
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(TestPMDModel::kTestString, vpvl::RigidBody::kNameSize);
    stream << quint16(0xffff)         // bone
           << quint8(3)               // collision group ID
           << quint16(2)              // collision mask
           << quint8(0)               // shape type
           << 4.0f                    // width
           << 5.0f                    // height
           << 6.0f                    // depth
           << 7.0f << 8.0f << 9.0f    // position
           << 10.0f << 11.0f << 12.0f // rotation
           << 0.1f                    // mass
           << 0.2f                    // linearDamping
           << 0.3f                    // angularDamping
           << 0.4f                    // restitution
           << 0.5f                    // friction
           << quint8(1)               // type
              ;
    vpvl::RigidBody *body = new vpvl::RigidBody();
    body->read(reinterpret_cast<const uint8_t *>(bytes.constData()), bones);
    return body;
}

static void TestBone(const vpvl::Bone &bone)
{
    QCOMPARE(QString(reinterpret_cast<const char *>(bone.name())), QString(TestPMDModel::kTestString));
    QCOMPARE(bone.id(), qint16(7));
    QCOMPARE(bone.type(), vpvl::Bone::kTwist);
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(bone.originPosition() == btVector3(4, 5, -6));
#else
    QVERIFY(bone.originPosition() == btVector3(4, 5, 6));
#endif
}

static void TestFace(vpvl::Face &face)
{
    QCOMPARE(QString(reinterpret_cast<const char *>(face.name())), QString(TestPMDModel::kTestString));
    QCOMPARE(face.type(), vpvl::Face::kBase);
    vpvl::VertexList vertices;
    vpvl::Vertex *vertex = new vpvl::Vertex();
    vertices.add(vertex);
    face.setVertices(vertices);
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(vertex->position() == btVector3(1.0f, 2.0f, -3.0f));
#else
    QVERIFY(vertex->position() == btVector3(1.0f, 2.0f, 3.0f));
#endif
    vertices.releaseAll();
}

static void TestMaterial(const vpvl::Material &material, const QString &path)
{
    QCOMPARE(QString(reinterpret_cast<const char *>(material.rawName())), QString(path));
    QCOMPARE(QString(reinterpret_cast<const char *>(material.mainTextureName())), QString("main.sph"));
    QCOMPARE(QString(reinterpret_cast<const char *>(material.subTextureName())), QString("sub.spa"));
    QVERIFY(material.ambient() == btVector4(0.8f, 0.9f, 1.0f, 1.0f));
    QVERIFY(material.averageColor() == btVector4(0.4f, 0.5f, 0.6f, 1.0f));
    QVERIFY(material.diffuse() == btVector4(0.0f, 0.1f, 0.2f, 1.0f));
    QVERIFY(material.specular() == btVector4(0.5f, 0.6f, 0.7f, 1.0f));
    QCOMPARE(material.opacity(), 0.3f);
    QCOMPARE(material.shiness(), 0.4f);
    QCOMPARE(material.countIndices(), 2);
    QCOMPARE(material.toonID(), quint8(0));
    QVERIFY(material.isEdgeEnabled());
    QVERIFY(material.isMainSphereModulate());
    QVERIFY(!material.isMainSphereAdd());
    QVERIFY(!material.isSubSphereModulate());
    QVERIFY(material.isSubSphereAdd());
}

static void TestVertex(const vpvl::Vertex &vertex)
{
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(vertex.position() == btVector3(0.0f, 1.0f, -2.0f));
    QVERIFY(vertex.normal() == btVector3(3.0f, 4.0f, -5.0f));
#else
    QVERIFY(vertex.position() == btVector3(0.0f, 1.0f, 2.0f));
    QVERIFY(vertex.normal() == btVector3(3.0f, 4.0f, 5.0f));
#endif
    QCOMPARE(vertex.u(), 6.0f);
    QCOMPARE(vertex.v(), 7.0f);
    QCOMPARE(vertex.bone1(), qint16(8));
    QCOMPARE(vertex.bone2(), qint16(9));
    QCOMPARE(vertex.weight(), 1.0f);
    QVERIFY(!vertex.isEdgeEnabled());
}

}

const char *TestPMDModel::kTestString = "0123456789012345678";

TestPMDModel::TestPMDModel()
{
}

void TestPMDModel::parseEmpty()
{
    vpvl::PMDModel model;
    vpvl::PMDModel::DataInfo info;
    QVERIFY(!model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(model.error(), vpvl::PMDModel::kInvalidHeaderError);
}

void TestPMDModel::parseFile()
{
    QFile file("../../render/res/miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::PMDModel model, model2;
        vpvl::PMDModel::DataInfo result, result2;
        QVERIFY(model.preparse(data, size, result));
        QVERIFY(model.load(data, size));
        QCOMPARE(result.verticesCount, size_t(model.vertices().count()));
        QCOMPARE(result.materialsCount, size_t(model.materials().count()));
        QCOMPARE(result.bonesCount, size_t(model.bones().count()));
        QCOMPARE(result.IKsCount, size_t(model.IKs().count()));
        QCOMPARE(result.facesCount, size_t(model.faces().count()));
        QCOMPARE(result.rigidBodiesCount, size_t(model.rigidBodies().count()));
        QCOMPARE(result.constranitsCount, size_t(model.constraints().count()));
        QCOMPARE(reinterpret_cast<const char *>(model.englishName()), "Miku Hatsune");
        QCOMPARE(model.error(), vpvl::PMDModel::kNoError);
        size_t estimated = model.estimateSize();
        QCOMPARE(estimated, size);
        QByteArray bytes2;
        bytes2.resize(estimated);
        bytes2.fill(0);
        uint8_t *toBeWritten = reinterpret_cast<uint8_t *>(bytes2.data());
        model.save(toBeWritten);
        QVERIFY(model2.preparse(toBeWritten, size, result2));
        QVERIFY(model2.load(toBeWritten, size));
        QCOMPARE(result2.verticesCount, size_t(model.vertices().count()));
        QCOMPARE(result2.materialsCount, size_t(model.materials().count()));
        QCOMPARE(result2.bonesCount, size_t(model.bones().count()));
        QCOMPARE(result2.IKsCount, size_t(model.IKs().count()));
        QCOMPARE(result2.facesCount, size_t(model.faces().count()));
        QCOMPARE(result2.rigidBodiesCount, size_t(model.rigidBodies().count()));
        QCOMPARE(result2.constranitsCount, size_t(model.constraints().count()));
        QCOMPARE(reinterpret_cast<const char *>(model2.englishName()), "Miku Hatsune");
        QCOMPARE(model2.error(), vpvl::PMDModel::kNoError);
#if 1
        QFile file("/Users/hkrn/test.pmd");
        file.open(QFile::WriteOnly);
        file.write(bytes2);
#endif
    }
    else {
        QSKIP("Require a model to test this", SkipSingle);
    }
}

void TestPMDModel::parseBone()
{
    QByteArray bytes, bytes2;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::Bone::kNameSize);
    stream << qint16(1)                 // parent
           << qint16(2)                 // child
           << qint8(vpvl::Bone::kTwist) // type
           << qint16(3)                 // target
           << 4.0f << 5.0f << 6.0f      // position
              ;
    size_t stride = vpvl::Bone::stride();
    QCOMPARE(size_t(bytes.size()), stride);
    vpvl::Bone bone, bone2;
    bone.read(reinterpret_cast<const uint8_t *>(bytes.constData()), 7);
    TestBone(bone);
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(vpvl::Bone::stride());
    bytes.fill(0);
    bone.write(reinterpret_cast<uint8_t *>(bytes.data()));
    bone2.read(reinterpret_cast<const uint8_t *>(bytes.data()), 7);
    QCOMPARE(bytes2, bytes);
    TestBone(bone2);
}

void TestPMDModel::parseConstraint()
{
    QByteArray bytes, bytes2, bytes3;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::Constraint::kNameSize);
    stream << qint32(0) << qint32(1)  // bodyID
           << 4.0f << 5.0 << 6.0f     // position
           << 7.0f << 8.0f << 9.0f    // rotation
           << 10.0f << 11.0f << 12.0f // limitPositionFrom
           << 13.0f << 14.0f << 15.0f // limitPositionTo
           << 16.0f << 17.0f << 18.0f // limitRotationFrom
           << 19.0f << 20.0f << 21.0f // limitRotationTo
           << 22.0f << 23.0f << 24.0f // stiffness
           << 25.0f << 26.0f << 27.0f
              ;
    QCOMPARE(size_t(bytes.size()), vpvl::Constraint::stride());
    vpvl::Constraint constaint;
    vpvl::RigidBodyList bodies;
    vpvl::BoneList bones;
    bones.add(CreateBone(1));
    bodies.add(CreateRigidBody(bytes3, &bones));
    bodies.add(CreateRigidBody(bytes3, &bones));
    btVector3 offset(1.0f, 2.0f, 3.0f);
    constaint.read(reinterpret_cast<const uint8_t *>(bytes.constData()), bodies, offset);
    QCOMPARE(QString(reinterpret_cast<const char *>(constaint.name())), QString(kTestString));
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(vpvl::Constraint::stride());
    bytes.fill(0);
    constaint.write(reinterpret_cast<uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    bodies.releaseAll();
}

void TestPMDModel::parseFace()
{
    QByteArray bytes, bytes2;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::Face::kNameSize);
    stream << qint32(1)                // size
           << qint8(vpvl::Face::kBase) // type
           << qint32(0)                // vertex id
           << 1.0f << 2.0f << 3.0f     // position
              ;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(bytes.constData());
    QCOMPARE(size_t(bytes.size()), vpvl::Face::stride(ptr));
    bool ok = false;
    size_t size = vpvl::Face::totalSize(ptr, bytes.size(), 1, ok);
    QVERIFY(ok);
    QCOMPARE(size, size_t(bytes.size()));
    size = vpvl::Face::totalSize(ptr, bytes.size(), 2, ok);
    QVERIFY(!ok);
    QCOMPARE(size, size_t(0));
    vpvl::Face face, face2;
    face.read(ptr);
    TestFace(face);
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(face.estimateSize());
    bytes.fill(0);
    face.write(reinterpret_cast<uint8_t *>(bytes.data()));
    face2.read(reinterpret_cast<const uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    TestFace(face2);
}

void TestPMDModel::parseIK()
{
    QByteArray bytes, bytes2;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << qint16(0)                 // dest
           << qint16(1)                 // target
           << qint8(1)                  // nlinks
           << qint16(2)                 // niterations
           << 4.0f                      // constaint
           << uint16_t(2)
              ;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(bytes.constData());
    QCOMPARE(size_t(bytes.size()), vpvl::IK::stride(ptr));
    bool ok = false;
    size_t size = vpvl::IK::totalSize(ptr, bytes.size(), 1, ok);
    QVERIFY(ok);
    QCOMPARE(size, size_t(bytes.size()));
    size = vpvl::IK::totalSize(ptr, bytes.size(), 2, ok);
    QVERIFY(!ok);
    QCOMPARE(size, size_t(0));
    vpvl::BoneList bones;
    bones.add(CreateBone(0));
    bones.add(CreateBone(1));
    bones.add(CreateBone(2));
    vpvl::IK ik;
    ik.read(ptr, &bones);
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(ik.estimateSize());
    bytes.fill(0);
    ik.write(reinterpret_cast<uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    bones.releaseAll();
}

void TestPMDModel::parseMaterial()
{
    // TODO: should rename primary to main, second to sub
    QByteArray bytes, bytes2;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << 0.0f << 0.1f << 0.2f // diffuse
           << 0.3f                 // alpha
           << 0.4f                 // shiness
           << 0.5f << 0.6f << 0.7f // specular
           << 0.8f << 0.9f << 1.0f // ambient
           << quint8(255)          // toonID
           << quint8(1)            // edge
           << quint32(2)           // nindices
              ;
    const char path[] = "main.sph*sub.spa";
    stream.writeRawData(path, vpvl::Material::kNameSize);
    QCOMPARE(size_t(bytes.size()), vpvl::Material::stride());
    vpvl::Material material, material2;
    material.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    TestMaterial(material, path);
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(vpvl::Material::stride());
    bytes.fill(0);
    material.write(reinterpret_cast<uint8_t *>(bytes.data()));
    material2.read(reinterpret_cast<const uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    TestMaterial(material2, path);
}

void TestPMDModel::parseRigidBody()
{
    QByteArray bytes, bytes2;
    vpvl::BoneList bones;
    bones.add(CreateBone(1));
    vpvl::RigidBody *body = CreateRigidBody(bytes, &bones);
    QCOMPARE(size_t(bytes.size()), vpvl::RigidBody::stride());
    body->read(reinterpret_cast<const uint8_t *>(bytes.constData()), &bones);
    QCOMPARE(QString(reinterpret_cast<const char *>(body->name())), QString(kTestString));
    btRigidBody *b = body->body();
    QVERIFY(b != 0);
    QCOMPARE(b->getLinearDamping(), 0.2f);
    QCOMPARE(b->getAngularDamping(), 0.3f);
    QCOMPARE(b->getRestitution(), 0.4f);
    QCOMPARE(b->getFriction(), 0.5f);
    QCOMPARE(body->groupID(), quint16(8));
    QCOMPARE(body->groupMask(), quint16(2));
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(vpvl::RigidBody::stride());
    bytes.fill(0);
    body->write(reinterpret_cast<uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    bones.releaseAll();
}

void TestPMDModel::parseVertex()
{
    QByteArray bytes, bytes2;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << 0.0f << 1.0f << 2.0f // position
           << 3.0f << 4.0f << 5.0f // normal
           << 6.0f << 7.0f         // uv
           << qint16(8)            // parent
           << qint16(9)            // child
           << quint8(100)          // weight
           << quint8(1)            // no edge
              ;
    QCOMPARE(size_t(bytes.size()), vpvl::Vertex::stride());
    vpvl::Vertex vertex, vertex2;
    vertex.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    TestVertex(vertex);
    bytes2 = bytes;
    bytes.clear();
    bytes.resize(vpvl::Vertex::stride());
    bytes.fill(0);
    vertex.write(reinterpret_cast<uint8_t *>(bytes.data()));
    vertex2.read(reinterpret_cast<const uint8_t *>(bytes.data()));
    QCOMPARE(bytes2, bytes);
    TestVertex(vertex2);
}

QTEST_APPLESS_MAIN(TestPMDModel)

#include "TestPMDModel.moc"

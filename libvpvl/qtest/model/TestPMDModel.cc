#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>

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
    QFile file("../../../gtest/res/miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::PMDModel model;
        vpvl::PMDModel::DataInfo result;
        QVERIFY(model.preparse(data, size, result));
        QVERIFY(model.load(data, size));
        QCOMPARE(result.verticesCount, size_t(model.vertices().size()));
        QCOMPARE(result.materialsCount, size_t(model.materials().size()));
        QCOMPARE(result.bonesCount, size_t(model.bones().size()));
        QCOMPARE(result.IKsCount, size_t(model.IKs().size()));
        QCOMPARE(result.facesCount, size_t(model.faces().size()));
        QCOMPARE(result.rigidBodiesCount, size_t(model.rigidBodies().size()));
        QCOMPARE(result.constranitsCount, size_t(model.constraints().size()));
        QCOMPARE(reinterpret_cast<const char *>(model.englishName()), "Miku Hatsune");
        QCOMPARE(model.error(), vpvl::PMDModel::kNoError);
    }
    else {
        QSKIP("Require a model to test this", SkipSingle);
    }
}

void TestPMDModel::parseBone()
{
    QByteArray bytes;
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
    QCOMPARE(size_t(bytes.size()), vpvl::Bone::stride());
    vpvl::Bone bone;
    bone.read(reinterpret_cast<const uint8_t *>(bytes.constData()), 7);
    QCOMPARE(QString(reinterpret_cast<const char *>(bone.name())), QString(kTestString));
    QCOMPARE(bone.id(), qint16(7));
    QCOMPARE(bone.type(), vpvl::Bone::kTwist);
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(bone.originPosition() == btVector3(4, 5, -6));
#else
    QVERIFY(bone.originPosition() == btVector3(4, 5, 6));
#endif
}

void TestPMDModel::parseConstraint()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::Constraint::kNameSize);
    stream << qint32(1) << qint32(2)  // bodyID
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
    bodies.push_back(new vpvl::RigidBody());
    bodies.push_back(new vpvl::RigidBody());
    btVector3 offset(1.0f, 2.0f, 3.0f);
    constaint.read(reinterpret_cast<const uint8_t *>(bytes.constData()), bodies, offset);
    QCOMPARE(QString(reinterpret_cast<const char *>(constaint.name())), QString(kTestString));
    vpvl::internal::clearArray(bodies);
}

void TestPMDModel::parseFace()
{
    QByteArray bytes;
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
    vpvl::Face face;
    bool ok = false;
    size_t size = vpvl::Face::totalSize(ptr, bytes.size(), 1, ok);
    QVERIFY(ok);
    QCOMPARE(size, size_t(bytes.size()));
    size = vpvl::Face::totalSize(ptr, bytes.size(), 2, ok);
    QVERIFY(!ok);
    QCOMPARE(size, size_t(0));
    face.read(ptr);
    QCOMPARE(QString(reinterpret_cast<const char *>(face.name())), QString(kTestString));
    QCOMPARE(face.type(), vpvl::Face::kBase);
    vpvl::VertexList vertices;
    vpvl::Vertex *vertex = new vpvl::Vertex();
    vertices.push_back(vertex);
    face.setVertices(vertices);
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(vertex->position() == btVector3(1.0f, 2.0f, -3.0f));
#else
    QVERIFY(vertex->position() == btVector3(1.0f, 2.0f, 3.0f));
#endif
    vpvl::internal::clearArray(vertices);
}

void TestPMDModel::parseIK()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << qint16(0)                 // dest
           << qint16(1)                 // target
           << qint8(1)                  // nlinks
           << qint16(2)                 // niterations
           << 4.0f                      // constaint
           << uint16_t(3)
              ;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(bytes.constData());
    QCOMPARE(size_t(bytes.size()), vpvl::IK::stride(ptr));
    vpvl::IK ik;
    bool ok = false;
    size_t size = vpvl::IK::totalSize(ptr, bytes.size(), 1, ok);
    QVERIFY(ok);
    QCOMPARE(size, size_t(bytes.size()));
    size = vpvl::IK::totalSize(ptr, bytes.size(), 2, ok);
    QVERIFY(!ok);
    QCOMPARE(size, size_t(0));
    vpvl::BoneList bones;
    bones.push_back(new vpvl::Bone());
    bones.push_back(new vpvl::Bone());
    ik.read(ptr, &bones);
    vpvl::internal::clearArray(bones);
}

void TestPMDModel::parseMaterial()
{
    // TODO: should rename primary to main, second to sub
}

void TestPMDModel::parseRigidBody()
{
}

void TestPMDModel::parseVertex()
{
    QByteArray bytes;
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
    vpvl::Vertex vertex;
    vertex.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
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
    QCOMPARE(vertex.isEdgeEnabled(), false);
}

QTEST_APPLESS_MAIN(TestPMDModel)

#include "TestPMDModel.moc"

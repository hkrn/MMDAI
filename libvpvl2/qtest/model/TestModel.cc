#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <btBulletDynamicsCommon.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>

using namespace vpvl2;
using namespace vpvl2::pmx;

namespace {

class String : public IString {
public:
    String(const QString &s) : m_bytes(s.toUtf8()), m_value(s) {
    }
    ~String() {
    }

    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(m_bytes.constData());
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const QString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_bytes.constData());
    }
    size_t length() const {
        return m_value.length();
    }

private:
    QByteArray m_bytes;
    QString m_value;
};

class Encoding : public IEncoding {
public:
    Encoding()
        : m_sjis(QTextCodec::codecForName("Shift-JIS")),
          m_utf8(QTextCodec::codecForName("UTF-8")),
          m_utf16(QTextCodec::codecForName("UTF-16"))
    {
    }
    ~Encoding() {
    }

    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(m_sjis->toUnicode(str, size));
            break;
        case IString::kUTF8:
            s = new String(m_utf8->toUnicode(str, size));
            break;
        case IString::kUTF16:
            s = new String(m_utf16->toUnicode(str, size));
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = qstrnlen(reinterpret_cast<const char *>(value), maxlen);
        return toString(value, size, codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        const String *s = static_cast<const String *>(value);
        QByteArray bytes;
        switch (codec) {
        case IString::kShiftJIS:
            bytes = m_sjis->fromUnicode(s->value());
            break;
        case IString::kUTF8:
            bytes = m_utf8->fromUnicode(s->value());
            break;
        case IString::kUTF16:
            bytes = m_utf16->fromUnicode(s->value());
            break;
        }
        size_t size = bytes.length();
        uint8_t *data = new uint8_t[size + 1];
        memcpy(data, bytes.constData(), size);
        data[size] = 0;
        return data;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }
    pmx::Model *m_model;

private:
    QTextCodec *m_sjis;
    QTextCodec *m_utf8;
    QTextCodec *m_utf16;
};

}

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
    static void setVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones);
    static void compareVertex(const Vertex &vertex, const Vertex &vertex2, const Array<Bone *> &bones);

private Q_SLOTS:
    void parseEmpty();
    void parseFile();
    void parseMaterial();
    void parseMorph();
    void parseVertexBdef1();
    void parseVertexBdef2();
    void parseVertexBdef4();
    void parseVertexSdef();
    void checkBoundVertex();
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

void TestModel::parseEmpty()
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    QVERIFY(!model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(model.error(), Model::kInvalidHeaderError);
}

void TestModel::parseFile()
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    QFile file("miku.pmx");
    file.open(QFile::ReadOnly);
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

void TestModel::parseMaterial()
{
    Encoding encoding;
    Material material, material2;
    Model::DataInfo info;
    info.encoding = &encoding;
    info.textureIndexSize = 4;
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
    size_t size = material.estimateSize(info);
    uint8_t *data = new uint8_t[size];
    material.write(data, info);
    material2.read(data, info, size);
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
    delete[] data;
}

void TestModel::parseMorph()
{
}

void TestModel::parseVertexBdef1()
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1;
    Model::DataInfo info;
    bones.add(&bone1);
    setVertex(vertex, Vertex::kBdef1, bones);
    info.additionalUVSize = 1;
    info.boneIndexSize = 1;
    size_t size = vertex.estimateSize(info);
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, size);
    compareVertex(vertex, vertex2, bones);
    delete[] data;
}

void TestModel::parseVertexBdef2()
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bones.add(&bone1);
    bones.add(&bone2);
    setVertex(vertex, Vertex::kBdef2, bones);
    info.additionalUVSize = 2;
    info.boneIndexSize = 2;
    size_t size = vertex.estimateSize(info);
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, size);
    compareVertex(vertex, vertex2, bones);
    delete[] data;
}

void TestModel::parseVertexBdef4()
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
    info.additionalUVSize = 4;
    info.boneIndexSize = 4;
    size_t size = vertex.estimateSize(info);
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, size);
    compareVertex(vertex, vertex2, bones);
    delete[] data;
}

void TestModel::parseVertexSdef()
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bones.add(&bone1);
    bones.add(&bone2);
    setVertex(vertex, Vertex::kSdef, bones);
    info.additionalUVSize = 4;
    info.boneIndexSize = 4;
    size_t size = vertex.estimateSize(info);
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, size);
    compareVertex(vertex, vertex2, bones);
    delete[] data;
}

void TestModel::checkBoundVertex()
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
}

QTEST_APPLESS_MAIN(TestModel)

#include "TestModel.moc"

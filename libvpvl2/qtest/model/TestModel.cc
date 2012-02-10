#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <btBulletDynamicsCommon.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>

using namespace vpvl2;
using namespace vpvl2::pmx;

class TestModel : public QObject
{
    Q_OBJECT

public:
    static const char *kTestString;
    static const uint8_t *kName;
    static const uint8_t *kEnglishName;

    TestModel();

private Q_SLOTS:
    void parseEmpty();
    void parseFile();
};

TestModel::TestModel()
{
}

void TestModel::parseEmpty()
{
    Model model;
    Model::DataInfo info;
    QVERIFY(!model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(model.error(), Model::kInvalidHeaderError);
}

void TestModel::parseFile()
{
    Model model;
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

QTEST_APPLESS_MAIN(TestModel)

#include "TestModel.moc"

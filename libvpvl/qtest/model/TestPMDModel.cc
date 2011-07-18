#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>

class TestPMDModel : public QObject
{
    Q_OBJECT

public:
    TestPMDModel();

private Q_SLOTS:
    void parseEmpty();
    void parseFile();
};

TestPMDModel::TestPMDModel()
{
}

void TestPMDModel::parseEmpty()
{
    vpvl::PMDModel model;
    vpvl::PMDModelDataInfo info;
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
        vpvl::PMDModelDataInfo result;
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

QTEST_APPLESS_MAIN(TestPMDModel)

#include "TestPMDModel.moc"

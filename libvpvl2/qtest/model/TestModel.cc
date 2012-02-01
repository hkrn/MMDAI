#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <btBulletDynamicsCommon.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>

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
}

QTEST_APPLESS_MAIN(TestModel)

#include "TestModel.moc"

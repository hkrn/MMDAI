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

QTEST_APPLESS_MAIN(TestPMDModel)

#include "TestPMDModel.moc"

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>
#include <vpvl/Project.h>

using namespace vpvl;

class TestProject : public QObject
{
    Q_OBJECT

public:
    TestProject();

private Q_SLOTS:
    void load();
};

TestProject::TestProject()
{
}

void TestProject::load()
{
    Project project;
    QVERIFY(project.load("project.xml"));
    const Array<Asset *> &assets = project.assets();
    QCOMPARE(assets.count(), 1);
    const Array<PMDModel *> &models = project.models();
    QCOMPARE(models.count(), 1);
    const Array<VMDMotion *> &motions = project.motions();
    QCOMPARE(motions.count(), 1);
    // settings
    QCOMPARE(project.globalSetting("no_such_key").c_str(), "");
    QCOMPARE(project.globalSetting("width").c_str(), "640");
}

QTEST_APPLESS_MAIN(TestProject)

#include "TestProject.moc"

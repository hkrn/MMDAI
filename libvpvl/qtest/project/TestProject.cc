#include <QtCore/QString>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>
#include <vpvl/Project.h>

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
    vpvl::Project project;
    QVERIFY(project.load("project.xml"));
}

QTEST_APPLESS_MAIN(TestProject)

#include "TestProject.moc"

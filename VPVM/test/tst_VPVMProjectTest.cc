#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "CameraRefObject.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MotionProxy.h"
#include "ProjectProxy.h"

#include <vpvl2/vpvl2.h>

using namespace vpvl2;

class VPVMProjectTest : public QObject
{
    Q_OBJECT

public:
    VPVMProjectTest();
    ~VPVMProjectTest();

private Q_SLOTS:
    void createEmptyProject();
    void createModelAndDelete();
    void createMotionAndDelete();
    void seekCurrentTimeIndex();
    void globalProjectSettings();
    void modelProjectSettings();

private:
    ModelProxy *createModelProxy(ProjectProxy *projectProxy);
    MotionProxy *createMotionProxy(ProjectProxy *projectProxy);
};

VPVMProjectTest::VPVMProjectTest()
{
}

VPVMProjectTest::~VPVMProjectTest()
{
}

void VPVMProjectTest::createEmptyProject()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    projectProxy->projectInstanceRef()->setDirty(true);
    QSignalSpy projectWillCreate(projectProxy, SIGNAL(projectWillCreate()));
    QSignalSpy projectDidCreate(projectProxy, SIGNAL(projectDidCreate()));
    QSignalSpy dirtyChanged(projectProxy, SIGNAL(dirtyChanged()));
    projectProxy->createAsync();
    projectProxy->internalCreateAsync();
    QVERIFY(!projectProxy->isDirty());
    QCOMPARE(projectWillCreate.size(), 1);
    QCOMPARE(projectDidCreate.size(), 1);
    QCOMPARE(dirtyChanged.size(), 1);
}

void VPVMProjectTest::createModelAndDelete()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    QSignalSpy modelWillLoad(projectProxy, SIGNAL(modelWillLoad(ModelProxy*)));
    QSignalSpy modelDidLoad(projectProxy, SIGNAL(modelDidLoad(ModelProxy*,bool)));
    QSignalSpy modelDidAdd(projectProxy, SIGNAL(modelDidAdd(ModelProxy*,bool)));
    IModel *model = projectProxy->factoryInstanceRef()->newModel(IModel::kPMDModel);
    const QUuid &uuid = QUuid::createUuid();
    ModelProxy *modelProxy = projectProxy->createModelProxy(model, uuid, QUrl(), false);
    projectProxy->internalAddModel(modelProxy, true, true);
    QCOMPARE(projectProxy->currentModel(), modelProxy);
    QCOMPARE(projectProxy->modelProxies().size(), 1);
    QCOMPARE(projectProxy->modelProxies().first(), modelProxy);
    QCOMPARE(modelProxy->data(), model);
    QCOMPARE(modelProxy->uuid(), uuid);
    QCOMPARE(modelWillLoad.size(), 1);
    QCOMPARE(modelDidLoad.size(), 1);
    QCOMPARE(modelDidAdd.size(), 1);
    QVERIFY(projectProxy->isDirty());
    QVERIFY(!projectProxy->findModel(QUuid::createUuid()));
    QVERIFY(!projectProxy->resolveModelProxy(0));
    QCOMPARE(projectProxy->findModel(uuid), modelProxy);
    QCOMPARE(projectProxy->resolveModelProxy(model), modelProxy);
    QSignalSpy modelWillRemove(projectProxy, SIGNAL(modelWillRemove(ModelProxy*)));
    QSignalSpy modelDidRemove(projectProxy, SIGNAL(modelDidRemove(ModelProxy*)));
    projectProxy->deleteModel(modelProxy);
    QCOMPARE(projectProxy->modelProxies().size(), 0);
    QCOMPARE(modelWillRemove.size(), 1);
    QCOMPARE(modelDidRemove.size(), 1);
    QVERIFY(!projectProxy->findModel(uuid));
    QVERIFY(!projectProxy->resolveModelProxy(model));
}

void VPVMProjectTest::createMotionAndDelete()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    QSignalSpy motionWillLoad(projectProxy, SIGNAL(motionWillLoad(MotionProxy*)));
    IMotion *motion = projectProxy->factoryInstanceRef()->newMotion(IMotion::kVMDMotion, 0);
    const QUuid &uuid = QUuid::createUuid();
    MotionProxy *motionProxy = projectProxy->createMotionProxy(motion, uuid, QUrl(), true);
    QCOMPARE(projectProxy->motionProxies().size(), 1);
    QCOMPARE(projectProxy->motionProxies().first(), motionProxy);
    QCOMPARE(motionProxy->data(), motion);
    QCOMPARE(motionProxy->uuid(), uuid);
    QCOMPARE(motionWillLoad.size(), 1);
    QVERIFY(!projectProxy->findMotion(QUuid::createUuid()));
    QVERIFY(!projectProxy->resolveMotionProxy(0));
    QCOMPARE(projectProxy->findMotion(uuid), motionProxy);
    QCOMPARE(projectProxy->resolveMotionProxy(motion), motionProxy);
    QSignalSpy motionWillDelete(projectProxy, SIGNAL(motionWillDelete(MotionProxy*)));
    projectProxy->deleteMotion(motionProxy, false);
    QCOMPARE(projectProxy->motionProxies().size(), 0);
    QCOMPARE(motionWillDelete.size(), 1);
    QVERIFY(!projectProxy->findMotion(uuid));
    QVERIFY(!projectProxy->resolveMotionProxy(motion));
}

void VPVMProjectTest::seekCurrentTimeIndex()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    projectProxy->createAsync();
    projectProxy->internalCreateAsync();
    QCOMPARE(projectProxy->currentTimeIndex(), qreal(0));
    QSignalSpy currentTimeIndexChanged(projectProxy, SIGNAL(currentTimeIndexChanged()));
    QSignalSpy cameraDidRefresh(projectProxy->camera(), SIGNAL(cameraDidReset()));
    QSignalSpy lightDidRefresh(projectProxy->light(), SIGNAL(lightDidReset()));
    /* test seek (signals will be emitted once) */
    projectProxy->seek(42);
    projectProxy->seek(42);
    QCOMPARE(projectProxy->currentTimeIndex(), qreal(42));
    QCOMPARE(projectProxy->projectInstanceRef()->currentTimeIndex(), IKeyframe::TimeIndex(42));
    QCOMPARE(currentTimeIndexChanged.size(), 1);
    QCOMPARE(cameraDidRefresh.size(), 1);
    QCOMPARE(lightDidRefresh.size(), 1);
    currentTimeIndexChanged.clear();
    cameraDidRefresh.clear();
    lightDidRefresh.clear();
    /* test rewind (signals will be emitted twice) */
    projectProxy->rewind();
    projectProxy->rewind();
    QCOMPARE(projectProxy->currentTimeIndex(), qreal(0));
    QCOMPARE(projectProxy->projectInstanceRef()->currentTimeIndex(), IKeyframe::TimeIndex(0));
    QCOMPARE(currentTimeIndexChanged.size(), 2);
    QCOMPARE(cameraDidRefresh.size(), 2);
    QCOMPARE(lightDidRefresh.size(), 2);
}

void VPVMProjectTest::globalProjectSettings()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    projectProxy->createAsync();
    QCOMPARE(projectProxy->globalSetting("test0"), QVariant());
    QCOMPARE(projectProxy->globalSetting("test0", "test0").toString(), QStringLiteral("test0"));
    projectProxy->setGlobalString("test1", "string");
    QCOMPARE(projectProxy->globalSetting("test1").toString(), QStringLiteral("string"));
    projectProxy->setGlobalString("test2", QVector3D(1, 2, 3));
    QCOMPARE(projectProxy->globalSetting("test2").toString(), QStringLiteral("1.00000,2.00000,3.00000"));
    projectProxy->setGlobalString("test3", QVector4D(4, 5, 6, 7));
    QCOMPARE(projectProxy->globalSetting("test3").toString(), QStringLiteral("4.00000,5.00000,6.00000,7.00000"));
    projectProxy->setGlobalString("test4", QQuaternion(11, 8, 9, 10));
    QCOMPARE(projectProxy->globalSetting("test4").toString(), QStringLiteral("8.00000,9.00000,10.00000,11.00000"));
}

void VPVMProjectTest::modelProjectSettings()
{
    ProjectProxy *projectProxy = new ProjectProxy(this);
    projectProxy->createAsync();
    IModel *model = projectProxy->factoryInstanceRef()->newModel(IModel::kPMDModel);
    ModelProxy *modelProxy = projectProxy->createModelProxy(model, QUuid::createUuid(), QUrl(), false);
    projectProxy->internalAddModel(modelProxy, false, true);
    /* add model to project instance manually because it does in RenderTarget */
    projectProxy->projectInstanceRef()->addModel(model, 0, modelProxy->uuid().toString().toStdString(), 0);
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test0"), QVariant());
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test0", "test0").toString(), QStringLiteral("test0"));
    projectProxy->setModelSetting(modelProxy, "test1", "string");
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test1").toString(), QStringLiteral("string"));
    projectProxy->setModelSetting(modelProxy, "test2", QVector3D(1, 2, 3));
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test2").toString(), QStringLiteral("1.00000,2.00000,3.00000"));
    projectProxy->setModelSetting(modelProxy, "test3", QVector4D(4, 5, 6, 7));
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test3").toString(), QStringLiteral("4.00000,5.00000,6.00000,7.00000"));
    projectProxy->setModelSetting(modelProxy, "test4", QQuaternion(11, 8, 9, 10));
    QCOMPARE(projectProxy->modelSetting(modelProxy, "test4").toString(), QStringLiteral("8.00000,9.00000,10.00000,11.00000"));
}

ModelProxy *VPVMProjectTest::createModelProxy(ProjectProxy *projectProxy)
{
    IModel *model = projectProxy->factoryInstanceRef()->newModel(IModel::kPMDModel);
    IBone *bone = model->createBone();
    ILabel *label = model->createLabel();
    IMorph *morph = model->createMorph();
    model->addBone(bone);
    model->addLabel(label);
    model->addMorph(morph);
    ModelProxy *modelProxy = projectProxy->createModelProxy(model, QUuid::createUuid(), QUrl(), false);
    projectProxy->addModel(modelProxy);
    return modelProxy;
}

MotionProxy *VPVMProjectTest::createMotionProxy(ProjectProxy *projectProxy)
{
    IMotion *motion = projectProxy->factoryInstanceRef()->newMotion(IMotion::kVMDMotion, 0);
    MotionProxy *motionProxy = projectProxy->createMotionProxy(motion, QUuid::createUuid(), QUrl(), true);
    return motionProxy;
}

QTEST_MAIN(VPVMProjectTest)

#include "tst_VPVMProjectTest.moc"

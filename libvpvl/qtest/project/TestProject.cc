#include <QtCore/QString>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>

using namespace vpvl;

namespace
{

class Delegate : public Project::IDelegate
{
public:
    Delegate()
        : m_codec(QTextCodec::codecForName("Shift-JIS"))
    {
    }
    ~Delegate() {
    }

    const std::string toUnicode(const std::string &value) {
        const QString &encoded = m_codec->toUnicode(value.c_str());
        return encoded.toStdString();
    }
    void error(const char *format, va_list ap) {
        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, format, ap);
    }
    void warning(const char *format, va_list ap) {
        fprintf(stderr, "WARN: ");
        vfprintf(stderr, format, ap);
    }

private:
    const QTextCodec *m_codec;
};

}

class TestProject : public QObject
{
    Q_OBJECT

public:
    static const std::string kModel1UUID;
    static const std::string kModel2UUID;
    static const std::string kAsset1UUID;
    static const std::string kAsset2UUID;
    static const std::string kMotionUUID;

    TestProject();
    ~TestProject();

private Q_SLOTS:
    void load();
    void save();
    void handleAssets();
    void handleModels();
    void handleMotions();

private:
    static void testGlobalSettings(const Project &project);
    static void testLocalSettings(const Project &project);
    static void testBoneAnimation(const VMDMotion *motion);
    static void testFaceAnimation(const VMDMotion *motion);
    static void testCameraAnimation(const VMDMotion *motion);
    static void testLightAnimation(const VMDMotion *motion);
};

const std::string TestProject::kAsset1UUID = "EEBC6A85-F333-429A-ADF8-B6188908A517";
const std::string TestProject::kAsset2UUID = "D4403C60-3D6C-4051-9B28-51DEFE021F59";
const std::string TestProject::kModel1UUID = "D41F00F2-FB75-4BFC-8DE8-0B1390F862F6";
const std::string TestProject::kModel2UUID = "B18ACADC-89FD-4945-9192-8E8FBC849E52";
const std::string TestProject::kMotionUUID = "E75F84CD-5DE0-4E95-A0DE-494E5AAE1DB6";

TestProject::TestProject()
{
}

TestProject::~TestProject()
{
}

void TestProject::load()
{
    Delegate delegate;
    Project project(&delegate);
    QVERIFY(project.load("../../../docs/project.xml"));
    QVERIFY(!project.isDirty());
    QCOMPARE(project.version(), 0.1f);
    QVERIFY(project.isPhysicsEnabled());
    QCOMPARE(project.assetUUIDs().size(), size_t(2));
    QCOMPARE(project.modelUUIDs().size(), size_t(2));
    QCOMPARE(project.motionUUIDs().size(), size_t(1));
    testGlobalSettings(project);
    testLocalSettings(project);
    VMDMotion *motion = project.motion(kMotionUUID);
    QCOMPARE(motion->parentModel(), project.model(kModel1UUID));
    testBoneAnimation(motion);
    testFaceAnimation(motion);
    testCameraAnimation(motion);
    testLightAnimation(motion);
}

void TestProject::save()
{
    Delegate delegate;
    Project project(&delegate);
    QVERIFY(project.load("../../../docs/project.xml"));
    QTemporaryFile file;
    file.open();
    file.setAutoRemove(true);
    project.setDirty(true);
    project.save(file.fileName().toUtf8());
    QVERIFY(!project.isDirty());
    Project project2(&delegate);
    QVERIFY(project2.load(file.fileName().toUtf8()));
    QCOMPARE(project2.version(), 0.1f);
    QVERIFY(project2.isPhysicsEnabled());
    QCOMPARE(project2.assetUUIDs().size(), size_t(2));
    QCOMPARE(project2.modelUUIDs().size(), size_t(2));
    QCOMPARE(project2.motionUUIDs().size(), size_t(1));
    testGlobalSettings(project2);
    testLocalSettings(project2);
    VMDMotion *motion = project2.motion(kMotionUUID);
    QCOMPARE(motion->parentModel(), project2.model(kModel1UUID));
    testBoneAnimation(motion);
    testFaceAnimation(motion);
    testCameraAnimation(motion);
    testLightAnimation(motion);
}

void TestProject::handleAssets()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Project project(&delegate);
    Asset *asset = new Asset(), *ptr = asset;
    QVERIFY(!project.containsAsset(ptr));
    QCOMPARE(project.assetUUID(0), Project::kNullUUID);
    project.addAsset(asset, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsAsset(ptr));
    QCOMPARE(project.assetUUID(asset), uuid.toStdString());
    QCOMPARE(project.asset(uuid.toStdString()), asset);
    QCOMPARE(project.asset(Project::kNullUUID), static_cast<Asset*>(0));
    project.deleteAsset(asset);
    QVERIFY(!project.containsAsset(ptr));
    QVERIFY(project.isDirty());
    QVERIFY(!asset);
    project.setDirty(false);
    project.deleteAsset(ptr);
    QVERIFY(!project.isDirty());
}

void TestProject::handleModels()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Project project(&delegate);
    PMDModel *model = new PMDModel(), *ptr = model;
    QVERIFY(!project.containsModel(ptr));
    QCOMPARE(project.modelUUID(0), Project::kNullUUID);
    project.addModel(model, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsModel(ptr));
    QCOMPARE(project.model(uuid.toStdString()), model);
    QCOMPARE(project.model(Project::kNullUUID), static_cast<PMDModel*>(0));
    project.deleteModel(model);
    QVERIFY(!project.containsModel(ptr));
    QVERIFY(project.isDirty());
    QVERIFY(!model);
    project.setDirty(false);
    project.deleteModel(ptr);
    QVERIFY(!project.isDirty());
}

void TestProject::handleMotions()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Project project(&delegate);
    PMDModel *model = new PMDModel();
    VMDMotion *motion = new VMDMotion(), *ptr = motion;
    QVERIFY(!project.containsMotion(ptr));
    QCOMPARE(project.motionUUID(0), Project::kNullUUID);
    project.addModel(model, "");
    project.addMotion(motion, model, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsMotion(ptr));
    QVERIFY(model->containsMotion(motion));
    QCOMPARE(project.motion(uuid.toStdString()), motion);
    QCOMPARE(project.motion(Project::kNullUUID), static_cast<VMDMotion*>(0));
    project.setDirty(false);
    project.deleteMotion(motion, model);
    QVERIFY(!project.containsMotion(ptr));
    QVERIFY(project.isDirty());
    QVERIFY(!model->containsMotion(motion));
    QVERIFY(!motion);
    project.setDirty(false);
    project.deleteMotion(ptr, model);
    QVERIFY(!project.isDirty());
}

void TestProject::testGlobalSettings(const Project &project)
{
    QCOMPARE(project.globalSetting("no_such_key").c_str(), "");
    QCOMPARE(project.globalSetting("width").c_str(), "640");
    QCOMPARE(project.globalSetting("height").c_str(), "480");
}

void TestProject::testLocalSettings(const Project &project)
{
    Asset *asset1 = project.asset(kAsset1UUID), *asset2 = project.asset(kAsset2UUID);
    PMDModel *model1 = project.model(kModel1UUID), *model2 = project.model(kModel2UUID);
    QCOMPARE(project.assetSetting(asset1, Project::kSettingURIKey).c_str(), "asset:/foo/bar/baz");
    QCOMPARE(project.modelSetting(model1, Project::kSettingURIKey).c_str(), "model:/foo/bar/baz");
    QCOMPARE(project.modelSetting(model1, "edge").c_str(), "1.0");
    QCOMPARE(project.assetSetting(asset2, Project::kSettingURIKey).c_str(), "asset:/baz/bar/foo");
    QCOMPARE(project.modelSetting(model2, Project::kSettingURIKey).c_str(), "model:/baz/bar/foo");
    QCOMPARE(project.modelSetting(model2, "edge").c_str(), "0.5");
}

void TestProject::testBoneAnimation(const VMDMotion *motion)
{
    const BoneAnimation &ba = motion->boneAnimation();
    QuadWord q;
    QCOMPARE(ba.countKeyFrames(), 2);
    QCOMPARE(ba.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(reinterpret_cast<const char *>(ba.frameAt(0)->name()), "bar");
    QCOMPARE(ba.frameAt(0)->position(), Vector3(1, 2, -3));
    QCOMPARE(ba.frameAt(0)->rotation(), Quaternion(-1, -2, 3, 4));
    for (int i = 0; i < BoneKeyFrame::kMax; i++) {
        int offset = i * 4;
        ba.frameAt(0)->getInterpolationParameter(static_cast<BoneKeyFrame::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 1, offset + 2, offset + 3, offset + 4));
    }
    QCOMPARE(ba.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(reinterpret_cast<const char *>(ba.frameAt(1)->name()), "baz");
    QCOMPARE(ba.frameAt(1)->position(), Vector3(3, 1, -2));
    QCOMPARE(ba.frameAt(1)->rotation(), Quaternion(-4, -3, 2, 1));
    for (int i = BoneKeyFrame::kMax - 1; i >= 0; i--) {
        int offset = (BoneKeyFrame::kMax - 1 - i) * 4;
        ba.frameAt(1)->getInterpolationParameter(static_cast<BoneKeyFrame::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 4, offset + 3, offset + 2, offset + 1));
    }
}

void TestProject::testFaceAnimation(const VMDMotion *motion)
{
    const FaceAnimation &fa = motion->faceAnimation();
    QCOMPARE(fa.countKeyFrames(), 2);
    QCOMPARE(fa.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(reinterpret_cast<const char *>(fa.frameAt(0)->name()), "bar");
    QCOMPARE(fa.frameAt(0)->weight(), 0.0f);
    QCOMPARE(fa.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(reinterpret_cast<const char *>(fa.frameAt(1)->name()), "baz");
    QCOMPARE(fa.frameAt(1)->weight(), 1.0f);
}

void TestProject::testCameraAnimation(const VMDMotion *motion)
{
    const CameraAnimation &ca = motion->cameraAnimation();
    QuadWord q;
    QCOMPARE(ca.countKeyFrames(), 2);
    QCOMPARE(ca.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(ca.frameAt(0)->position(), Vector3(1, 2, -3));
    QCOMPARE(ca.frameAt(0)->angle(), Vector3(-degree(1), -degree(2), -degree(3)));
    QCOMPARE(ca.frameAt(0)->fovy(), 15.0f);
    QCOMPARE(ca.frameAt(0)->distance(), 150.0f);
    for (int i = 0; i < CameraKeyFrame::kMax; i++) {
        int offset = i * 4;
        ca.frameAt(0)->getInterpolationParameter(static_cast<CameraKeyFrame::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 1, offset + 2, offset + 3, offset + 4));
    }
    QCOMPARE(ca.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(ca.frameAt(1)->position(), Vector3(3, 1, -2));
    QCOMPARE(ca.frameAt(1)->angle(), Vector3(-degree(3), -degree(1), -degree(2)));
    QCOMPARE(ca.frameAt(1)->fovy(), 30.0f);
    QCOMPARE(ca.frameAt(1)->distance(), 300.0f);
    for (int i = CameraKeyFrame::kMax - 1; i >= 0; i--) {
        int offset = (CameraKeyFrame::kMax - 1 - i) * 4;
        ca.frameAt(1)->getInterpolationParameter(static_cast<CameraKeyFrame::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 4, offset + 3, offset + 2, offset + 1));
    }
}

void TestProject::testLightAnimation(const VMDMotion *motion)
{
    const LightAnimation &la = motion->lightAnimation();
    QCOMPARE(la.countKeyFrames(), 2);
    QCOMPARE(la.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(la.frameAt(0)->color(), Vector3(1, 2, 3));
    QCOMPARE(la.frameAt(0)->direction(), Vector3(1, 2, 3));
    QCOMPARE(la.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(la.frameAt(1)->color(), Vector3(3, 1, 2));
    QCOMPARE(la.frameAt(1)->direction(), Vector3(3, 1, 2));
}

QTEST_APPLESS_MAIN(TestProject)

#include "TestProject.moc"

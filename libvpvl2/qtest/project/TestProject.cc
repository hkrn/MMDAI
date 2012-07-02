#include <QtCore/QString>
#include <QtTest/QtTest>

#include <vpvl2/vpvl2.h>
#include <vpvl2/Project.h>

#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

#include "../common.h"

using namespace vpvl2;

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

    const std::string toStdFromString(const IString *value) const {
        const std::string s(reinterpret_cast<const char *>(value->toByteArray()), value->length());
        return s;
    }
    IString *toStringFromStd(const std::string &value) const {
        const QString &s = m_codec->toUnicode(value.c_str());
        return new String(s);
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
    void handleNullUUID();

private:
    static void testGlobalSettings(const Project &project);
    static void testLocalSettings(const Project &project);
    static void testBoneAnimation(const IMotion *motion);
    static void testMorphAnimation(const IMotion *motion);
    static void testCameraAnimation(const IMotion *motion);
    static void testLightAnimation(const IMotion *motion);
};

const std::string TestProject::kAsset1UUID = "{EEBC6A85-F333-429A-ADF8-B6188908A517}";
const std::string TestProject::kAsset2UUID = "{D4403C60-3D6C-4051-9B28-51DEFE021F59}";
const std::string TestProject::kModel1UUID = "{D41F00F2-FB75-4BFC-8DE8-0B1390F862F6}";
const std::string TestProject::kModel2UUID = "{B18ACADC-89FD-4945-9192-8E8FBC849E52}";
const std::string TestProject::kMotionUUID = "{E75F84CD-5DE0-4E95-A0DE-494E5AAE1DB6}";

TestProject::TestProject()
{
}

TestProject::~TestProject()
{
}

void TestProject::load()
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QVERIFY(!Project(&delegate, &factory).load("../../../docs/project_uuid_dup.xml"));
    QVERIFY(project.load("../../../docs/project.xml"));
    QVERIFY(!project.isDirty());
    QCOMPARE(project.version().c_str(), "0.1");
    QCOMPARE(project.modelUUIDs().size(), size_t(4));
    QCOMPARE(project.motionUUIDs().size(), size_t(1));
    testGlobalSettings(project);
    testLocalSettings(project);
    IMotion *motion = project.motion(kMotionUUID);
    QCOMPARE(motion->parentModel(), project.model(kModel1UUID));
    testBoneAnimation(motion);
    testMorphAnimation(motion);
    testCameraAnimation(motion);
    testLightAnimation(motion);
}

void TestProject::save()
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QVERIFY(project.load("../../../docs/project.xml"));
    QTemporaryFile file;
    file.open();
    file.setAutoRemove(true);
    project.setDirty(true);
    project.save(file.fileName().toUtf8());
    QVERIFY(!project.isDirty());
    Project project2(&delegate, &factory);
    QVERIFY(project2.load(file.fileName().toUtf8()));
    QCOMPARE(project2.version().c_str(), libraryVersionString());
    QCOMPARE(project2.modelUUIDs().size(), size_t(4));
    QCOMPARE(project2.motionUUIDs().size(), size_t(1));
    testGlobalSettings(project2);
    testLocalSettings(project2);
    IMotion *motion = project2.motion(kMotionUUID);
    QCOMPARE(motion->parentModel(), project2.model(kModel1UUID));
    testBoneAnimation(motion);
    testMorphAnimation(motion);
    testCameraAnimation(motion);
    testLightAnimation(motion);
}

void TestProject::handleAssets()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    QVERIFY(!project.containsModel(ptr));
    QCOMPARE(project.modelUUID(0), Project::kNullUUID);
    project.addModel(ptr, 0, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsModel(ptr));
    QCOMPARE(project.modelUUIDs().size(), size_t(1));
    QCOMPARE(project.modelUUID(ptr), uuid.toStdString());
    QCOMPARE(project.model(uuid.toStdString()), ptr);
    QCOMPARE(project.model(Project::kNullUUID), static_cast<IModel*>(0));
    project.removeModel(ptr);
    QVERIFY(!project.containsModel(ptr));
    QCOMPARE(project.modelUUIDs().size(), size_t(0));
    QVERIFY(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    QVERIFY(!project.isDirty());
    ptr = asset.take();
    project.deleteModel(ptr);
    QVERIFY(!ptr);
}

void TestProject::handleModels()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    IModel *ptr = model.data();
    QVERIFY(!project.containsModel(ptr));
    QCOMPARE(project.modelUUID(0), Project::kNullUUID);
    project.addModel(ptr, 0, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsModel(ptr));
    QCOMPARE(project.modelUUIDs().size(), size_t(1));
    QCOMPARE(project.modelUUID(ptr), uuid.toStdString());
    QCOMPARE(project.model(uuid.toStdString()), ptr);
    QCOMPARE(project.model(Project::kNullUUID), static_cast<IModel*>(0));
    project.removeModel(ptr);
    QVERIFY(!project.containsModel(ptr));
    QCOMPARE(project.modelUUIDs().size(), size_t(0));
    QVERIFY(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    QVERIFY(!project.isDirty());
    ptr = model.take();
    project.deleteModel(ptr);
    QVERIFY(!ptr);
}

void TestProject::handleMotions()
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IMotion> motion(factory.createMotion());
    IMotion *ptr = motion.data();
    QVERIFY(!project.containsMotion(ptr));
    QCOMPARE(project.motionUUID(0), Project::kNullUUID);
    project.addMotion(ptr, uuid.toStdString());
    QVERIFY(project.isDirty());
    QVERIFY(project.containsMotion(ptr));
    QCOMPARE(project.motion(uuid.toStdString()), ptr);
    QCOMPARE(project.motion(Project::kNullUUID), static_cast<IMotion*>(0));
    project.setDirty(false);
    project.removeMotion(ptr);
    QVERIFY(!project.containsMotion(ptr));
    QVERIFY(project.isDirty());
}

void TestProject::handleNullUUID()
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    QCOMPARE(project.modelUUIDs().size(), size_t(1));
    project.removeModel(ptr);
    QCOMPARE(project.modelUUIDs().size(), size_t(0));
    ptr = asset.take();
    project.deleteModel(ptr);
    QVERIFY(!ptr);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    ptr = model.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    QCOMPARE(project.modelUUIDs().size(), size_t(1));
    project.removeModel(ptr);
    QCOMPARE(project.modelUUIDs().size(), size_t(0));
    ptr = model.take();
    project.deleteModel(ptr);
    QVERIFY(!ptr);
    QScopedPointer<IMotion> motion(factory.createMotion());
    IMotion *ptr2 = motion.data();
    project.addMotion(ptr2, Project::kNullUUID);
    QCOMPARE(project.motionUUIDs().size(), size_t(1));
    project.removeMotion(ptr2);
    QCOMPARE(project.motionUUIDs().size(), size_t(0));
    model.reset(factory.createModel(IModel::kPMD));
    ptr = model.data();
    motion.reset(factory.createMotion());
    ptr2 = motion.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    project.addMotion(ptr2, Project::kNullUUID);
    project.removeMotion(ptr2);
    QCOMPARE(project.motionUUIDs().size(), size_t(0));
    project.removeModel(ptr);
    ptr = model.take();
    project.deleteModel(ptr);
    QVERIFY(!ptr);
}

void TestProject::testGlobalSettings(const Project &project)
{
    QCOMPARE(project.globalSetting("no_such_key").c_str(), "");
    QCOMPARE(project.globalSetting("width").c_str(), "640");
    QCOMPARE(project.globalSetting("height").c_str(), "480");
}

void TestProject::testLocalSettings(const Project &project)
{
    IModel *asset1 = project.model(kAsset1UUID), *asset2 = project.model(kAsset2UUID);
    IModel *model1 = project.model(kModel1UUID), *model2 = project.model(kModel2UUID);
    QCOMPARE(project.modelSetting(asset1, Project::kSettingURIKey).c_str(), "asset:/foo/bar/baz");
    QCOMPARE(project.modelSetting(model1, Project::kSettingURIKey).c_str(), "model:/foo/bar/baz");
    QCOMPARE(project.modelSetting(model1, "edge").c_str(), "1.0");
    QCOMPARE(project.modelSetting(asset2, Project::kSettingURIKey).c_str(), "asset:/baz/bar/foo");
    QCOMPARE(project.modelSetting(model2, Project::kSettingURIKey).c_str(), "model:/baz/bar/foo");
    QCOMPARE(project.modelSetting(model2, "edge").c_str(), "0.5");
}

void TestProject::testBoneAnimation(const IMotion *motion)
{
    const vmd::BoneAnimation &ba = static_cast<const vmd::Motion *>(motion)->boneAnimation();
    const String bar("bar"), baz("baz");
    QuadWord q;
    QCOMPARE(ba.countKeyframes(), 2);
    QCOMPARE(ba.frameAt(0)->frameIndex(), 1.0f);
    QVERIFY(ba.frameAt(0)->name()->equals(&bar));
    QCOMPARE(ba.frameAt(0)->position(), Vector3(1, 2, -3));
    QCOMPARE(ba.frameAt(0)->rotation(), Quaternion(-1, -2, 3, 4));
    // QVERIFY(ba.frameAt(0)->isIKEnabled());
    for (int i = 0; i < IBoneKeyframe::kMax; i++) {
        int offset = i * 4;
        ba.frameAt(0)->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 1, offset + 2, offset + 3, offset + 4));
    }
    QCOMPARE(ba.frameAt(1)->frameIndex(), 2.0f);
    QVERIFY(ba.frameAt(1)->name()->equals(&baz));
    QCOMPARE(ba.frameAt(1)->position(), Vector3(3, 1, -2));
    QCOMPARE(ba.frameAt(1)->rotation(), Quaternion(-4, -3, 2, 1));
    // QVERIFY(!ba.frameAt(1)->isIKEnabled());
    for (int i = IBoneKeyframe::kMax - 1; i >= 0; i--) {
        int offset = (IBoneKeyframe::kMax - 1 - i) * 4;
        ba.frameAt(1)->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 4, offset + 3, offset + 2, offset + 1));
    }
}

void TestProject::testMorphAnimation(const IMotion *motion)
{
    const vmd::MorphAnimation &ma = static_cast<const vmd::Motion *>(motion)->morphAnimation();
    String bar("bar"), baz("baz");
    QCOMPARE(ma.countKeyframes(), 2);
    QCOMPARE(ma.frameAt(0)->frameIndex(), 1.0f);
    QVERIFY(ma.frameAt(0)->name()->equals(&bar));
    QCOMPARE(ma.frameAt(0)->weight(), 0.0f);
    QCOMPARE(ma.frameAt(1)->frameIndex(), 2.0f);
    QVERIFY(ma.frameAt(1)->name()->equals(&baz));
    QCOMPARE(ma.frameAt(1)->weight(), 1.0f);
}

void TestProject::testCameraAnimation(const IMotion *motion)
{
    const vmd::CameraAnimation &ca = static_cast<const vmd::Motion *>(motion)->cameraAnimation();
    QuadWord q;
    QCOMPARE(ca.countKeyframes(), 2);
    QCOMPARE(ca.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(ca.frameAt(0)->position(), Vector3(1, 2, -3));
    const Vector3 &angle1 = ca.frameAt(0)->angle();
    QVERIFY(qFuzzyCompare(angle1.x(), -degree(1)));
    QVERIFY(qFuzzyCompare(angle1.y(), -degree(2)));
    QVERIFY(qFuzzyCompare(angle1.z(), -degree(3)));
    QCOMPARE(ca.frameAt(0)->fov(), 15.0f);
    QCOMPARE(ca.frameAt(0)->distance(), 150.0f);
    for (int i = 0; i < ICameraKeyframe::kMax; i++) {
        int offset = i * 4;
        ca.frameAt(0)->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 1, offset + 2, offset + 3, offset + 4));
    }
    QCOMPARE(ca.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(ca.frameAt(1)->position(), Vector3(3, 1, -2));
    const Vector3 &angle2 = ca.frameAt(1)->angle();
    QVERIFY(qFuzzyCompare(angle2.x(), -degree(3)));
    QVERIFY(qFuzzyCompare(angle2.y(), -degree(1)));
    QVERIFY(qFuzzyCompare(angle2.z(), -degree(2)));
    QCOMPARE(ca.frameAt(1)->fov(), 30.0f);
    QCOMPARE(ca.frameAt(1)->distance(), 300.0f);
    for (int i = ICameraKeyframe::kMax - 1; i >= 0; i--) {
        int offset = (ICameraKeyframe::kMax - 1 - i) * 4;
        ca.frameAt(1)->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
        QCOMPARE(q, QuadWord(offset + 4, offset + 3, offset + 2, offset + 1));
    }
}

void TestProject::testLightAnimation(const IMotion *motion)
{
    const vmd::LightAnimation &la = static_cast<const vmd::Motion *>(motion)->lightAnimation();
    QCOMPARE(la.countKeyframes(), 2);
    QCOMPARE(la.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(la.frameAt(0)->color(), Vector3(1, 2, 3));
    QCOMPARE(la.frameAt(0)->direction(), Vector3(1, 2, 3));
    QCOMPARE(la.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(la.frameAt(1)->color(), Vector3(3, 1, 2));
    QCOMPARE(la.frameAt(1)->direction(), Vector3(3, 1, 2));
}

QTEST_APPLESS_MAIN(TestProject)

#include "TestProject.moc"

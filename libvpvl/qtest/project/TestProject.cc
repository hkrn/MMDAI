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

private:
    static void testGlobalSettings(const Project &project);
    static void testLocalSettings(const Project &project, const Array<Asset *> &assets, const Array<PMDModel *> &models);
    static void testBoneAnimation(const Array<VMDMotion *> &motions);
    static void testFaceAnimation(const Array<VMDMotion *> &motions);
    static void testCameraAnimation(const Array<VMDMotion *> &motions);
    static void testLightAnimation(const Array<VMDMotion *> &motions);
};

TestProject::TestProject()
{
}

void TestProject::load()
{
    Project project;
    QVERIFY(project.load("project.xml"));
    const Array<Asset *> &assets = project.assets();
    QCOMPARE(assets.count(), 2);
    const Array<PMDModel *> &models = project.models();
    QCOMPARE(models.count(), 2);
    const Array<VMDMotion *> &motions = project.motions();
    QCOMPARE(motions.count(), 1);
    testGlobalSettings(project);
    testLocalSettings(project, assets, models);
    testBoneAnimation(motions);
    testFaceAnimation(motions);
    testCameraAnimation(motions);
    testLightAnimation(motions);
}

void TestProject::testGlobalSettings(const Project &project)
{
    QCOMPARE(project.globalSetting("no_such_key").c_str(), "");
    QCOMPARE(project.globalSetting("width").c_str(), "640");
    QCOMPARE(project.globalSetting("height").c_str(), "480");
}

void TestProject::testLocalSettings(const Project &project, const Array<Asset *> &assets, const Array<PMDModel *> &models)
{
    QCOMPARE(project.localAssetSetting(assets.at(0), "path").c_str(), "asset:/foo/bar/baz");
    QCOMPARE(project.localModelSetting(models.at(0), "path").c_str(), "model:/foo/bar/baz");
    QCOMPARE(project.localModelSetting(models.at(0), "edge").c_str(), "1.0");
    QCOMPARE(project.localAssetSetting(assets.at(1), "path").c_str(), "asset:/baz/bar/foo");
    QCOMPARE(project.localModelSetting(models.at(1), "path").c_str(), "model:/baz/bar/foo");
    QCOMPARE(project.localModelSetting(models.at(1), "edge").c_str(), "0.5");
}

void TestProject::testBoneAnimation(const Array<VMDMotion *> &motions)
{
    const BoneAnimation &ba = motions.at(0)->boneAnimation();
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

void TestProject::testFaceAnimation(const Array<VMDMotion *> &motions)
{
    const FaceAnimation &fa = motions.at(0)->faceAnimation();
    QCOMPARE(fa.countKeyFrames(), 2);
    QCOMPARE(fa.frameAt(0)->frameIndex(), 1.0f);
    QCOMPARE(reinterpret_cast<const char *>(fa.frameAt(0)->name()), "bar");
    QCOMPARE(fa.frameAt(0)->weight(), 0.0f);
    QCOMPARE(fa.frameAt(1)->frameIndex(), 2.0f);
    QCOMPARE(reinterpret_cast<const char *>(fa.frameAt(1)->name()), "baz");
    QCOMPARE(fa.frameAt(1)->weight(), 1.0f);
}

void TestProject::testCameraAnimation(const Array<VMDMotion *> &motions)
{
    const CameraAnimation &ca = motions.at(0)->cameraAnimation();
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

void TestProject::testLightAnimation(const Array<VMDMotion *> &motions)
{
    const LightAnimation &la = motions.at(0)->lightAnimation();
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

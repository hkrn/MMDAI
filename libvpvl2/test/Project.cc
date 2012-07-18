#include <gtest/gtest.h>
#include <QtCore/QtCore>

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

#include "Common.h"

using namespace ::testing;
using namespace vpvl2;

namespace
{

const Project::UUID kAsset1UUID = "{EEBC6A85-F333-429A-ADF8-B6188908A517}";
const Project::UUID kAsset2UUID = "{D4403C60-3D6C-4051-9B28-51DEFE021F59}";
const Project::UUID kModel1UUID = "{D41F00F2-FB75-4BFC-8DE8-0B1390F862F6}";
const Project::UUID kModel2UUID = "{B18ACADC-89FD-4945-9192-8E8FBC849E52}";
const Project::UUID kMotionUUID = "{E75F84CD-5DE0-4E95-A0DE-494E5AAE1DB6}";

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


static void TestGlobalSettings(const Project &project)
{
    EXPECT_STREQ("", project.globalSetting("no_such_key").c_str());
    EXPECT_STREQ("640", project.globalSetting("width").c_str());
    EXPECT_STREQ("480", project.globalSetting("height").c_str());
}

static void TestLocalSettings(const Project &project)
{
    IModel *asset1 = project.model(kAsset1UUID), *asset2 = project.model(kAsset2UUID);
    IModel *model1 = project.model(kModel1UUID), *model2 = project.model(kModel2UUID);
    EXPECT_STREQ("asset:/foo/bar/baz", project.modelSetting(asset1, Project::kSettingURIKey).c_str());
    EXPECT_STREQ("model:/foo/bar/baz", project.modelSetting(model1, Project::kSettingURIKey).c_str());
    EXPECT_STREQ("1.0", project.modelSetting(model1, "edge").c_str());
    EXPECT_STREQ("asset:/baz/bar/foo", project.modelSetting(asset2, Project::kSettingURIKey).c_str());
    EXPECT_STREQ("model:/baz/bar/foo", project.modelSetting(model2, Project::kSettingURIKey).c_str());
    EXPECT_STREQ("0.5", project.modelSetting(model2, "edge").c_str());
}

static void TestBoneAnimation(const IMotion *motion)
{
    const vmd::BoneAnimation &ba = static_cast<const vmd::Motion *>(motion)->boneAnimation();
    const String bar("bar"), baz("baz");
    QuadWord q;
    EXPECT_EQ(2, ba.countKeyframes());
    EXPECT_EQ(IKeyframe::TimeIndex(1), ba.frameAt(0)->timeIndex());
    EXPECT_TRUE(ba.frameAt(0)->name()->equals(&bar));
    EXPECT_EQ(Vector3(1, 2, -3), ba.frameAt(0)->position());
    EXPECT_EQ(Quaternion(-1, -2, 3, 4), ba.frameAt(0)->rotation());
    // EXPECT_TRUE(ba.frameAt(0)->isIKEnabled());
    for (int i = 0; i < IBoneKeyframe::kMax; i++) {
        int offset = i * 4;
        ba.frameAt(0)->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        EXPECT_EQ(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q);
    }
    EXPECT_EQ(IKeyframe::TimeIndex(2), ba.frameAt(1)->timeIndex());
    EXPECT_TRUE(ba.frameAt(1)->name()->equals(&baz));
    EXPECT_EQ(Vector3(3, 1, -2), ba.frameAt(1)->position());
    EXPECT_EQ(Quaternion(-4, -3, 2, 1), ba.frameAt(1)->rotation());
    // EXPECT_FALSE(ba.frameAt(1)->isIKEnabled());
    for (int i = IBoneKeyframe::kMax - 1; i >= 0; i--) {
        int offset = (IBoneKeyframe::kMax - 1 - i) * 4;
        ba.frameAt(1)->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        EXPECT_EQ(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q);
    }
}

static void TestMorphAnimation(const IMotion *motion)
{
    const vmd::MorphAnimation &ma = static_cast<const vmd::Motion *>(motion)->morphAnimation();
    String bar("bar"), baz("baz");
    EXPECT_EQ(2, ma.countKeyframes());
    EXPECT_EQ(IKeyframe::TimeIndex(1), ma.frameAt(0)->timeIndex());
    EXPECT_TRUE(ma.frameAt(0)->name()->equals(&bar));
    EXPECT_EQ(IMorph::WeightPrecision(0), ma.frameAt(0)->weight());
    EXPECT_EQ(IKeyframe::TimeIndex(2), ma.frameAt(1)->timeIndex());
    EXPECT_TRUE(ma.frameAt(1)->name()->equals(&baz));
    EXPECT_EQ(IMorph::WeightPrecision(1), ma.frameAt(1)->weight());
}

static void TestCameraAnimation(const IMotion *motion)
{
    const vmd::CameraAnimation &ca = static_cast<const vmd::Motion *>(motion)->cameraAnimation();
    QuadWord q;
    EXPECT_EQ(2, ca.countKeyframes());
    EXPECT_EQ(IKeyframe::TimeIndex(1), ca.frameAt(0)->timeIndex());
    EXPECT_EQ(Vector3(1, 2, -3), ca.frameAt(0)->position());
    const Vector3 &angle1 = ca.frameAt(0)->angle();
    EXPECT_TRUE(qFuzzyCompare(angle1.x(), -degree(1)));
    EXPECT_TRUE(qFuzzyCompare(angle1.y(), -degree(2)));
    EXPECT_TRUE(qFuzzyCompare(angle1.z(), -degree(3)));
    EXPECT_EQ(15.0f, ca.frameAt(0)->fov());
    EXPECT_EQ(150.0f, ca.frameAt(0)->distance());
    for (int i = 0; i < ICameraKeyframe::kMax; i++) {
        int offset = i * 4;
        ca.frameAt(0)->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
        EXPECT_EQ(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q);
    }
    EXPECT_EQ(IKeyframe::TimeIndex(2), ca.frameAt(1)->timeIndex());
    EXPECT_EQ(Vector3(3, 1, -2), ca.frameAt(1)->position());
    const Vector3 &angle2 = ca.frameAt(1)->angle();
    EXPECT_TRUE(qFuzzyCompare(angle2.x(), -degree(3)));
    EXPECT_TRUE(qFuzzyCompare(angle2.y(), -degree(1)));
    EXPECT_TRUE(qFuzzyCompare(angle2.z(), -degree(2)));
    EXPECT_EQ(30.0f, ca.frameAt(1)->fov());
    EXPECT_EQ(300.0f, ca.frameAt(1)->distance());
    for (int i = ICameraKeyframe::kMax - 1; i >= 0; i--) {
        int offset = (ICameraKeyframe::kMax - 1 - i) * 4;
        ca.frameAt(1)->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
        EXPECT_EQ(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q);
    }
}

static void TestLightAnimation(const IMotion *motion)
{
    const vmd::LightAnimation &la = static_cast<const vmd::Motion *>(motion)->lightAnimation();
    EXPECT_EQ(2, la.countKeyframes());
    EXPECT_EQ(IKeyframe::TimeIndex(1), la.frameAt(0)->timeIndex());
    EXPECT_EQ(Vector3(1, 2, 3), la.frameAt(0)->color());
    EXPECT_EQ(Vector3(1, 2, 3), la.frameAt(0)->direction());
    EXPECT_EQ(IKeyframe::TimeIndex(2), la.frameAt(1)->timeIndex());
    EXPECT_EQ(Vector3(3, 1, 2), la.frameAt(1)->color());
    EXPECT_EQ(Vector3(3, 1, 2), la.frameAt(1)->direction());
}


}

TEST(Project, Load)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    EXPECT_FALSE(Project(&delegate, &factory).load("../../docs/project_uuid_dup.xml"));
    EXPECT_TRUE(project.load("../../docs/project.xml"));
    EXPECT_FALSE(project.isDirty());
    EXPECT_STREQ("0.1", project.version().c_str());
    EXPECT_EQ(size_t(4), project.modelUUIDs().size());
    EXPECT_EQ(size_t(1), project.motionUUIDs().size());
    TestGlobalSettings(project);
    TestLocalSettings(project);
    IMotion *motion = project.motion(kMotionUUID);
    EXPECT_EQ(project.model(kModel1UUID), motion->parentModel());
    TestBoneAnimation(motion);
    TestMorphAnimation(motion);
    TestCameraAnimation(motion);
    TestLightAnimation(motion);
}

TEST(Project, Save)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    EXPECT_TRUE(project.load("../../docs/project.xml"));
    QTemporaryFile file;
    file.open();
    file.setAutoRemove(true);
    project.setDirty(true);
    project.save(file.fileName().toUtf8());
    EXPECT_FALSE(project.isDirty());
    Project project2(&delegate, &factory);
    EXPECT_TRUE(project2.load(file.fileName().toUtf8()));
    EXPECT_STREQ(libraryVersionString(), project2.version().c_str());
    EXPECT_EQ(size_t(4), project2.modelUUIDs().size());
    EXPECT_EQ(size_t(1), project2.motionUUIDs().size());
    TestGlobalSettings(project2);
    TestLocalSettings(project2);
    IMotion *motion = project2.motion(kMotionUUID);
    EXPECT_EQ(project2.model(kModel1UUID), motion->parentModel());
    TestBoneAnimation(motion);
    TestMorphAnimation(motion);
    TestCameraAnimation(motion);
    TestLightAnimation(motion);
}

TEST(Project, HandleAssets)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    EXPECT_FALSE(project.containsModel(ptr));
    EXPECT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(ptr, 0, uuid.toStdString());
    EXPECT_TRUE(project.isDirty());
    EXPECT_TRUE(project.containsModel(ptr));
    EXPECT_EQ(size_t(1), project.modelUUIDs().size());
    EXPECT_EQ(uuid.toStdString(), project.modelUUID(ptr));
    EXPECT_EQ(ptr, project.model(uuid.toStdString()));
    EXPECT_EQ(static_cast<IModel*>(0), project.model(Project::kNullUUID));
    project.removeModel(ptr);
    EXPECT_FALSE(project.containsModel(ptr));
    EXPECT_EQ(size_t(0), project.modelUUIDs().size());
    EXPECT_TRUE(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    EXPECT_FALSE(project.isDirty());
    ptr = asset.take();
    project.deleteModel(ptr);
    EXPECT_FALSE(ptr);
}

TEST(Project, HandleModels)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    IModel *ptr = model.data();
    EXPECT_FALSE(project.containsModel(ptr));
    EXPECT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(ptr, 0, uuid.toStdString());
    EXPECT_TRUE(project.isDirty());
    EXPECT_TRUE(project.containsModel(ptr));
    EXPECT_EQ(size_t(1), project.modelUUIDs().size());
    EXPECT_EQ(uuid.toStdString(), project.modelUUID(ptr));
    EXPECT_EQ(ptr, project.model(uuid.toStdString()));
    EXPECT_EQ(static_cast<IModel*>(0), project.model(Project::kNullUUID));
    project.removeModel(ptr);
    EXPECT_FALSE(project.containsModel(ptr));
    EXPECT_EQ(size_t(0), project.modelUUIDs().size());
    EXPECT_TRUE(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    EXPECT_FALSE(project.isDirty());
    ptr = model.take();
    project.deleteModel(ptr);
    EXPECT_FALSE(ptr);
}

TEST(Project, HandleMotions)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IMotion> motion(factory.createMotion());
    IMotion *ptr = motion.data();
    EXPECT_FALSE(project.containsMotion(ptr));
    EXPECT_EQ(Project::kNullUUID, project.motionUUID(0));
    project.addMotion(ptr, uuid.toStdString());
    EXPECT_TRUE(project.isDirty());
    EXPECT_TRUE(project.containsMotion(ptr));
    EXPECT_EQ(ptr, project.motion(uuid.toStdString()));
    EXPECT_EQ(static_cast<IMotion*>(0), project.motion(Project::kNullUUID));
    project.setDirty(false);
    project.removeMotion(ptr);
    EXPECT_FALSE(project.containsMotion(ptr));
    EXPECT_TRUE(project.isDirty());
}

TEST(Project, HandleNullUUID)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    EXPECT_EQ(size_t(1), project.modelUUIDs().size());
    project.removeModel(ptr);
    EXPECT_EQ(size_t(0), project.modelUUIDs().size());
    ptr = asset.take();
    project.deleteModel(ptr);
    EXPECT_FALSE(ptr);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    ptr = model.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    EXPECT_EQ(size_t(1), project.modelUUIDs().size());
    project.removeModel(ptr);
    EXPECT_EQ(size_t(0), project.modelUUIDs().size());
    ptr = model.take();
    project.deleteModel(ptr);
    EXPECT_FALSE(ptr);
    QScopedPointer<IMotion> motion(factory.createMotion());
    IMotion *ptr2 = motion.data();
    project.addMotion(ptr2, Project::kNullUUID);
    EXPECT_EQ(size_t(1), project.motionUUIDs().size());
    project.removeMotion(ptr2);
    EXPECT_EQ(size_t(0), project.motionUUIDs().size());
    model.reset(factory.createModel(IModel::kPMD));
    ptr = model.data();
    motion.reset(factory.createMotion());
    ptr2 = motion.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    project.addMotion(ptr2, Project::kNullUUID);
    project.removeMotion(ptr2);
    EXPECT_EQ(size_t(0), project.motionUUIDs().size());
    project.removeModel(ptr);
    ptr = model.take();
    project.deleteModel(ptr);
    EXPECT_FALSE(ptr);
}

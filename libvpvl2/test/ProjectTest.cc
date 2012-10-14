#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/Project.h"
#include "vpvl2/icu/Encoding.h"
#include "vpvl2/mvd/AssetKeyframe.h"
#include "vpvl2/mvd/AssetSection.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/ProjectKeyframe.h"
#include "vpvl2/mvd/ProjectSection.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::icu;

namespace
{

const Project::UUID kAsset1UUID = "{EEBC6A85-F333-429A-ADF8-B6188908A517}";
const Project::UUID kAsset2UUID = "{D4403C60-3D6C-4051-9B28-51DEFE021F59}";
const Project::UUID kModel1UUID = "{D41F00F2-FB75-4BFC-8DE8-0B1390F862F6}";
const Project::UUID kModel2UUID = "{B18ACADC-89FD-4945-9192-8E8FBC849E52}";
const Project::UUID kMotion1UUID = "{E75F84CD-5DE0-4E95-A0DE-494E5AAE1DB6}";
const Project::UUID kMotion2UUID = "{481E1B4E-FC24-4D61-841D-C8AB7CF1096D}";

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
        const std::string &s = String::toStdString(static_cast<const String *>(value)->value());
        return s;
    }
    IString *toStringFromStd(const std::string &value) const {
        const QString &s = m_codec->toUnicode(value.c_str());
        return new String(UnicodeString::fromUTF8(s.toStdString()));
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
    ASSERT_STREQ("", project.globalSetting("no_such_key").c_str());
    ASSERT_STREQ("640", project.globalSetting("width").c_str());
    ASSERT_STREQ("480", project.globalSetting("height").c_str());
}

static void TestLocalSettings(const Project &project)
{
    IModel *asset1 = project.findModel(kAsset1UUID), *asset2 = project.findModel(kAsset2UUID);
    IModel *model1 = project.findModel(kModel1UUID), *model2 = project.findModel(kModel2UUID);
    ASSERT_STREQ("asset:/foo/bar/baz", project.modelSetting(asset1, Project::kSettingURIKey).c_str());
    ASSERT_STREQ("model:/foo/bar/baz", project.modelSetting(model1, Project::kSettingURIKey).c_str());
    ASSERT_STREQ("1.0", project.modelSetting(model1, "edge").c_str());
    ASSERT_STREQ("asset:/baz/bar/foo", project.modelSetting(asset2, Project::kSettingURIKey).c_str());
    ASSERT_STREQ("model:/baz/bar/foo", project.modelSetting(model2, Project::kSettingURIKey).c_str());
    ASSERT_STREQ("0.5", project.modelSetting(model2, "edge").c_str());
}

static void TestBoneMotion(const IMotion *motion, bool hasLayer)
{
    const IBoneKeyframe *keyframe1 = motion->findBoneKeyframeAt(0);
    const String bar("bar"), baz("baz");
    QuadWord q;
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kBone));
    ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe1->timeIndex());
    ASSERT_TRUE(keyframe1->name()->equals(&bar));
    ASSERT_EQ(Vector3(1, 2, -3), keyframe1->position());
    ASSERT_EQ(Quaternion(-1, -2, 3, 4), keyframe1->rotation());
    // ASSERT_TRUE(ba.frameAt(0)->isIKEnabled());
    for (int i = 0; i < IBoneKeyframe::kMaxInterpolationType; i++) {
        int offset = i * 4;
        keyframe1->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        ASSERT_EQ(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q);
    }
    const IBoneKeyframe *keyframe2 = motion->findBoneKeyframeAt(1);
    ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe2->timeIndex());
    ASSERT_EQ(IKeyframe::LayerIndex(hasLayer ? 1 : 0), keyframe2->layerIndex());
    ASSERT_TRUE(keyframe2->name()->equals(&baz));
    ASSERT_EQ(Vector3(3, 1, -2), keyframe2->position());
    ASSERT_EQ(Quaternion(-4, -3, 2, 1), keyframe2->rotation());
    // ASSERT_FALSE(ba.frameAt(1)->isIKEnabled());
    for (int i = IBoneKeyframe::kMaxInterpolationType - 1; i >= 0; i--) {
        int offset = (IBoneKeyframe::kMaxInterpolationType - 1 - i) * 4;
        keyframe2->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
        ASSERT_EQ(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q);
    }
}

static void TestMorphMotion(const IMotion *motion)
{
    String bar("bar"), baz("baz");
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kMorph));
    const IMorphKeyframe *keyframe1 = motion->findMorphKeyframeAt(0);
    ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe1->timeIndex());
    ASSERT_TRUE(keyframe1->name()->equals(&bar));
    ASSERT_EQ(IMorph::WeightPrecision(0), keyframe1->weight());
    const IMorphKeyframe *keyframe2 = motion->findMorphKeyframeAt(1);
    ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe2->timeIndex());
    ASSERT_TRUE(keyframe2->name()->equals(&baz));
    ASSERT_EQ(IMorph::WeightPrecision(1), keyframe2->weight());
}

static void TestCameraMotion(const IMotion *motion, bool hasLayer, bool skipYZ)
{
    QuadWord q;
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kCamera));
    const ICameraKeyframe *keyframe1 = motion->findCameraKeyframeAt(0);
    ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe1->timeIndex());
    ASSERT_EQ(Vector3(1, 2, -3), keyframe1->position());
    const Vector3 &angle1 = keyframe1->angle();
    ASSERT_TRUE(qFuzzyCompare(angle1.x(), -degree(1)));
    ASSERT_TRUE(qFuzzyCompare(angle1.y(), -degree(2)));
    ASSERT_TRUE(qFuzzyCompare(angle1.z(), -degree(3)));
    ASSERT_EQ(15.0f, keyframe1->fov());
    ASSERT_EQ(150.0f, keyframe1->distance());
    if (skipYZ) {
        for (int i = 0; i < ICameraKeyframe::kMaxInterpolationType; i++) {
            int offset = (i < 3 ? 0 : i - 2) * 4;
            keyframe1->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
            ASSERT_EQ(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q);
        }
    }
    else {
        for (int i = 0; i < ICameraKeyframe::kMaxInterpolationType; i++) {
            int offset = i * 4;
            keyframe1->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
            ASSERT_EQ(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q);
        }
    }
    const ICameraKeyframe *keyframe2 = motion->findCameraKeyframeAt(1);
    ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe2->timeIndex());
    ASSERT_EQ(IKeyframe::LayerIndex(hasLayer ? 1 : 0), keyframe2->layerIndex());
    ASSERT_EQ(Vector3(3, 1, -2), keyframe2->position());
    const Vector3 &angle2 = keyframe2->angle();
    ASSERT_TRUE(qFuzzyCompare(angle2.x(), -degree(3)));
    ASSERT_TRUE(qFuzzyCompare(angle2.y(), -degree(1)));
    ASSERT_TRUE(qFuzzyCompare(angle2.z(), -degree(2)));
    ASSERT_EQ(30.0f, keyframe2->fov());
    ASSERT_EQ(300.0f, keyframe2->distance());
    if (skipYZ) {
        for (int max = ICameraKeyframe::kMaxInterpolationType - 1, i = max; i >= 0; i--) {
            int offset = qMin((max - i) * 4, 12);
            keyframe2->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
            ASSERT_EQ(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q);
        }
    }
    else {
        for (int max = ICameraKeyframe::kMaxInterpolationType - 1, i = max; i >= 0; i--) {
            int offset = (max - i) * 4;
            keyframe2->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
            ASSERT_EQ(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q);
        }
    }
}

static void TestLightMotion(const IMotion *motion)
{
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kLight));
    const ILightKeyframe *keyframe1 = motion->findLightKeyframeAt(0);
    ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe1->timeIndex());
    ASSERT_EQ(Vector3(1, 2, 3), keyframe1->color());
    ASSERT_EQ(Vector3(1, 2, 3), keyframe1->direction());
    const ILightKeyframe *keyframe2 = motion->findLightKeyframeAt(1);
    ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe2->timeIndex());
    ASSERT_EQ(Vector3(3, 1, 2), keyframe2->color());
    ASSERT_EQ(Vector3(3, 1, 2), keyframe2->direction());
}

}

TEST(ProjectTest, Load)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    ASSERT_FALSE(Project(&delegate, &factory).load("../../docs/project_uuid_dup.xml"));
    ASSERT_TRUE(project.load("../../docs/project.xml"));
    ASSERT_FALSE(project.isDirty());
    ASSERT_STREQ("0.1", project.version().c_str());
    ASSERT_EQ(size_t(4), project.modelUUIDs().size());
    ASSERT_EQ(size_t(2), project.motionUUIDs().size());
    TestGlobalSettings(project);
    TestLocalSettings(project);
    IMotion *motion = project.findMotion(kMotion1UUID);
    ASSERT_EQ(IMotion::kVMD, motion->type());
    ASSERT_EQ(project.findModel(kModel1UUID), motion->parentModel());
    TestBoneMotion(motion, false);
    TestMorphMotion(motion);
    TestCameraMotion(motion, false, false);
    TestLightMotion(motion);
    IMotion *motion2 = project.findMotion(kMotion2UUID);
    ASSERT_EQ(IMotion::kMVD, motion2->type());
    ASSERT_EQ(project.findModel(kModel2UUID), motion2->parentModel());
    TestBoneMotion(motion2, true);
    TestMorphMotion(motion2);
    TestCameraMotion(motion2, true, true);
    TestLightMotion(motion2);
}

TEST(ProjectTest, Save)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    ASSERT_TRUE(project.load("../../docs/project.xml"));
    QTemporaryFile file;
    file.open();
    file.setAutoRemove(true);
    project.setDirty(true);
    project.save(file.fileName().toUtf8());
    ASSERT_FALSE(project.isDirty());
    Project project2(&delegate, &factory);
    ASSERT_TRUE(project2.load(file.fileName().toUtf8()));
    QString s;
    s.sprintf("%.1f", Project::formatVersion());
    ASSERT_STREQ(qPrintable(s), project2.version().c_str());
    ASSERT_EQ(size_t(4), project2.modelUUIDs().size());
    ASSERT_EQ(size_t(2), project2.motionUUIDs().size());
    TestGlobalSettings(project2);
    TestLocalSettings(project2);
    IMotion *motion = project2.findMotion(kMotion1UUID);
    ASSERT_EQ(project2.findModel(kModel1UUID), motion->parentModel());
    ASSERT_EQ(IMotion::kVMD, motion->type());
    TestBoneMotion(motion, false);
    TestMorphMotion(motion);
    TestCameraMotion(motion, false, false);
    TestLightMotion(motion);
    IMotion *motion2 = project2.findMotion(kMotion2UUID);
    ASSERT_EQ(IMotion::kMVD, motion2->type());
    ASSERT_EQ(project2.findModel(kModel2UUID), motion2->parentModel());
    TestBoneMotion(motion2, true);
    TestMorphMotion(motion2);
    TestCameraMotion(motion2, true, true);
    TestLightMotion(motion2);
}

TEST(ProjectTest, HandleAssets)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    ASSERT_FALSE(project.containsModel(ptr));
    ASSERT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(ptr, 0, uuid.toStdString());
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsModel(ptr));
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    ASSERT_EQ(uuid.toStdString(), project.modelUUID(ptr));
    ASSERT_EQ(ptr, project.findModel(uuid.toStdString()));
    ASSERT_EQ(static_cast<IModel*>(0), project.findModel(Project::kNullUUID));
    project.removeModel(ptr);
    ASSERT_FALSE(project.containsModel(ptr));
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ASSERT_TRUE(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    ASSERT_FALSE(project.isDirty());
    ptr = asset.take();
    project.deleteModel(ptr);
    ASSERT_FALSE(ptr);
}

TEST(ProjectTest, HandleModels)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    IModel *ptr = model.data();
    ASSERT_FALSE(project.containsModel(ptr));
    ASSERT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(ptr, 0, uuid.toStdString());
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsModel(ptr));
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    ASSERT_EQ(uuid.toStdString(), project.modelUUID(ptr));
    ASSERT_EQ(ptr, project.findModel(uuid.toStdString()));
    ASSERT_EQ(static_cast<IModel*>(0), project.findModel(Project::kNullUUID));
    project.removeModel(ptr);
    ASSERT_FALSE(project.containsModel(ptr));
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ASSERT_TRUE(project.isDirty());
    project.setDirty(false);
    project.removeModel(ptr);
    ASSERT_FALSE(project.isDirty());
    ptr = model.take();
    project.deleteModel(ptr);
    ASSERT_FALSE(ptr);
}

TEST(ProjectTest, HandleMotions)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    IMotion *ptr = motion.data();
    ASSERT_FALSE(project.containsMotion(ptr));
    ASSERT_EQ(Project::kNullUUID, project.motionUUID(0));
    project.addMotion(ptr, uuid.toStdString());
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsMotion(ptr));
    ASSERT_EQ(ptr, project.findMotion(uuid.toStdString()));
    ASSERT_EQ(static_cast<IMotion*>(0), project.findMotion(Project::kNullUUID));
    project.setDirty(false);
    project.removeMotion(ptr);
    ASSERT_FALSE(project.containsMotion(ptr));
    ASSERT_TRUE(project.isDirty());
}

TEST(ProjectTest, HandleNullUUID)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *ptr = asset.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    project.removeModel(ptr);
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ptr = asset.take();
    project.deleteModel(ptr);
    ASSERT_FALSE(ptr);
    QScopedPointer<IModel> model(factory.createModel(IModel::kPMD));
    ptr = model.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    project.removeModel(ptr);
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ptr = model.take();
    project.deleteModel(ptr);
    ASSERT_FALSE(ptr);
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    IMotion *ptr2 = motion.data();
    project.addMotion(ptr2, Project::kNullUUID);
    ASSERT_EQ(size_t(1), project.motionUUIDs().size());
    project.removeMotion(ptr2);
    ASSERT_EQ(size_t(0), project.motionUUIDs().size());
    model.reset(factory.createModel(IModel::kPMD));
    ptr = model.data();
    motion.reset(factory.createMotion(IMotion::kVMD, 0));
    ptr2 = motion.data();
    project.addModel(ptr, 0, Project::kNullUUID);
    project.addMotion(ptr2, Project::kNullUUID);
    project.removeMotion(ptr2);
    ASSERT_EQ(size_t(0), project.motionUUIDs().size());
    project.removeModel(ptr);
    ptr = model.take();
    project.deleteModel(ptr);
    ASSERT_FALSE(ptr);
}

#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/Project.h"
#include "vpvl2/extensions/icu/Encoding.h"
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
using namespace vpvl2::extensions::icu;

namespace
{

const Project::UUID kAsset1UUID = "{EEBC6A85-F333-429A-ADF8-B6188908A517}";
const Project::UUID kAsset2UUID = "{D4403C60-3D6C-4051-9B28-51DEFE021F59}";
const Project::UUID kModel1UUID = "{D41F00F2-FB75-4BFC-8DE8-0B1390F862F6}";
const Project::UUID kModel2UUID = "{B18ACADC-89FD-4945-9192-8E8FBC849E52}";
const Project::UUID kMotion1UUID = "{E75F84CD-5DE0-4E95-A0DE-494E5AAE1DB6}";
const Project::UUID kMotion2UUID = "{481E1B4E-FC24-4D61-841D-C8AB7CF1096D}";
const Project::UUID kMotion3UUID = "{766CF45D-DE91-4387-9704-4B3D5B1414DC}";

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
        if (value) {
            const std::string &s = String::toStdString(static_cast<const String *>(value)->value());
            return s;
        }
        return std::string();
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
    const String bar("bar"), baz("baz");
    QuadWord q;
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kBone));
    {
        const IBoneKeyframe *keyframe = motion->findBoneKeyframeAt(0);
        ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
        ASSERT_TRUE(keyframe->name()->equals(&bar));
        ASSERT_TRUE(CompareVector(Vector3(1, 2, -3), keyframe->localPosition()));
        ASSERT_TRUE(CompareVector(Quaternion(-1, -2, 3, 4), keyframe->localRotation()));
        // ASSERT_TRUE(ba.frameAt(0)->isIKEnabled());
        for (int i = 0; i < IBoneKeyframe::kMaxInterpolationType; i++) {
            int offset = i * 4;
            keyframe->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
            ASSERT_TRUE(CompareVector(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q));
        }
    }
    {
        const IBoneKeyframe *keyframe = motion->findBoneKeyframeAt(1);
        ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
        ASSERT_EQ(IKeyframe::LayerIndex(hasLayer ? 1 : 0), keyframe->layerIndex());
        ASSERT_TRUE(keyframe->name()->equals(&baz));
        ASSERT_TRUE(CompareVector(Vector3(3, 1, -2), keyframe->localPosition()));
        ASSERT_TRUE(CompareVector(Quaternion(-4, -3, 2, 1), keyframe->localRotation()));
        // ASSERT_FALSE(ba.frameAt(1)->isIKEnabled());
        for (int i = IBoneKeyframe::kMaxInterpolationType - 1; i >= 0; i--) {
            int offset = (IBoneKeyframe::kMaxInterpolationType - 1 - i) * 4;
            keyframe->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), q);
            ASSERT_TRUE(CompareVector(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q));
        }
    }
}

static void TestMorphMotion(const IMotion *motion)
{
    String bar("bar"), baz("baz");
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kMorph));
    {
        const IMorphKeyframe *keyframe = motion->findMorphKeyframeAt(0);
        ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
        ASSERT_TRUE(keyframe->name()->equals(&bar));
        ASSERT_EQ(IMorph::WeightPrecision(0), keyframe->weight());
    }
    {
        const IMorphKeyframe *keyframe = motion->findMorphKeyframeAt(1);
        ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
        ASSERT_TRUE(keyframe->name()->equals(&baz));
        ASSERT_EQ(IMorph::WeightPrecision(1), keyframe->weight());
    }
}

static void TestCameraMotion(const IMotion *motion, bool hasLayer)
{
    QuadWord q;
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kCamera));
    {
        const ICameraKeyframe *keyframe = motion->findCameraKeyframeAt(0);
        ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
        ASSERT_TRUE(CompareVector(Vector3(1, 2, -3), keyframe->lookAt()));
        const Vector3 &angle1 = keyframe->angle();
        ASSERT_TRUE(qFuzzyCompare(angle1.x(), -degree(1)));
        ASSERT_TRUE(qFuzzyCompare(angle1.y(), -degree(2)));
        ASSERT_TRUE(qFuzzyCompare(angle1.z(), -degree(3)));
        ASSERT_EQ(15.0f, keyframe->fov());
        ASSERT_EQ(150.0f, keyframe->distance());
        if (motion->type() == IMotion::kMVD) {
            for (int i = 0; i < ICameraKeyframe::kMaxInterpolationType; i++) {
                int offset = (i < 3 ? 0 : i - 2) * 4;
                keyframe->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
                ASSERT_TRUE(CompareVector(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q));
            }
        }
        else {
            for (int i = 0; i < ICameraKeyframe::kMaxInterpolationType; i++) {
                int offset = i * 4;
                keyframe->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
                ASSERT_TRUE(CompareVector(QuadWord(offset + 1, offset + 2, offset + 3, offset + 4), q));
            }
        }
    }
    {
        const ICameraKeyframe *keyframe = motion->findCameraKeyframeAt(1);
        ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
        ASSERT_EQ(IKeyframe::LayerIndex(hasLayer ? 1 : 0), keyframe->layerIndex());
        ASSERT_EQ(Vector3(3, 1, -2), keyframe->lookAt());
        const Vector3 &angle2 = keyframe->angle();
        ASSERT_TRUE(qFuzzyCompare(angle2.x(), -degree(3)));
        ASSERT_TRUE(qFuzzyCompare(angle2.y(), -degree(1)));
        ASSERT_TRUE(qFuzzyCompare(angle2.z(), -degree(2)));
        ASSERT_EQ(30.0f, keyframe->fov());
        ASSERT_EQ(300.0f, keyframe->distance());
        if (motion->type() == IMotion::kMVD) {
            for (int max = ICameraKeyframe::kMaxInterpolationType - 1, i = max; i >= 0; i--) {
                int offset = qMin((max - i) * 4, 12);
                keyframe->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
                ASSERT_TRUE(CompareVector(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q));
            }
        }
        else {
            for (int max = ICameraKeyframe::kMaxInterpolationType - 1, i = max; i >= 0; i--) {
                int offset = (max - i) * 4;
                keyframe->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), q);
                ASSERT_TRUE(CompareVector(QuadWord(offset + 4, offset + 3, offset + 2, offset + 1), q));
            }
        }
    }
}

static void TestLightMotion(const IMotion *motion)
{
    ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kLight));
    {
        const ILightKeyframe *keyframe = motion->findLightKeyframeAt(0);
        ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
        ASSERT_TRUE(CompareVector(Vector3(1, 2, 3), keyframe->color()));
        ASSERT_TRUE(CompareVector(Vector3(1, 2, 3), keyframe->direction()));
    }
    {
        const ILightKeyframe *keyframe = motion->findLightKeyframeAt(1);
        ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
        ASSERT_TRUE(CompareVector(Vector3(3, 1, 2), keyframe->color()));
        ASSERT_TRUE(CompareVector(Vector3(3, 1, 2), keyframe->direction()));
    }
}

static void TestEffectMotion(const IMotion *motion)
{
    if (motion->type() == IMotion::kMVD) {
        ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kEffect));
        {
            const IEffectKeyframe *keyframe = motion->findEffectKeyframeAt(0);
            ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
            ASSERT_EQ(true, keyframe->isVisible());
            ASSERT_EQ(false, keyframe->isAddBlendEnabled());
            ASSERT_EQ(true, keyframe->isShadowEnabled());
            ASSERT_EQ(0.42f, keyframe->scaleFactor());
            ASSERT_EQ(0.24f, keyframe->opacity());
        }
        {
            const IEffectKeyframe *keyframe = motion->findEffectKeyframeAt(1);
            ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
            ASSERT_EQ(false, keyframe->isVisible());
            ASSERT_EQ(true, keyframe->isAddBlendEnabled());
            ASSERT_EQ(false, keyframe->isShadowEnabled());
            ASSERT_EQ(0.24f, keyframe->scaleFactor());
            ASSERT_EQ(0.42f, keyframe->opacity());
        }
    }
    else {
        ASSERT_EQ(0, motion->countKeyframes(IKeyframe::kEffect));
    }
}

static void TestModelMotion(const IMotion *motion)
{
    if (motion->type() == IMotion::kMVD) {
        ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kModel));
        {
            const IModelKeyframe *keyframe = motion->findModelKeyframeAt(0);
            ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
            ASSERT_EQ(true, keyframe->isVisible());
            ASSERT_EQ(false, keyframe->isAddBlendEnabled());
            ASSERT_EQ(true, keyframe->isPhysicsEnabled());
            ASSERT_EQ(0, keyframe->physicsStillMode());
            ASSERT_EQ(0.24f, keyframe->edgeWidth());
            ASSERT_TRUE(CompareVector(Color(0.1, 0.2, 0.3, 0.4), keyframe->edgeColor()));
        }
        {
            const IModelKeyframe *keyframe = motion->findModelKeyframeAt(1);
            ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
            ASSERT_EQ(false, keyframe->isVisible());
            ASSERT_EQ(true, keyframe->isAddBlendEnabled());
            ASSERT_EQ(false, keyframe->isPhysicsEnabled());
            ASSERT_EQ(1, keyframe->physicsStillMode());
            ASSERT_EQ(0.42f, keyframe->edgeWidth());
            ASSERT_TRUE(CompareVector(Color(0.4, 0.3, 0.2, 0.1), keyframe->edgeColor()));
        }
    }
    else {
        ASSERT_EQ(0, motion->countKeyframes(IKeyframe::kModel));
    }
}

static void TestProjectMotion(const IMotion *motion)
{
    if (motion->type() == IMotion::kMVD) {
        ASSERT_EQ(2, motion->countKeyframes(IKeyframe::kProject));
        {
            const IProjectKeyframe *keyframe = motion->findProjectKeyframeAt(0);
            ASSERT_EQ(IKeyframe::TimeIndex(1), keyframe->timeIndex());
            ASSERT_EQ(9.8f, keyframe->gravityFactor());
            ASSERT_TRUE(CompareVector(Vector3(0.1, 0.2, 0.3), keyframe->gravityDirection()));
            ASSERT_EQ(0, keyframe->shadowMode());
            ASSERT_EQ(0.24f, keyframe->shadowDepth());
            ASSERT_EQ(0.42f, keyframe->shadowDistance());
        }
        {
            const IProjectKeyframe *keyframe = motion->findProjectKeyframeAt(1);
            ASSERT_EQ(IKeyframe::TimeIndex(2), keyframe->timeIndex());
            ASSERT_EQ(8.9f, keyframe->gravityFactor());
            ASSERT_TRUE(CompareVector(Vector3(0.3, 0.1, 0.2), keyframe->gravityDirection()));
            ASSERT_EQ(1, keyframe->shadowMode());
            ASSERT_EQ(0.42f, keyframe->shadowDepth());
            ASSERT_EQ(0.24f, keyframe->shadowDistance());
        }
    }
    else {
        ASSERT_EQ(0, motion->countKeyframes(IKeyframe::kProject));
    }
}

}

TEST(ProjectTest, Load)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    /* duplicated UUID doesn't allow */
    ASSERT_FALSE(Project(&delegate, &factory).load("../../docs/project_uuid_dup.xml"));
    ASSERT_TRUE(project.load("../../docs/project.xml"));
    ASSERT_FALSE(project.isDirty());
    ASSERT_STREQ("0.1", project.version().c_str());
    ASSERT_EQ(size_t(4), project.modelUUIDs().size());
    ASSERT_EQ(size_t(3), project.motionUUIDs().size());
    TestGlobalSettings(project);
    TestLocalSettings(project);
    /* VMD motion for model */
    IMotion *motion = project.findMotion(kMotion1UUID);
    ASSERT_EQ(IMotion::kVMD, motion->type());
    ASSERT_EQ(project.findModel(kModel1UUID), motion->parentModelRef());
    TestBoneMotion(motion, false);
    TestMorphMotion(motion);
    TestCameraMotion(motion, false);
    TestLightMotion(motion);
    TestEffectMotion(motion);
    TestModelMotion(motion);
    TestProjectMotion(motion);
    /* MVD motion */
    IMotion *motion2 = project.findMotion(kMotion2UUID);
    ASSERT_EQ(IMotion::kMVD, motion2->type());
    ASSERT_EQ(project.findModel(kModel2UUID), motion2->parentModelRef());
    TestBoneMotion(motion2, true);
    TestMorphMotion(motion2);
    TestCameraMotion(motion2, true);
    TestLightMotion(motion2);
    TestEffectMotion(motion2);
    TestModelMotion(motion2);
    TestProjectMotion(motion2);
    /* VMD motion for asset */
    IMotion *motion3 = project.findMotion(kMotion3UUID);
    ASSERT_EQ(IMotion::kVMD, motion3->type());
    ASSERT_EQ(project.findModel(kAsset2UUID), motion3->parentModelRef());
    TestBoneMotion(motion3, false);
    TestMorphMotion(motion3);
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
    ASSERT_EQ(size_t(3), project2.motionUUIDs().size());
    TestGlobalSettings(project2);
    TestLocalSettings(project2);
    /* VMD motion for model */
    IMotion *motion = project2.findMotion(kMotion1UUID);
    ASSERT_EQ(project2.findModel(kModel1UUID), motion->parentModelRef());
    ASSERT_EQ(IMotion::kVMD, motion->type());
    TestBoneMotion(motion, false);
    TestMorphMotion(motion);
    TestCameraMotion(motion, false);
    TestLightMotion(motion);
    TestEffectMotion(motion);
    TestModelMotion(motion);
    TestProjectMotion(motion);
    /* MVD motion */
    IMotion *motion2 = project2.findMotion(kMotion2UUID);
    ASSERT_EQ(IMotion::kMVD, motion2->type());
    ASSERT_EQ(project2.findModel(kModel2UUID), motion2->parentModelRef());
    TestBoneMotion(motion2, true);
    TestMorphMotion(motion2);
    TestCameraMotion(motion2, true);
    TestLightMotion(motion2);
    TestEffectMotion(motion2);
    TestModelMotion(motion2);
    TestProjectMotion(motion2);
    /* VMD motion for asset */
    IMotion *motion3 = project.findMotion(kMotion3UUID);
    ASSERT_EQ(IMotion::kVMD, motion3->type());
    ASSERT_EQ(project.findModel(kAsset2UUID), motion3->parentModelRef());
    TestBoneMotion(motion3, false);
    TestMorphMotion(motion3);
}

TEST(ProjectTest, HandleAssets)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *model = asset.data();
    /* before adding an asset to the project */
    ASSERT_FALSE(project.containsModel(model));
    ASSERT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(model, 0, uuid.toStdString());
    /* after adding an asset to the project */
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsModel(model));
    /* reference will be null because render engine instance is not passed */
    ASSERT_EQ(static_cast<Scene *>(0), model->parentSceneRef());
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    ASSERT_EQ(uuid.toStdString(), project.modelUUID(model));
    ASSERT_EQ(model, project.findModel(uuid.toStdString()));
    /* finding inexists asset should returns null */
    ASSERT_EQ(static_cast<IModel*>(0), project.findModel(Project::kNullUUID));
    project.removeModel(model);
    ASSERT_EQ(static_cast<Scene *>(0), model->parentSceneRef());
    /* finding removed asset should returns null */
    ASSERT_FALSE(project.containsModel(model));
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ASSERT_TRUE(project.isDirty());
    project.setDirty(false);
    /* removing removed asset should not be dirty */
    project.removeModel(model);
    ASSERT_FALSE(project.isDirty());
    model = asset.take();
    /* deleting removed asset should do nothing */
    project.deleteModel(model);
    ASSERT_FALSE(model);
}

TEST(ProjectTest, HandleModels)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> modelPtr(factory.createModel(IModel::kPMD));
    IModel *model = modelPtr.data();
    /* before adding a model to the project */
    ASSERT_FALSE(project.containsModel(model));
    ASSERT_EQ(Project::kNullUUID, project.modelUUID(0));
    project.addModel(model, 0, uuid.toStdString());
    /* before adding a model to the project */
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsModel(model));
    /* reference will be null because render engine instance is not passed */
    ASSERT_EQ(static_cast<Scene *>(0), model->parentSceneRef());
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    ASSERT_EQ(uuid.toStdString(), project.modelUUID(model));
    ASSERT_EQ(model, project.findModel(uuid.toStdString()));
    /* finding inexists model should returns null */
    ASSERT_EQ(static_cast<IModel*>(0), project.findModel(Project::kNullUUID));
    project.removeModel(model);
    ASSERT_EQ(static_cast<Scene *>(0), model->parentSceneRef());
    /* finding removed model should returns null */
    ASSERT_FALSE(project.containsModel(model));
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    ASSERT_TRUE(project.isDirty());
    project.setDirty(false);
    /* removing removed model should not be dirty */
    project.removeModel(model);
    ASSERT_FALSE(project.isDirty());
    model = modelPtr.take();
    /* deleting removed model should do nothing */
    project.deleteModel(model);
    ASSERT_FALSE(model);
}

TEST(ProjectTest, HandleMotions)
{
    const QString &uuid = QUuid::createUuid().toString();
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IMotion> motionPtr(factory.createMotion(IMotion::kVMD, 0));
    IMotion *motion = motionPtr.data();
    /* before adding a motion to the project */
    ASSERT_FALSE(project.containsMotion(motion));
    ASSERT_EQ(Project::kNullUUID, project.motionUUID(0));
    project.addMotion(motion, uuid.toStdString());
    /* after adding a motion to the project */
    ASSERT_TRUE(project.isDirty());
    ASSERT_TRUE(project.containsMotion(motion));
    ASSERT_EQ(motion, project.findMotion(uuid.toStdString()));
    /* finding inexists motion should returns null */
    ASSERT_EQ(static_cast<IMotion*>(0), project.findMotion(Project::kNullUUID));
    project.setDirty(false);
    /* finding removed motion should returns null */
    project.removeMotion(motion);
    ASSERT_FALSE(project.containsMotion(motion));
    ASSERT_TRUE(project.isDirty());
    project.setDirty(false);
    /* removing removed motion should not be dirty */
    project.removeMotion(motion);
    ASSERT_FALSE(project.isDirty());
}

TEST(ProjectTest, HandleNullUUID)
{
    Delegate delegate;
    Encoding encoding;
    Factory factory(&encoding);
    Project project(&delegate, &factory);
    QScopedPointer<IModel> asset(factory.createModel(IModel::kAsset));
    IModel *model = asset.data();
    /* null model can be added */
    project.addModel(model, 0, Project::kNullUUID);
    /* reference will be null because render engine instance is not passed */
    ASSERT_EQ(static_cast<Scene *>(0), model->parentSceneRef());
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    /* and null model can be removed */
    project.removeModel(model);
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    model = asset.take();
    project.deleteModel(model);
    ASSERT_FALSE(model);
    QScopedPointer<IModel> modelPtr(factory.createModel(IModel::kPMD));
    model = modelPtr.data();
    /* null model can be added */
    project.addModel(model, 0, Project::kNullUUID);
    /* reference will be null because render engine instance is not passed */
    ASSERT_EQ(0, model->parentSceneRef());
    ASSERT_EQ(size_t(1), project.modelUUIDs().size());
    /* and null model can be removed */
    project.removeModel(model);
    ASSERT_EQ(size_t(0), project.modelUUIDs().size());
    model = modelPtr.take();
    project.deleteModel(model);
    ASSERT_FALSE(model);
    QScopedPointer<IMotion> motionPtr(factory.createMotion(IMotion::kVMD, 0));
    IMotion *motion = motionPtr.data();
    /* null motion can be added */
    project.addMotion(motion, Project::kNullUUID);
    ASSERT_EQ(size_t(1), project.motionUUIDs().size());
    /* and null motion can be removed */
    project.removeMotion(motion);
    ASSERT_EQ(size_t(0), project.motionUUIDs().size());
    modelPtr.reset(factory.createModel(IModel::kPMD));
    model = modelPtr.data();
    motionPtr.reset(factory.createMotion(IMotion::kVMD, 0));
    motion = motionPtr.data();
    /* duplicated null motion should be integrated into one */
    project.addModel(model, 0, Project::kNullUUID);
    project.addMotion(motion, Project::kNullUUID);
    project.removeMotion(motion);
    ASSERT_EQ(size_t(0), project.motionUUIDs().size());
    project.removeModel(model);
    model = modelPtr.take();
    project.deleteModel(model);
    ASSERT_FALSE(model);
}

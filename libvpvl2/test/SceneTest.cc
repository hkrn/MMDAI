#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/extensions/icu/Encoding.h"
#include "mock/Model.h"
#include "mock/Motion.h"
#include "mock/RenderContext.h"
#include "mock/RenderEngine.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu;

TEST(SceneTest, AddModel)
{
    Scene scene;
    /* adding an null model should not be crashed */
    scene.addModel(0, 0);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    /* adding a model but no rendering engine should not be added */
    scene.addModel(model.data(), 0);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    /* no rendering context class will be referered */
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    /* adding a rendering engine but no model should not be added */
    scene.addModel(0, engine.data());
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    scene.addModel(model.take(), engine.take());
    /* both model and rendering engine should be added */
    ASSERT_EQ(1, scene.models().count());
    ASSERT_EQ(1, scene.renderEngines().count());
}

TEST(SceneTest, AddMotion)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene;
    /* adding an null motion should not be crashed */
    scene.addMotion(0);
    ASSERT_EQ(0, scene.motions().count());
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    scene.addMotion(motion.take());
    ASSERT_EQ(1, scene.motions().count());
}

TEST(SceneTest, FindModel)
{
    Scene scene;
    /* adding an null motion should not be crashed */
    scene.findModel(0);
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("foo_bar_baz")), s2(s.value());
    EXPECT_CALL(*model, name()).WillOnce(Return(&s));
    scene.addModel(model.data(), engine.take());
    ASSERT_EQ(model.take(), scene.findModel(&s2));
}

TEST(SceneTest, FindRenderEngine)
{
    Scene scene;
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    scene.addModel(model.data(), engine.data());
    ASSERT_EQ(engine.take(), scene.findRenderEngine(model.take()));
}

TEST(SceneTest, DeleteModel)
{
    Scene scene;
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    IModel *fakePtr = 0;
    /* deleting an null model should not be crashed */
    scene.deleteModel(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addModel(model.data(), engine.take());
    IModel *modelPtr = model.data();
    /* model should be deleted and set it null */
    scene.deleteModel(modelPtr);
    model.take();
    ASSERT_EQ(0, modelPtr);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
}

TEST(SceneTest, Update)
{
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateRenderEngines);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateAll);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).Times(0);
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateCamera);
        scene.update(Scene::kUpdateLight);
        scene.update(Scene::kUpdateModels);
    }
}

TEST(SceneTest, RemoveMotion)
{
    Factory factory(0);
    Scene scene;
    /* removing an null motion should not be crashed */
    scene.removeMotion(0);
    ASSERT_EQ(0, scene.motions().count());
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    scene.addMotion(motion.data());
    /* motion should be removed and set it null */
    scene.removeMotion(motion.data());
    ASSERT_EQ(0, scene.motions().count());
}

TEST(SceneTest, AdvanceMotions)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, SeekMotions)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, Camera)
{
    Scene scene;
    QScopedPointer<ICamera> camera1(scene.createCamera()), camera2(scene.createCamera());
    ASSERT_NE(camera2.data(), camera1.data());
    ASSERT_EQ(scene.camera(), scene.camera());
}

TEST(SceneTest, AdvanceSceneCamera)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateCamera);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.camera()->setMotion(0);
    }
    scene.camera()->setMotion(0);
}

TEST(SceneTest, SeekSceneCamera)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateCamera);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.camera()->setMotion(0);
    }
    scene.camera()->setMotion(0);
}

TEST(SceneTest, Light)
{
    Scene scene;
    QScopedPointer<ILight> light1(scene.createLight()), light2(scene.createLight());
    ASSERT_NE(light2.data(), light1.data());
    ASSERT_EQ(scene.light(), scene.light());
}

TEST(SceneTest, AdvanceSceneLight)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateLight);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.light()->setMotion(0);
    }
    scene.light()->setMotion(0);
}

TEST(SceneTest, SeekSceneLight)
{
    Scene scene;
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateLight);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.light()->setMotion(0);
    }
    scene.light()->setMotion(0);
}

class SceneModelTest : public TestWithParam<IModel::Type> {};

TEST_P(SceneModelTest, SetParentSceneRef)
{
    Encoding encoding;
    Factory factory(&encoding);
    MockIRenderContext renderContext;
    EXPECT_CALL(renderContext, findProcedureAddress(_)).WillRepeatedly(Return(static_cast<void *>(0)));
    Scene scene;
    IModel::Type type = GetParam();
    QScopedPointer<IModel> modelPtr(factory.createModel(type));
    QScopedPointer<IRenderEngine> enginePtr(scene.createRenderEngine(&renderContext, modelPtr.data(), 0));
    scene.addModel(modelPtr.data(), enginePtr.take());
    ASSERT_EQ(&scene, modelPtr->parentSceneRef());
    scene.removeModel(modelPtr.data());
    ASSERT_EQ(static_cast<Scene *>(0), modelPtr->parentSceneRef());
    IModel *m = modelPtr.take();
    scene.deleteModel(m);
}

class SceneMotionTest : public TestWithParam<IMotion::Type> {};

TEST_P(SceneMotionTest, SetParentSceneRefForScene)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene;
    IMotion::Type type = GetParam();
    QScopedPointer<IMotion> cameraMotion(factory.createMotion(type, 0));
    scene.camera()->setMotion(cameraMotion.data());
    ASSERT_EQ(&scene, cameraMotion->parentSceneRef());
    scene.camera()->setMotion(0);
    ASSERT_EQ(static_cast<Scene *>(0), cameraMotion->parentSceneRef());
    QScopedPointer<IMotion> lightMotion(factory.createMotion(type, 0));
    scene.light()->setMotion(lightMotion.data());
    ASSERT_EQ(&scene, lightMotion->parentSceneRef());
    scene.light()->setMotion(0);
    ASSERT_EQ(static_cast<Scene *>(0), lightMotion->parentSceneRef());
}

TEST_P(SceneMotionTest, SetParentSceneRefForModel)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene;
    IMotion::Type type = GetParam();
    QScopedPointer<IMotion> motion(factory.createMotion(type, 0));
    scene.addMotion(motion.data());
    ASSERT_EQ(&scene, motion->parentSceneRef());
    scene.removeMotion(motion.data());
    ASSERT_EQ(static_cast<Scene *>(0), motion->parentSceneRef());
}

INSTANTIATE_TEST_CASE_P(SceneInstance, SceneModelTest, Values(IModel::kAsset, IModel::kPMD, IModel::kPMX));
INSTANTIATE_TEST_CASE_P(SceneInstance, SceneMotionTest, Values(IMotion::kMVD, IMotion::kVMD));

#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "mock/Model.h"
#include "mock/Motion.h"
#include "mock/RenderContext.h"
#include "mock/RenderEngine.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/cg/AssetRenderEngine.h"
#include "vpvl2/cg/PMXRenderEngine.h"
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/extensions/World.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

TEST(SceneTest, AddModel)
{
    Array<IModel *> models;
    Array<IRenderEngine *> engines;
    Scene scene(true);
    /* adding an null model should not be crashed */
    scene.addModel(0, 0, 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    /* adding a model but no rendering engine should not be added */
    scene.addModel(model.data(), 0, 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
    /* no rendering context class will be referered */
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    /* adding a rendering engine but no model should not be added */
    scene.addModel(0, engine.data(), 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
    scene.addModel(model.take(), engine.take(), 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    /* both model and rendering engine should be added */
    ASSERT_EQ(1, models.count());
    ASSERT_EQ(1, engines.count());
}

TEST(SceneTest, AddMotion)
{
    Array<IMotion *> motions;
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene(true);
    /* adding an null motion should not be crashed */
    scene.addMotion(0);
    scene.getMotionRefs(motions);
    ASSERT_EQ(0, motions.count());
    QScopedPointer<IMotion> motion(factory.newMotion(IMotion::kVMDMotion, 0));
    scene.addMotion(motion.take());
    scene.getMotionRefs(motions);
    ASSERT_EQ(1, motions.count());
}

TEST(SceneTest, FindModel)
{
    Scene scene(true);
    /* adding an null motion should not be crashed */
    scene.findModel(0);
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("foo_bar_baz")), s2(s.value());
    EXPECT_CALL(*model, name()).WillOnce(Return(&s));
    scene.addModel(model.data(), engine.take(), 0);
    ASSERT_EQ(model.take(), scene.findModel(&s2));
}

TEST(SceneTest, FindRenderEngine)
{
    Scene scene(true);
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    scene.addModel(model.data(), engine.data(), 0);
    ASSERT_EQ(engine.take(), scene.findRenderEngine(model.take()));
}

TEST(SceneTest, RemoveModel)
{
    Array<IModel *> models;
    Array<IRenderEngine *> engines;
    Scene scene(true);
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    EXPECT_CALL(*model, leaveWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    /* removing an null model should do nothing */
    scene.removeModel(0);
    scene.addModel(model.data(), engine.take(), 0);
    /* model should be deleted and set it null */
    scene.removeModel(model.data());
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
}

TEST(SceneTest, DeleteModel)
{
    Array<IModel *> models;
    Array<IRenderEngine *> engines;
    Scene scene(true);
    QScopedPointer<MockIModel> model(new MockIModel());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    EXPECT_CALL(*model, leaveWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    IModel *fakePtr = 0;
    /* deleting an null model should not be crashed */
    scene.deleteModel(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addModel(model.data(), engine.take(), 0);
    IModel *modelPtr = model.data();
    /* model should be deleted and set it null */
    scene.deleteModel(modelPtr);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    model.take();
    ASSERT_EQ(0, modelPtr);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
}

TEST(SceneTest, RemoveMotion)
{
    Array<IMotion *> motions;
    Scene scene(true);
    QScopedPointer<MockIMotion> motion(new MockIMotion());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
    /* removing an null model should do nothing */
    scene.removeMotion(0);
    scene.addMotion(motion.data());
    /* model should be deleted and set it null */
    scene.removeMotion(motion.data());
    scene.getMotionRefs(motions);
    ASSERT_EQ(0, motions.count());
}

TEST(SceneTest, DeleteMotion)
{
    Array<IMotion *> motions;
    Scene scene(true);
    QScopedPointer<MockIMotion> motion(new MockIMotion());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
    IMotion *fakePtr = 0;
    /* deleting an null motion should not be crashed */
    scene.deleteMotion(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addMotion(motion.data());
    IMotion *motionPtr = motion.data();
    /* model should be deleted and set it null */
    scene.deleteMotion(motionPtr);
    scene.getMotionRefs(motions);
    motion.take();
    ASSERT_EQ(0, motionPtr);
    ASSERT_EQ(0, motions.count());
}

TEST(SceneTest, Update)
{
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.take(), engine.take(), 0);
        scene.update(Scene::kUpdateRenderEngines);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.take(), engine.take(), 0);
        scene.update(Scene::kUpdateAll);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).Times(0);
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.take(), engine.take(), 0);
        scene.update(Scene::kUpdateCamera);
        scene.update(Scene::kUpdateLight);
        scene.update(Scene::kUpdateModels);
    }
}

TEST(SceneTest, AdvanceMotions)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#advance should be called once if Scene#advance with kUpdateModels is called */
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#advance should be called once if Scene#advance with kUpdateAll is called */
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#advance should not be called if Scene#advance with kUpdate(Camera|Light|RenderEngines) is called */
        EXPECT_CALL(motion, advance(0)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, SeekMotions)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#seek should be called once if Scene#seek with kUpdateModels is called */
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#seek should be called once if Scene#seek with kUpdateAll is called */
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(&motion);
        /* IMotion#seek should not be called if Scene#seek with kUpdate(Camera|Light|RenderEngines) is called */
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, Camera)
{
    Scene scene(true);
    QScopedPointer<ICamera> camera1(scene.createCamera()), camera2(scene.createCamera());
    ASSERT_NE(camera2.data(), camera1.data());
    ASSERT_EQ(scene.camera(), scene.camera());
}

TEST(SceneTest, AdvanceSceneCamera)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateCamera is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateCamera);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateAll is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#advanceScene should not be called if Scene#advance with kUpdate(Light|Models|RenderEngines) is called */
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
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateCamera is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateCamera);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateAll is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.camera()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.camera()->setMotion(&motion);
        /* IMotion#seekScene should not be called if Scene#seek with kUpdate(Light|Models|RenderEngines) is called */
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
    Scene scene(true);
    QScopedPointer<ILight> light1(scene.createLight()), light2(scene.createLight());
    ASSERT_NE(light2.data(), light1.data());
    ASSERT_EQ(scene.light(), scene.light());
}

TEST(SceneTest, AdvanceSceneLight)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateLight is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateLight);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateAll is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#advanceScene should not be called if Scene#advance with kUpdate(Camera|Models|RenderEngines) is called */
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
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateLight is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateLight);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateAll is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.light()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.light()->setMotion(&motion);
        /* IMotion#seekScene should not be called if Scene#seek with kUpdate(Camera|Models|RenderEngines) is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.light()->setMotion(0);
    }
    scene.light()->setMotion(0);
}

TEST(SceneTest, SetWorldRef)
{
    extensions::World world;
    btDiscreteDynamicsWorld *worldRef = world.dynamicWorldRef();
    {
        // 1. call setWorldRef first and addModel without removing model
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.setWorldRef(worldRef);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.addModel(model.data(), engine.take(), 0);
        model.take();
    }
    {
        // 2. add model first and call setWorldRef without removing model
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        scene.addModel(model.data(), engine.take(), 0);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.setWorldRef(worldRef);
        model.take();
    }
    {
        // 3. deleting (removing) model explicitly (result should be same as 2)
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene(true);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        scene.addModel(model.data(), engine.take(), 0);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.setWorldRef(worldRef);
        IModel *m = model.take();
        scene.deleteModel(m);
    }
}

TEST(SceneTest, CreateRenderEngine)
{
    Scene scene(true);
    MockIRenderContext context;
    EXPECT_CALL(context, findProcedureAddress(_)).WillRepeatedly(Return(static_cast<void *>(0)));
    {
        asset::Model model(0); /* no IEncoding instance will be referred */
        QScopedPointer<IRenderEngine> engine(scene.createRenderEngine(&context, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::AssetRenderEngine *>(engine.data()));
        engine.reset(scene.createRenderEngine(&context, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<cg::AssetRenderEngine *>(engine.data()));
    }
    {
        pmd::Model model(0); /* no IEncoding instance will be referred */
        QScopedPointer<IRenderEngine> engine(scene.createRenderEngine(&context, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::PMXRenderEngine *>(engine.data()));
        engine.reset(scene.createRenderEngine(&context, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<cg::PMXRenderEngine *>(engine.data()));
    }
    {
        pmx::Model model(0); /* no IEncoding instance will be referred */
        QScopedPointer<IRenderEngine> engine(scene.createRenderEngine(&context, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::PMXRenderEngine *>(engine.data()));
        engine.reset(scene.createRenderEngine(&context, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<cg::PMXRenderEngine *>(engine.data()));
    }
    /* should not be crashed */
    ASSERT_EQ(static_cast<IRenderEngine *>(0), scene.createRenderEngine(0, 0, 0));
}

class SceneModelTest : public TestWithParam<IModel::Type> {};

TEST_P(SceneModelTest, SetParentSceneRef)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIRenderContext renderContext;
    EXPECT_CALL(renderContext, findProcedureAddress(_)).WillRepeatedly(Return(static_cast<void *>(0)));
    Scene scene(true);
    IModel::Type type = GetParam();
    QScopedPointer<IModel> modelPtr(factory.newModel(type));
    QScopedPointer<IRenderEngine> enginePtr(scene.createRenderEngine(&renderContext, modelPtr.data(), 0));
    scene.addModel(modelPtr.data(), enginePtr.take(), 0);
    /* IModel#parentSceneRef should not be null if the motion is added from the scene */
    ASSERT_EQ(&scene, modelPtr->parentSceneRef());
    scene.removeModel(modelPtr.data());
    /* IModel#parentSceneRef should be null if the motion is removed from the scene */
    ASSERT_EQ(static_cast<Scene *>(0), modelPtr->parentSceneRef());
    IModel *m = modelPtr.take();
    scene.deleteModel(m);
}

TEST_P(SceneModelTest, DeleteModelUnlessReferred)
{
    {
        /* should be freed and no memory leak warning */
        QSharedPointer<MockIModel> modelPtr(new MockIModel(), &Scene::deleteModelUnlessReferred);
        EXPECT_CALL(*modelPtr, parentSceneRef()).WillRepeatedly(Return(static_cast<Scene *>(0)));
        Q_UNUSED(modelPtr)
    }
    {
        /* should be freed and no memory leak warning */
        Scene scene(true);
        QSharedPointer<MockIModel> modelPtr(new MockIModel(), &Scene::deleteModelUnlessReferred);
        EXPECT_CALL(*modelPtr, name()).WillRepeatedly(Return(static_cast<IString *>(0)));
        EXPECT_CALL(*modelPtr, parentSceneRef()).WillRepeatedly(Return(&scene));
        EXPECT_CALL(*modelPtr, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*modelPtr, joinWorld(0)).Times(1);
        QScopedPointer<IRenderEngine> enginePtr(new MockIRenderEngine());
        scene.addModel(modelPtr.data(), enginePtr.take(), 0);
    }
}

class SceneRenderEngineTest : public TestWithParam< tuple<IModel::Type, int> > {};

TEST_P(SceneRenderEngineTest, DeleteRenderEngineUnlessReferred)
{
    Factory factory(0);
    Scene scene(false);
    MockIRenderContext renderContext;
    IModel::Type type = get<0>(GetParam());
    int flags = get<1>(GetParam());
    QSharedPointer<IModel> modelPtr(factory.newModel(type));
    QSharedPointer<IRenderEngine> enginePtr(scene.createRenderEngine(&renderContext, modelPtr.data(), flags),
                                            &Scene::deleteRenderEngineUnlessReferred);
    IRenderEngine *engine = enginePtr.data();
    scene.addModel(modelPtr.data(), engine, 0);
    enginePtr.clear();
    /* should not be crashed */
    ASSERT_EQ(modelPtr.data(), engine->parentModelRef());
    enginePtr = QSharedPointer<IRenderEngine>(engine);
    IModel *model = modelPtr.data();
    scene.deleteModel(model);
    /* IRenderEngine#parentModelRef should be null after calling Scene#deleteModel  */
    ASSERT_EQ(0, enginePtr->parentModelRef());
}

class SceneMotionTest : public TestWithParam<IMotion::Type> {};

TEST_P(SceneMotionTest, SetParentSceneRefForScene)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene(true);
    IMotion::Type type = GetParam();
    QScopedPointer<IMotion> cameraMotion(factory.newMotion(type, 0));
    scene.camera()->setMotion(cameraMotion.data());
    /* IMotion#parentSceneRef should not be null if ICamera#setMotion is called with motion */
    ASSERT_EQ(&scene, cameraMotion->parentSceneRef());
    scene.camera()->setMotion(0);
    /* IMotion#parentSceneRef should be null if ICamera#setMotion is called without motion */
    ASSERT_EQ(static_cast<Scene *>(0), cameraMotion->parentSceneRef());
    QScopedPointer<IMotion> lightMotion(factory.newMotion(type, 0));
    scene.light()->setMotion(lightMotion.data());
    /* IMotion#parentSceneRef should not be null if ILight#setMotion is called with motion */
    ASSERT_EQ(&scene, lightMotion->parentSceneRef());
    scene.light()->setMotion(0);
    /* IMotion#parentSceneRef should be null if ILight#setMotion is called without motion */
    ASSERT_EQ(static_cast<Scene *>(0), lightMotion->parentSceneRef());
}

TEST_P(SceneMotionTest, SetParentSceneRefForModel)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene(true);
    IMotion::Type type = GetParam();
    QScopedPointer<IMotion> motion(factory.newMotion(type, 0));
    scene.addMotion(motion.data());
    /* IMotion#parentSceneRef should not be null if the motion is added to the scene */
    ASSERT_EQ(&scene, motion->parentSceneRef());
    scene.removeMotion(motion.data());
    /* IMotion#parentSceneRef should be null if the motion is removed from the scene */
    ASSERT_EQ(static_cast<Scene *>(0), motion->parentSceneRef());
}

TEST_P(SceneMotionTest, DeleteMotionUnlessReferred)
{
    {
        // should be freed and no memory leak warning
        QSharedPointer<MockIMotion> motionPtr(new MockIMotion(), &Scene::deleteMotionUnlessReferred);
        EXPECT_CALL(*motionPtr, parentSceneRef()).WillRepeatedly(Return(static_cast<Scene *>(0)));
        Q_UNUSED(motionPtr)
    }
    {
        // should be freed and no memory leak warning
        Scene scene(true);
        QSharedPointer<MockIMotion> motionPtr(new MockIMotion(), &Scene::deleteMotionUnlessReferred);
        EXPECT_CALL(*motionPtr, parentSceneRef()).WillRepeatedly(Return(&scene));
        EXPECT_CALL(*motionPtr, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(motionPtr.data());
    }
}

class SceneModelMotionTest : public TestWithParam< tuple<IModel::Type, IMotion::Type> > {};

TEST_P(SceneModelMotionTest, CreateWithoutOwnMemory)
{
    IModel::Type modelType = get<0>(GetParam());
    IMotion::Type motionType = get<1>(GetParam());
    Factory factory(0);
    QScopedPointer<IModel> model(factory.newModel(modelType));
    QScopedPointer<IMotion> motion(factory.newMotion(motionType, model.data()));
    QScopedPointer<IRenderEngine> engine(new MockIRenderEngine());
    {
        Scene scene(false);
        scene.addModel(model.data(), engine.data(), 0);
        scene.addMotion(motion.data());
        /* releases all models/motions/renderEngines at dtor if ownMemory is true */
    }
    /* should not be crashed */
    ASSERT_EQ(motionType, motion->type());
    ASSERT_EQ(modelType, model->type());
    {
        Scene scene(false);
        scene.addModel(model.data(), engine.data(), 0);
        scene.addMotion(motion.data());
        IModel *m = model.data();
        scene.deleteModel(m);
        scene.removeMotion(motion.data());
    }
    /* should not be crashed */
    ASSERT_EQ(modelType, model->type());
    ASSERT_EQ(motionType, motion->type());
}

INSTANTIATE_TEST_CASE_P(SceneInstance, SceneModelTest, Values(IModel::kAssetModel, IModel::kPMDModel, IModel::kPMXModel));
INSTANTIATE_TEST_CASE_P(SceneInstance, SceneRenderEngineTest, Combine(Values(IModel::kAssetModel, IModel::kPMDModel, IModel::kPMXModel),
                                                                      Values(0, Scene::kEffectCapable)));
INSTANTIATE_TEST_CASE_P(SceneInstance, SceneMotionTest, Values(IMotion::kMVDMotion, IMotion::kVMDMotion));
INSTANTIATE_TEST_CASE_P(SceneInstance, SceneModelMotionTest, Combine(Values(IModel::kAssetModel, IModel::kPMDModel, IModel::kPMXModel),
                                                                     Values(IMotion::kMVDMotion, IMotion::kVMDMotion)));

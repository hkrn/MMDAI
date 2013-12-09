#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/IApplicationContext.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "mock/ApplicationContext.h"
#include "mock/Model.h"
#include "mock/Motion.h"
#include "mock/RenderEngine.h"

#include "vpvl2/asset/Model.h"
#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#else
#include "vpvl2/pmd2/Model.h"
#endif
#include "vpvl2/pmx/Model.h"
#include "vpvl2/fx/AssetRenderEngine.h"
#include "vpvl2/fx/PMXRenderEngine.h"
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/extensions/World.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

struct MockResolver : IApplicationContext::FunctionResolver {
    bool hasExtension(const char * /* name */) const { return false; }
    void *resolveSymbol(const char * /* name */) const { return 0; }
    int query(QueryType /* type */) const { return 0; }
} g_resolver;

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
    std::unique_ptr<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    /* adding a model but no rendering engine should not be added */
    scene.addModel(model.get(), 0, 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
    /* no rendering context class will be referered */
    std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
    /* adding a rendering engine but no model should not be added */
    scene.addModel(0, engine.get(), 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
    scene.addModel(model.release(), engine.release(), 0);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    /* both model and rendering engine should be added */
    ASSERT_EQ(1, models.count());
    ASSERT_EQ(1, engines.count());
}

TEST(SceneTest, AddMotion)
{
    Array<IMotion *> motions;
    Encoding encoding(0);
    Factory factory(&encoding);
    Scene scene(true);
    /* adding an null motion should not be crashed */
    scene.addMotion(0);
    scene.getMotionRefs(motions);
    ASSERT_EQ(0, motions.count());
    std::unique_ptr<IMotion> motion(factory.newMotion(IMotion::kVMDMotion, 0));
    scene.addMotion(motion.release());
    scene.getMotionRefs(motions);
    ASSERT_EQ(1, motions.count());
}

TEST(SceneTest, FindModel)
{
    Scene scene(true);
    /* adding an null motion should not be crashed */
    scene.findModel(0);
    std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
    std::unique_ptr<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("foo_bar_baz")), s2(s.value());
    EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillOnce(Return(&s));
    scene.addModel(model.get(), engine.release(), 0);
    ASSERT_EQ(model.release(), scene.findModel(&s2));
}

TEST(SceneTest, FindRenderEngine)
{
    Scene scene(true);
    std::unique_ptr<MockIModel> model(new MockIModel());
    /* ignore setting setParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
    std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
    scene.addModel(model.get(), engine.get(), 0);
    ASSERT_EQ(engine.release(), scene.findRenderEngine(model.release()));
}

TEST(SceneTest, RemoveModel)
{
    Array<IModel *> models;
    Array<IRenderEngine *> engines;
    Scene scene(true);
    std::unique_ptr<MockIModel> model(new MockIModel());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    EXPECT_CALL(*model, leaveWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
    std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
    /* removing an null model should do nothing */
    scene.removeModel(0);
    scene.addModel(model.get(), engine.release(), 0);
    /* model should be deleted and set it null */
    scene.removeModel(model.get());
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
    std::unique_ptr<MockIModel> model(new MockIModel());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
    EXPECT_CALL(*model, joinWorld(0)).Times(1);
    EXPECT_CALL(*model, leaveWorld(0)).Times(1);
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
    std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
    IModel *fakePtr = 0;
    /* deleting an null model should not be crashed */
    scene.deleteModel(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addModel(model.get(), engine.release(), 0);
    IModel *modelPtr = model.get();
    /* model should be deleted and set it null */
    scene.deleteModel(modelPtr);
    scene.getModelRefs(models);
    scene.getRenderEngineRefs(engines);
    model.release();
    ASSERT_EQ(0, modelPtr);
    ASSERT_EQ(0, models.count());
    ASSERT_EQ(0, engines.count());
}

TEST(SceneTest, RemoveMotion)
{
    Array<IMotion *> motions;
    Scene scene(true);
    std::unique_ptr<MockIMotion> motion(new MockIMotion());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
    /* removing an null model should do nothing */
    scene.removeMotion(0);
    scene.addMotion(motion.get());
    /* model should be deleted and set it null */
    scene.removeMotion(motion.get());
    scene.getMotionRefs(motions);
    ASSERT_EQ(0, motions.count());
}

TEST(SceneTest, DeleteMotion)
{
    Array<IMotion *> motions;
    Scene scene(true);
    std::unique_ptr<MockIMotion> motion(new MockIMotion());
    /* ignore setting VPVL2SceneSetParentSceneRef */
    EXPECT_CALL(*motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
    IMotion *fakePtr = 0;
    /* deleting an null motion should not be crashed */
    scene.deleteMotion(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addMotion(motion.get());
    IMotion *motionPtr = motion.get();
    /* model should be deleted and set it null */
    scene.deleteMotion(motionPtr);
    scene.getMotionRefs(motions);
    motion.release();
    ASSERT_EQ(0, motionPtr);
    ASSERT_EQ(0, motions.count());
}

TEST(SceneTest, Update)
{
    {
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.release(), engine.release(), 0);
        scene.update(Scene::kUpdateRenderEngines);
    }
    {
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, performUpdate()).Times(1);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.release(), engine.release(), 0);
        scene.update(Scene::kUpdateAll);
    }
    {
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).Times(0);
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*model, performUpdate()).Times(1);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.addModel(model.release(), engine.release(), 0);
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
    std::unique_ptr<ICamera> camera1(scene.createCamera()), camera2(scene.createCamera());
    ASSERT_NE(camera2.get(), camera1.get());
    ASSERT_EQ(scene.cameraRef(), scene.cameraRef());
}

TEST(SceneTest, AdvanceSceneCamera)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateCamera is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateCamera);
        scene.cameraRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateAll is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.cameraRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#advanceScene should not be called if Scene#advance with kUpdate(Light|Models|RenderEngines) is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.cameraRef()->setMotion(0);
    }
    scene.cameraRef()->setMotion(0);
}

TEST(SceneTest, SeekSceneCamera)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateCamera is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateCamera);
        scene.cameraRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateAll is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.cameraRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.cameraRef()->setMotion(&motion);
        /* IMotion#seekScene should not be called if Scene#seek with kUpdate(Light|Models|RenderEngines) is called */
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.cameraRef()->setMotion(0);
    }
    scene.cameraRef()->setMotion(0);
}

TEST(SceneTest, Light)
{
    Scene scene(true);
    std::unique_ptr<ILight> light1(scene.createLight()), light2(scene.createLight());
    ASSERT_NE(light2.get(), light1.get());
    ASSERT_EQ(scene.lightRef(), scene.lightRef());
}

TEST(SceneTest, AdvanceSceneLight)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateLight is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateLight);
        scene.lightRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#advanceScene should be called once if Scene#advance with kUpdateAll is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.lightRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#advanceScene should not be called if Scene#advance with kUpdate(Camera|Models|RenderEngines) is called */
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.lightRef()->setMotion(0);
    }
    scene.lightRef()->setMotion(0);
}

TEST(SceneTest, SeekSceneLight)
{
    Scene scene(true);
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateLight is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateLight);
        scene.lightRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#seekScene should be called once if Scene#seek with kUpdateAll is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.lightRef()->setMotion(0);
    }
    {
        MockIMotion motion;
        /* ignore setting setParentSceneRef */
        EXPECT_CALL(motion, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.lightRef()->setMotion(&motion);
        /* IMotion#seekScene should not be called if Scene#seek with kUpdate(Camera|Models|RenderEngines) is called */
        EXPECT_CALL(motion, seekScene(0, &scene)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.lightRef()->setMotion(0);
    }
    scene.lightRef()->setMotion(0);
}

TEST(SceneTest, SetWorldRef)
{
    extensions::World world;
    btDiscreteDynamicsWorld *worldRef = world.dynamicWorldRef();
    {
        // 1. call setWorldRef first and addModel without removing model
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        scene.setWorldRef(worldRef);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.addModel(model.get(), engine.release(), 0);
        model.release();
    }
    {
        // 2. add model first and call setWorldRef without removing model
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        scene.addModel(model.get(), engine.release(), 0);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.setWorldRef(worldRef);
        model.release();
    }
    {
        // 3. deleting (removing) model explicitly (result should be same as 2)
        std::unique_ptr<MockIRenderEngine> engine(new MockIRenderEngine());
        std::unique_ptr<MockIModel> model(new MockIModel());
        EXPECT_CALL(*model, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(&s));
        Scene scene(true);
        EXPECT_CALL(*model, joinWorld(0)).Times(1);
        scene.addModel(model.get(), engine.release(), 0);
        EXPECT_CALL(*model, joinWorld(worldRef)).Times(1);
        EXPECT_CALL(*model, leaveWorld(worldRef)).Times(1);
        scene.setWorldRef(worldRef);
        IModel *m = model.release();
        scene.deleteModel(m);
    }
}

TEST(SceneTest, CreateRenderEngine)
{
    Scene scene(true);
    Encoding encoding(0);
    MockIApplicationContext applicationContext;
    EXPECT_CALL(applicationContext, sharedFunctionResolverInstance()).Times(AnyNumber()).WillRepeatedly(Return(&g_resolver));
    {
        asset::Model model(&encoding);
        std::unique_ptr<IRenderEngine> engine(scene.createRenderEngine(&applicationContext, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::AssetRenderEngine *>(engine.get()));
        engine.reset(scene.createRenderEngine(&applicationContext, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<fx::AssetRenderEngine *>(engine.get()));
    }
    {
#ifdef VPVL2_LINK_VPVL
        pmd::Model model(&encoding);
#else
        pmd2::Model model(&encoding);
#endif
        std::unique_ptr<IRenderEngine> engine(scene.createRenderEngine(&applicationContext, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::PMXRenderEngine *>(engine.get()));
        engine.reset(scene.createRenderEngine(&applicationContext, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<fx::PMXRenderEngine *>(engine.get()));
    }
    {
        pmx::Model model(&encoding);
        std::unique_ptr<IRenderEngine> engine(scene.createRenderEngine(&applicationContext, &model, 0));
        ASSERT_TRUE(dynamic_cast<gl2::PMXRenderEngine *>(engine.get()));
        engine.reset(scene.createRenderEngine(&applicationContext, &model, Scene::kEffectCapable));
        ASSERT_TRUE(dynamic_cast<fx::PMXRenderEngine *>(engine.get()));
    }
    /* should not be crashed */
    ASSERT_EQ(static_cast<IRenderEngine *>(0), scene.createRenderEngine(&applicationContext, 0, 0));
}

TEST(SceneModel, HandleDefaultCamera)
{
    Scene scene(true);
    Factory factory(0);
    std::unique_ptr<IMotion> motion(factory.newMotion(IMotion::kVMDMotion, 0));
    // set camera parameters
    ICamera *camera = scene.cameraRef();
    camera->setAngle(Vector3(1, 2, 3));
    camera->setDistance(4);
    camera->setFov(5);
    camera->setLookAt(Vector3(6, 7, 8));
    camera->setMotion(motion.get());
    // will be same as set
    CompareVector(Vector3(1, 2, 3), camera->angle());
    ASSERT_FLOAT_EQ(4, camera->distance());
    ASSERT_FLOAT_EQ(5, camera->fov());
    CompareVector(Vector3(6, 7, 8), camera->lookAt());
    ASSERT_EQ(motion.get(), camera->motion());
    // create camera object with default parameters and copy
    std::unique_ptr<ICamera> camera2(scene.createCamera());
    camera2->copyFrom(camera);
    // reset camera object and will be same as default parameters except motion
    camera->resetDefault();
    CompareVector(kZeroV3, camera->angle());
    ASSERT_FLOAT_EQ(50, camera->distance());
    ASSERT_FLOAT_EQ(27, camera->fov());
    CompareVector(Vector3(0, 10, 0), camera->lookAt());
    ASSERT_EQ(motion.get(), camera->motion());
    // copied camera object will be same as set except motion
    CompareVector(Vector3(1, 2, 3), camera2->angle());
    ASSERT_FLOAT_EQ(4, camera2->distance());
    ASSERT_FLOAT_EQ(5, camera2->fov());
    CompareVector(Vector3(6, 7, 8), camera2->lookAt());
    ASSERT_EQ(0, camera2->motion());
    // release reference of stack allocated camera motion
    camera->setMotion(0);
}

TEST(SceneModel, HandleDefaultLight)
{
    Scene scene(true);
    Factory factory(0);
    std::unique_ptr<IMotion> motion(factory.newMotion(IMotion::kVMDMotion, 0));
    // set light parameters
    ILight *light = scene.lightRef();
    light->setColor(Vector3(0.1f, 0.2f, 0.3f));
    light->setDirection(Vector3(0.4f, 0.5f, 0.6f));
    light->setToonEnable(true);
    light->setMotion(motion.get());
    // will be same as set
    CompareVector(Vector3(0.1f, 0.2f, 0.3f), light->color());
    CompareVector(Vector3(0.4f, 0.5f, 0.6f), light->direction());
    ASSERT_TRUE(light->isToonEnabled());
    ASSERT_EQ(motion.get(), light->motion());
    // create light object with default parameters and copy
    std::unique_ptr<ILight> light2(scene.createLight());
    light2->copyFrom(light);
    // reset camera object and will be same as default parameters except motion
    light->resetDefault();
    CompareVector(Vector3(0.6f, 0.6f, 0.6f), light->color());
    CompareVector(Vector3(-0.5f, -1.0f, -0.5f), light->direction());
    ASSERT_FALSE(light->isToonEnabled());
    ASSERT_EQ(motion.get(), light->motion());
    // copied light object will be same as set except motion
    CompareVector(Vector3(0.1f, 0.2f, 0.3f), light2->color());
    CompareVector(Vector3(0.4f, 0.5f, 0.6f), light2->direction());
    ASSERT_TRUE(light2->isToonEnabled());
    ASSERT_EQ(0, light2->motion());
    // release reference of stack allocated light motion
    light->setMotion(0);
}

class SceneModelTest : public TestWithParam<IModel::Type> {};

TEST_P(SceneModelTest, SetParentSceneRef)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    MockIApplicationContext applicationContext;
    EXPECT_CALL(applicationContext, sharedFunctionResolverInstance()).Times(AnyNumber()).WillRepeatedly(Return(&g_resolver));
    Scene scene(true);
    IModel::Type type = GetParam();
    std::unique_ptr<IModel> modelPtr(factory.newModel(type));
    std::unique_ptr<IRenderEngine> enginePtr(scene.createRenderEngine(&applicationContext, modelPtr.get(), 0));
    scene.addModel(modelPtr.get(), enginePtr.release(), 0);
    /* IModel#parentSceneRef should not be null if the motion is added from the scene */
    ASSERT_EQ(&scene, modelPtr->parentSceneRef());
    scene.removeModel(modelPtr.get());
    /* IModel#parentSceneRef should be null if the motion is removed from the scene */
    ASSERT_EQ(static_cast<Scene *>(0), modelPtr->parentSceneRef());
    IModel *m = modelPtr.release();
    scene.deleteModel(m);
}

TEST_P(SceneModelTest, DeleteModelUnlessReferred)
{
    {
        /* should be freed and no memory leak warning */
        QSharedPointer<MockIModel> modelPtr(new MockIModel(), &Scene::deleteModelUnlessReferred);
        EXPECT_CALL(*modelPtr, parentSceneRef()).WillRepeatedly(Return(static_cast<Scene *>(0)));
        Q_UNUSED(modelPtr);
    }
    {
        /* should be freed and no memory leak warning */
        Scene scene(true);
        std::shared_ptr<MockIModel> modelPtr(new MockIModel(), &Scene::deleteModelUnlessReferred);
        EXPECT_CALL(*modelPtr, name(IEncoding::kDefaultLanguage)).WillRepeatedly(Return(static_cast<IString *>(0)));
        EXPECT_CALL(*modelPtr, parentSceneRef()).WillRepeatedly(Return(&scene));
        EXPECT_CALL(*modelPtr, type()).WillRepeatedly(Return(IModel::kMaxModelType));
        EXPECT_CALL(*modelPtr, joinWorld(0)).Times(1);
        std::unique_ptr<IRenderEngine> enginePtr(new MockIRenderEngine());
        scene.addModel(modelPtr.get(), enginePtr.release(), 0);
    }
}

class SceneRenderEngineTest : public TestWithParam< tuple<IModel::Type, int> > {};

TEST_P(SceneRenderEngineTest, DeleteRenderEngineUnlessReferred)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    Scene scene(false);
    MockIApplicationContext applicationContext;
    EXPECT_CALL(applicationContext, sharedFunctionResolverInstance()).Times(AnyNumber()).WillRepeatedly(Return(&g_resolver));
    IModel::Type type = get<0>(GetParam());
    int flags = get<1>(GetParam());
    std::unique_ptr<IModel> modelPtr(factory.newModel(type));
    std::shared_ptr<IRenderEngine> enginePtr(scene.createRenderEngine(&applicationContext, modelPtr.get(), flags),
                                            &Scene::deleteRenderEngineUnlessReferred);
    IRenderEngine *engine = enginePtr.get();
    scene.addModel(modelPtr.get(), engine, 0);
    enginePtr.reset();
    /* should not be crashed */
    ASSERT_EQ(modelPtr.get(), engine->parentModelRef());
    enginePtr = std::shared_ptr<IRenderEngine>(engine);
    IModel *model = modelPtr.get();
    scene.deleteModel(model);
    /* IRenderEngine#parentModelRef should be null after calling Scene#deleteModel  */
    ASSERT_EQ(0, enginePtr->parentModelRef());
}

class SceneMotionTest : public TestWithParam<IMotion::Type> {};

TEST_P(SceneMotionTest, SetParentSceneRefForScene)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    Scene scene(true);
    IMotion::Type type = GetParam();
    std::unique_ptr<IMotion> cameraMotion(factory.newMotion(type, 0));
    scene.cameraRef()->setMotion(cameraMotion.get());
    /* IMotion#parentSceneRef should not be null if ICamera#setMotion is called with motion */
    ASSERT_EQ(&scene, cameraMotion->parentSceneRef());
    scene.cameraRef()->setMotion(0);
    /* IMotion#parentSceneRef should be null if ICamera#setMotion is called without motion */
    ASSERT_EQ(static_cast<Scene *>(0), cameraMotion->parentSceneRef());
    std::unique_ptr<IMotion> lightMotion(factory.newMotion(type, 0));
    scene.lightRef()->setMotion(lightMotion.get());
    /* IMotion#parentSceneRef should not be null if ILight#setMotion is called with motion */
    ASSERT_EQ(&scene, lightMotion->parentSceneRef());
    scene.lightRef()->setMotion(0);
    /* IMotion#parentSceneRef should be null if ILight#setMotion is called without motion */
    ASSERT_EQ(static_cast<Scene *>(0), lightMotion->parentSceneRef());
}

TEST_P(SceneMotionTest, SetParentSceneRefForModel)
{
    Encoding encoding(0);
    Factory factory(&encoding);
    Scene scene(true);
    IMotion::Type type = GetParam();
    std::unique_ptr<IMotion> motion(factory.newMotion(type, 0));
    scene.addMotion(motion.get());
    /* IMotion#parentSceneRef should not be null if the motion is added to the scene */
    ASSERT_EQ(&scene, motion->parentSceneRef());
    scene.removeMotion(motion.get());
    /* IMotion#parentSceneRef should be null if the motion is removed from the scene */
    ASSERT_EQ(static_cast<Scene *>(0), motion->parentSceneRef());
}

TEST_P(SceneMotionTest, DeleteMotionUnlessReferred)
{
    {
        // should be freed and no memory leak warning
        std::shared_ptr<MockIMotion> motionPtr(new MockIMotion(), &Scene::deleteMotionUnlessReferred);
        EXPECT_CALL(*motionPtr, parentSceneRef()).WillRepeatedly(Return(static_cast<Scene *>(0)));
        Q_UNUSED(motionPtr);
    }
    {
        // should be freed and no memory leak warning
        Scene scene(true);
        std::shared_ptr<MockIMotion> motionPtr(new MockIMotion(), &Scene::deleteMotionUnlessReferred);
        EXPECT_CALL(*motionPtr, parentSceneRef()).WillRepeatedly(Return(&scene));
        EXPECT_CALL(*motionPtr, type()).WillRepeatedly(Return(IMotion::kMaxMotionType));
        scene.addMotion(motionPtr.get());
    }
}

class SceneModelMotionTest : public TestWithParam< tuple<IModel::Type, IMotion::Type> > {};

TEST_P(SceneModelMotionTest, CreateWithoutOwnMemory)
{
    IModel::Type modelType = get<0>(GetParam());
    IMotion::Type motionType = get<1>(GetParam());
    Encoding encoding(0);
    Factory factory(&encoding);
    std::unique_ptr<IModel> model(factory.newModel(modelType));
    std::unique_ptr<IMotion> motion(factory.newMotion(motionType, model.get()));
    std::unique_ptr<IRenderEngine> engine(new MockIRenderEngine());
    {
        Scene scene(false);
        scene.addModel(model.get(), engine.get(), 0);
        scene.addMotion(motion.get());
        /* releases all models/motions/renderEngines at dtor if ownMemory is true */
    }
    /* should not be crashed */
    ASSERT_EQ(motionType, motion->type());
    ASSERT_EQ(modelType, model->type());
    {
        Scene scene(false);
        scene.addModel(model.get(), engine.get(), 0);
        scene.addMotion(motion.get());
        IModel *m = model.get();
        scene.deleteModel(m);
        scene.removeMotion(motion.get());
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

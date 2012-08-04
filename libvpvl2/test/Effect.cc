#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tr1/tuple>
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/cg/EffectEngine.h>
#include <vpvl2/qt/CString.h>
#include "mock/Model.h"
#include "mock/RenderDelegate.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::cg;
using namespace vpvl2::qt;

class EffectTest : public ::testing::Test {
public:
    void setMatrix(MockIRenderDelegate &delegate, const IModel *modelPtr, int flags) {
        int cw = IRenderDelegate::kWorldMatrix | flags;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cw)).Times(1);
        int cv = IRenderDelegate::kViewMatrix | flags;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cv)).Times(1);
        int cp = IRenderDelegate::kProjectionMatrix | flags;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cp)).Times(1);
        int cwv = cw | cv;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cwv)).Times(1);
        int cvp = cv | cp;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cvp)).Times(1);
        int cwvp = cw | cv | cp;
        EXPECT_CALL(delegate, getMatrix(_, modelPtr, cwvp)).Times(1);
    }
    cg::Effect *createEffect(const QString &effectPath, Scene &scene, MockIRenderDelegate &delegate, CGeffect &ptr) {
        QFile file(effectPath);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            CString *source = new CString(bytes);
            EXPECT_CALL(delegate, getViewport(_)).Times(AnyNumber()).WillRepeatedly(Return());
            EXPECT_CALL(delegate, loadShaderSource(IRenderDelegate::kModelEffectTechniques, _))
                    .Times(1).WillRepeatedly(Return(source));
            cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(0, &delegate));
            ptr = static_cast<CGeffect>(effect->internalPointer());
            return effect;
        }
        return 0;
    }
};

TEST_F(EffectTest, ToBool)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    createEffect(":effects/util.cgfx", scene, delegate, effectPtr);
    CGparameter parameter = cgGetNamedEffectParameter(effectPtr, "ValueTest");
    ASSERT_TRUE(Util::toBool(cgGetNamedParameterAnnotation(parameter, "BooleanTrueValue")));
    ASSERT_FALSE(Util::toBool(cgGetNamedParameterAnnotation(parameter, "BooleanFalseValue")));
    ASSERT_FALSE(Util::toBool(cgGetNamedParameterAnnotation(parameter, "IntegerValue")));
    ASSERT_FALSE(Util::toBool(cgGetNamedParameterAnnotation(parameter, "FloatValue")));
    ASSERT_FALSE(Util::toBool(cgGetNamedParameterAnnotation(parameter, "StringValue")));
}

TEST_F(EffectTest, ToFloat)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    createEffect(":effects/util.cgfx", scene, delegate, effectPtr);
    CGparameter parameter = cgGetNamedEffectParameter(effectPtr, "ValueTest");
    ASSERT_FLOAT_EQ(0.0, Util::toFloat(cgGetNamedParameterAnnotation(parameter, "BooleanTrueValue")));
    ASSERT_FLOAT_EQ(0.0, Util::toFloat(cgGetNamedParameterAnnotation(parameter, "BooleanFalseValue")));
    ASSERT_FLOAT_EQ(0.0, Util::toFloat(cgGetNamedParameterAnnotation(parameter, "IntegerValue")));
    ASSERT_FLOAT_EQ(42.0, Util::toFloat(cgGetNamedParameterAnnotation(parameter, "FloatValue")));
    ASSERT_FLOAT_EQ(0.0, Util::toFloat(cgGetNamedParameterAnnotation(parameter, "StringValue")));
}

TEST_F(EffectTest, ToInt)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    createEffect(":effects/util.cgfx", scene, delegate, effectPtr);
    CGparameter parameter = cgGetNamedEffectParameter(effectPtr, "ValueTest");
    ASSERT_EQ(0, Util::toInt(cgGetNamedParameterAnnotation(parameter, "BooleanTrueValue")));
    ASSERT_EQ(0, Util::toInt(cgGetNamedParameterAnnotation(parameter, "BooleanFalseValue")));
    ASSERT_EQ(42, Util::toInt(cgGetNamedParameterAnnotation(parameter, "IntegerValue")));
    ASSERT_EQ(0, Util::toInt(cgGetNamedParameterAnnotation(parameter, "FloatValue")));
    ASSERT_EQ(0, Util::toInt(cgGetNamedParameterAnnotation(parameter, "StringValue")));
}

TEST_F(EffectTest, IsPassEquals)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    createEffect(":effects/util.cgfx", scene, delegate, effectPtr);
    CGparameter parameter = cgGetNamedEffectParameter(effectPtr, "ValueTest");
    const char target[] = "This is string.";
    ASSERT_TRUE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "NoSuchAnnotation"), target));
    ASSERT_FALSE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "BooleanTrueValue"), target));
    ASSERT_FALSE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "BooleanFalseValue"), target));
    ASSERT_FALSE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "IntegerValue"), target));
    ASSERT_FALSE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "FloatValue"), target));
    ASSERT_TRUE(Util::isPassEquals(cgGetNamedParameterAnnotation(parameter, "StringValue"), target));
}

TEST_F(EffectTest, IsIntegerParameter)
{
    struct Expect {
        const char *name;
        bool expected;
    };
    Expect expects[] = {
        { "BooleanValue",   true },
        { "FloatValue",     true },
        { "IntegerValue",   true },
        { "Float2Value",    false },
        { "Float3Value",    false },
        { "Float4Value",    false },
        { "Float3x3Value",  false },
        { "Float3x4Value",  false },
        { "Float4x4Value",  false },
        { "SamplerValue",   false },
        { "Sampler2DValue", false },
        { "TextureValue",   false },
        { "Texture2DValue", false }
    };
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    createEffect(":effects/util.cgfx", scene, delegate, effectPtr);
    const int nexpects = sizeof(expects) / sizeof(expects[0]);
    for (int i = 0; i < nexpects; i++) {
        Expect &e = expects[i];
        CGparameter parameter = cgGetNamedEffectParameter(effectPtr, e.name);
        EXPECT_TRUE(parameter);
        EXPECT_EQ(e.expected, Util::isIntegerParameter(parameter));
    }
}

TEST_F(EffectTest, LoadMatrices)
{
    MockIRenderDelegate delegate;
    MockIModel model, *modelPtr = &model;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/matrices.cgfx", scene, delegate, effectPtr);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kInverseMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
    EffectEngine engine(&scene, 0, effect, &delegate);
    engine.setModelMatrixParameters(modelPtr, 0, 0);
}

TEST_F(EffectTest, LoadMaterialColors)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/materials.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    Vector4 v;
    float f;
    cgGLGetParameter4f(engine.ambient.geometryParameter(), v);
    ASSERT_EQ(Vector4(0.01, 0.02, 0.03, 0.04), v);
    cgGLGetParameter4f(engine.diffuse.geometryParameter(), v);
    ASSERT_EQ(Vector4(0.05, 0.06, 0.07, 0.08), v);
    cgGLGetParameter4f(engine.emissive.geometryParameter(), v);
    ASSERT_EQ(Vector4(0.09, 0.10, 0.11, 0.12), v);
    cgGLGetParameter4f(engine.specular.geometryParameter(), v);
    ASSERT_EQ(Vector4(0.13, 0.14, 0.15, 0.16), v);
    cgGLGetParameter1f(engine.specularPower.geometryParameter(), &f);
    ASSERT_FLOAT_EQ(0.17, f);
    cgGLGetParameter4f(engine.toonColor.geometryParameter(), v);
    ASSERT_EQ(Vector4(0.18, 0.19, 0.20, 0.21), v);
    cgGLGetParameter4f(engine.ambient.lightParameter(), v);
    ASSERT_EQ(Vector4(0.22, 0.23, 0.24, 0.25), v);
    cgGLGetParameter4f(engine.diffuse.lightParameter(), v);
    ASSERT_EQ(Vector4(0.26, 0.27, 0.28, 0.29), v);
    cgGLGetParameter4f(engine.specular.lightParameter(), v);
    ASSERT_EQ(Vector4(0.30, 0.31, 0.32, 0.33), v);
}

TEST_F(EffectTest, LoadGeometries)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/geometries.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    Vector4 v;
    cgGLGetParameter4f(engine.direction.cameraParameter(), v);
    ASSERT_EQ(Vector4(0.01, 0.02, 0.03, 0.04), v);
    cgGLGetParameter4f(engine.position.cameraParameter(), v);
    ASSERT_EQ(Vector4(0.05, 0.06, 0.07, 0.08), v);
    cgGLGetParameter4f(engine.direction.lightParameter(), v);
    ASSERT_EQ(Vector4(0.09, 0.10, 0.11, 0.12), v);
    cgGLGetParameter4f(engine.position.lightParameter(), v);
    ASSERT_EQ(Vector4(0.13, 0.14, 0.15, 0.16), v);
}

TEST_F(EffectTest, LoadTimes)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/times.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    float f;
    cgGLGetParameter1f(engine.time.syncDisabledParameter(), &f);
    ASSERT_FLOAT_EQ(0.1, f);
    cgGLGetParameter1f(engine.elapsedTime.syncDisabledParameter(), &f);
    ASSERT_FLOAT_EQ(0.2, f);
    cgGLGetParameter1f(engine.time.syncEnabledParameter(), &f);
    ASSERT_FLOAT_EQ(0.3, f);
    cgGLGetParameter1f(engine.elapsedTime.syncEnabledParameter(), &f);
    ASSERT_FLOAT_EQ(0.4, f);
}

TEST_F(EffectTest, LoadSpecials)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/specials.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    float f;
    cgGLGetParameter1f(engine.parthf.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.spadd.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.transp.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.useTexture.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.useSpheremap.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.useToon.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.opadd.baseParameter(), &f);
    ASSERT_FLOAT_EQ(1.0, f);
    cgGLGetParameter1f(engine.vertexCount.baseParameter(), &f);
    ASSERT_FLOAT_EQ(2.0, f);
    cgGLGetParameter1f(engine.subsetCount.baseParameter(), &f);
    ASSERT_FLOAT_EQ(3.0, f);
}

TEST_F(EffectTest, LoadSASPreProcess)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/preprocess.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_EQ(EffectEngine::kScene, engine.scriptClass());
    ASSERT_EQ(IEffect::kPreProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.size());
    ASSERT_STREQ("test1", cgGetTechniqueName(techniques[0]));
    ASSERT_STREQ("test2", cgGetTechniqueName(techniques[1]));
    ASSERT_STREQ("test3", cgGetTechniqueName(techniques[2]));
    const EffectEngine::Script *nullScript = engine.findPassScript(cgGetNamedPass(techniques[0], "null"));
    ASSERT_EQ(1, nullScript->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawBuffer, nullScript->at(0).type);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(cgGetNamedPass(techniques[1], "null"));
    ASSERT_FALSE(nullScript2);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(cgGetNamedPass(techniques[2], "null"));
    ASSERT_FALSE(nullScript3);
}

TEST_F(EffectTest, LoadSASStandard)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/standard.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_EQ(EffectEngine::kObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kStandard, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.size());
    ASSERT_STREQ("test1", cgGetTechniqueName(techniques[0]));
    ASSERT_STREQ("test2", cgGetTechniqueName(techniques[1]));
    ASSERT_STREQ("test3", cgGetTechniqueName(techniques[2]));
    const EffectEngine::Script *nullScript = engine.findPassScript(cgGetNamedPass(techniques[0], "null"));
    ASSERT_FALSE(nullScript);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(cgGetNamedPass(techniques[1], "null"));
    ASSERT_EQ(1, nullScript2->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript2->at(0).type);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(cgGetNamedPass(techniques[2], "null"));
    ASSERT_EQ(1, nullScript3->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript3->at(0).type);
}

TEST_F(EffectTest, LoadSASStandard2)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/standard2.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_EQ(EffectEngine::kObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kStandard, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(1, techniques.size());
    ASSERT_STREQ("test1", cgGetTechniqueName(techniques[0]));
}

TEST_F(EffectTest, LoadSASPostProcess)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/postprocess.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_EQ(EffectEngine::kSceneOrObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kPostProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.size());
    ASSERT_STREQ("test1", cgGetTechniqueName(techniques[0]));
    ASSERT_STREQ("test2", cgGetTechniqueName(techniques[1]));
    ASSERT_STREQ("test3", cgGetTechniqueName(techniques[2]));
    const EffectEngine::Script *nullScript = engine.findPassScript(cgGetNamedPass(techniques[0], "null"));
    ASSERT_EQ(1, nullScript->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawBuffer, nullScript->at(0).type);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(cgGetNamedPass(techniques[1], "null"));
    ASSERT_EQ(1, nullScript2->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript2->at(0).type);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(cgGetNamedPass(techniques[2], "null"));
    ASSERT_EQ(1, nullScript3->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript3->at(0).type);
}

TEST_F(EffectTest, FindTechniques)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/techniques.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_STREQ("MainTec7",   cgGetTechniqueName(engine.findTechnique("object",     1, 42, true,  true,  true)));
    ASSERT_STREQ("MainTec6",   cgGetTechniqueName(engine.findTechnique("object",     2, 42, false, true,  true)));
    ASSERT_STREQ("MainTec5",   cgGetTechniqueName(engine.findTechnique("object",     3, 42, true,  false, true)));
    ASSERT_STREQ("MainTec4",   cgGetTechniqueName(engine.findTechnique("object",     4, 42, false, false, true)));
    ASSERT_STREQ("MainTec3",   cgGetTechniqueName(engine.findTechnique("object",     5, 42, true,  true,  false)));
    ASSERT_STREQ("MainTec2",   cgGetTechniqueName(engine.findTechnique("object",     6, 42, false, true,  false)));
    ASSERT_STREQ("MainTec1",   cgGetTechniqueName(engine.findTechnique("object",     7, 42, true,  false, false)));
    ASSERT_STREQ("MainTec0",   cgGetTechniqueName(engine.findTechnique("object",     8, 42, false, false, false)));
    ASSERT_STREQ("MainTecBS7", cgGetTechniqueName(engine.findTechnique("object_ss",  9, 42, true,  true,  true)));
    ASSERT_STREQ("MainTecBS6", cgGetTechniqueName(engine.findTechnique("object_ss", 10, 42, false, true,  true)));
    ASSERT_STREQ("MainTecBS5", cgGetTechniqueName(engine.findTechnique("object_ss", 11, 42, true,  false, true)));
    ASSERT_STREQ("MainTecBS4", cgGetTechniqueName(engine.findTechnique("object_ss", 12, 42, false, false, true)));
    ASSERT_STREQ("MainTecBS3", cgGetTechniqueName(engine.findTechnique("object_ss", 13, 42, true,  true,  false)));
    ASSERT_STREQ("MainTecBS2", cgGetTechniqueName(engine.findTechnique("object_ss", 14, 42, false, true,  false)));
    ASSERT_STREQ("MainTecBS1", cgGetTechniqueName(engine.findTechnique("object_ss", 15, 42, true,  false, false)));
    ASSERT_STREQ("MainTecBS0", cgGetTechniqueName(engine.findTechnique("object_ss", 16, 42, false, false, false)));
}

class FindTechnique : public EffectTest, public WithParamInterface< tuple<int, int, bool, bool, bool> > {};

TEST_P(FindTechnique, TestEdge)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/techniques.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_STREQ("EdgeTec", cgGetTechniqueName(engine.findTechnique("edge",
                                                                    get<0>(GetParam()),
                                                                    get<1>(GetParam()),
                                                                    get<2>(GetParam()),
                                                                    get<3>(GetParam()),
                                                                    get<4>(GetParam()))));
}

TEST_P(FindTechnique, TestShadow)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/techniques.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    ASSERT_STREQ("ShadowTec", cgGetTechniqueName(engine.findTechnique("shadow",
                                                                      get<0>(GetParam()),
                                                                      get<1>(GetParam()),
                                                                      get<2>(GetParam()),
                                                                      get<3>(GetParam()),
                                                                      get<4>(GetParam()))));
}

INSTANTIATE_TEST_CASE_P(EffectValueTest, FindTechnique,
                        Combine(Range(-1, 1), Values(2), Bool(), Bool(), Bool()));

TEST_F(EffectTest, ParseSyntaxErrorsScript)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/scripts.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    CGtechnique technique = cgGetNamedTechnique(effectPtr, "SyntaxErrors");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(1, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(0).type);
}

TEST_F(EffectTest, ParseRenderTargetsScript)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/scripts.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    CGtechnique technique = cgGetNamedTechnique(effectPtr, "RenderTargets");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(15, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget0, script->at(0).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT0"), script->at(0).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget1, script->at(1).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT1"), script->at(1).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget2, script->at(2).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT2"), script->at(2).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget3, script->at(3).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT3"), script->at(3).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderDepthStencilTarget, script->at(4).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT4"), script->at(4).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kClearSetColor, script->at(5).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "ClearColor"), script->at(5).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kClearSetDepth, script->at(6).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "ClearDepth"), script->at(6).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kClearColor, script->at(7).type);
    ASSERT_EQ(EffectEngine::ScriptState::kClearDepth, script->at(8).type);
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(9).type);
    ASSERT_EQ(cgGetNamedPass(technique, "null"), script->at(9).pass);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget0, script->at(10).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT0"), script->at(10).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget1, script->at(11).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT1"), script->at(11).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget2, script->at(12).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT2"), script->at(12).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget3, script->at(13).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT3"), script->at(13).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderDepthStencilTarget, script->at(14).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "RT4"), script->at(14).parameter);
}

TEST_F(EffectTest, ParseInvalidRenderTargetsScript)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/scripts.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    CGtechnique technique = cgGetNamedTechnique(effectPtr, "InvalidRenderTargets");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(1, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(0).type);
    ASSERT_EQ(cgGetNamedPass(technique, "null"), script->at(0).pass);
}

TEST_F(EffectTest, ParseLoopScript)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CGeffect effectPtr;
    cg::Effect *effect = createEffect(":effects/scripts.cgfx", scene, delegate, effectPtr);
    EffectEngine engine(&scene, 0, effect, &delegate);
    CGtechnique technique = cgGetNamedTechnique(effectPtr, "Loop");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(4, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kLoopByCount, script->at(0).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "LoopCountNum"), script->at(0).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopGetIndex, script->at(1).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "LoopIndexIn"), script->at(1).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopGetIndex, script->at(2).type);
    ASSERT_EQ(cgGetNamedEffectParameter(effectPtr, "LoopIndexIn2"), script->at(2).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopEnd, script->at(3).type);
    // try executing the script to get the value of LoopIndexIn
    engine.executeTechniquePasses(technique, 0 , 0, 0, 0);
    Vector3 value;
    cgGLGetParameter1f(cgGetNamedEffectParameter(effectPtr, "LoopIndexIn"), value);
    ASSERT_FLOAT_EQ(42, value.x());
    cgGLGetParameter1f(cgGetNamedEffectParameter(effectPtr, "LoopIndexIn2"), value);
    ASSERT_FLOAT_EQ(42, value.x());
}

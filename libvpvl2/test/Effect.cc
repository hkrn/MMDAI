#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/cg/EffectEngine.h>
#include <vpvl2/qt/CString.h>
#include "mock/Model.h"
#include "mock/RenderDelegate.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::cg;
using namespace vpvl2::qt;
using ::testing::InSequence;

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
    void setSource(MockIRenderDelegate &delegate, const CString &mockPath, const QString &effectPath) {
        QFile file(effectPath);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            CString *source = new CString(bytes);
            EXPECT_CALL(delegate, loadShaderSource(IRenderDelegate::kModelEffectTechniques, &mockPath))
                    .Times(1).WillRepeatedly(Return(source));
        }
    }
};

TEST_F(EffectTest, ToBool)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/util.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    CGeffect effectPtr = static_cast<CGeffect>(effect->internalPointer());
    ASSERT_TRUE(effectPtr);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/util.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    CGeffect effectPtr = static_cast<CGeffect>(effect->internalPointer());
    ASSERT_TRUE(effectPtr);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/util.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    CGeffect effectPtr = static_cast<CGeffect>(effect->internalPointer());
    ASSERT_TRUE(effectPtr);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/util.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    CGeffect effectPtr = static_cast<CGeffect>(effect->internalPointer());
    ASSERT_TRUE(effectPtr);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/util.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    CGeffect effectPtr = static_cast<CGeffect>(effect->internalPointer());
    ASSERT_TRUE(effectPtr);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/matrices.cgfx");
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kInverseMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
    setMatrix(delegate, modelPtr, IRenderDelegate::kLightMatrix  | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
    engine.setModelMatrixParameters(modelPtr, 0, 0);
}

TEST_F(EffectTest, LoadMaterialColors)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/materials.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/geometries.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/times.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/specials.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
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
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/preprocess.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
    ASSERT_EQ(EffectEngine::kScene, engine.scriptClass());
    ASSERT_EQ(IEffect::kPreProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(1, techniques.size());
    ASSERT_STREQ("test", cgGetTechniqueName(techniques[0]));
}

TEST_F(EffectTest, LoadSASStandard)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/standard.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
    ASSERT_EQ(EffectEngine::kObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kStandard, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.size());
    ASSERT_STREQ("test1", cgGetTechniqueName(techniques[0]));
    ASSERT_STREQ("test2", cgGetTechniqueName(techniques[1]));
    ASSERT_STREQ("test3", cgGetTechniqueName(techniques[2]));
}

TEST_F(EffectTest, LoadPostProcess)
{
    MockIRenderDelegate delegate;
    Scene scene;
    CString path("/foo/bar/path");
    setSource(delegate, path, ":effects/postprocess.cgfx");
    cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffect(&path, &delegate));
    ASSERT_TRUE(effect->internalPointer());
    EffectEngine engine(&scene, &path, effect, &delegate);
    ASSERT_EQ(EffectEngine::kSceneOrObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kPostProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(1, techniques.size());
    ASSERT_STREQ("test", cgGetTechniqueName(techniques[0]));
}

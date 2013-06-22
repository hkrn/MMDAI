#include "Common.h"
#include "vpvl2/vpvl2.h"
#include "vpvl2/fx/EffectEngine.h"
#include "vpvl2/extensions/fx/Util.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"
#include "mock/ApplicationContext.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::fx;
using namespace vpvl2::extensions::fx;
using namespace vpvl2::extensions::icu4c;

namespace {

static void AssertParameterFloat(const IEffect *effectPtr, const char *name, float expected)
{
    float v;
    SCOPED_TRACE(name);
    const IEffect::IParameter *parameter = effectPtr->findUniformParameter(name);
    ASSERT_TRUE(parameter);
    parameter->getValue(v);
    ASSERT_FLOAT_EQ(expected, v);
}

template<typename T>
static void AssertParameterVector(const IEffect *effectPtr, const char *name, const T &expected)
{
    T v;
    SCOPED_TRACE(name);
    const IEffect::IParameter *parameter = effectPtr->findUniformParameter(name);
    ASSERT_TRUE(parameter);
    parameter->getValue(v);
    ASSERT_TRUE(CompareVector(expected, v));
}

static void AssertParameterMatrix(const IEffect *effectPtr, const char *name, const float *expected)
{
    float v[16] = { 0 };
    SCOPED_TRACE(name);
    const IEffect::IParameter *parameter = effectPtr->findUniformParameter(name);
    ASSERT_TRUE(parameter);
    parameter->getMatrix(v);
    AssertMatrix(expected, v);
}

class MockEffectEngine : public EffectEngine {
public:
    MockEffectEngine(Scene *scene, IEffect *effectRef, IApplicationContext *applicationContextRef)
        : EffectEngine(scene, applicationContextRef)
    {
        setEffect(effectRef, 0, false);
    }

protected:
    void drawPrimitives(const DrawPrimitiveCommand & /* command */) const
    {
    }
    void rebindVertexBundle() {}
};

class EffectTest : public ::testing::Test {
public:
    void setMatrix(MockIApplicationContext &applicationContext, const IModel *modelPtr, int flags) {
        int cw = IApplicationContext::kWorldMatrix | flags;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cw)).Times(1).WillRepeatedly(Return());
        int cv = IApplicationContext::kViewMatrix | flags;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cv)).Times(1).WillRepeatedly(Return());
        int cp = IApplicationContext::kProjectionMatrix | flags;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cp)).Times(1).WillRepeatedly(Return());
        int cwv = cw | cv;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cwv)).Times(1).WillRepeatedly(Return());
        int cvp = cv | cp;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cvp)).Times(1).WillRepeatedly(Return());
        int cwvp = cw | cv | cp;
        EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, cwvp)).Times(1).WillRepeatedly(Return());
    }
    cg::Effect *createEffect(const QString &effectPath, Scene &scene, MockIApplicationContext &applicationContext, CGeffect &ptr) {
        QFile file(effectPath);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            EXPECT_CALL(applicationContext, getEffectCompilerArguments(_)).Times(AnyNumber());
            EXPECT_CALL(applicationContext, getViewport(_)).Times(AnyNumber());
            EXPECT_CALL(applicationContext, tryGetSharedTextureParameter(_, _))
                    .Times(AnyNumber()).WillRepeatedly(Return(false));
            String source(UnicodeString::fromUTF8(bytes.constData()));
            cg::Effect *effect = dynamic_cast<cg::Effect *>(scene.createEffectFromSource(&source, &applicationContext));
            ptr = static_cast<CGeffect>(effect->internalPointer());
            return effect;
        }
        return 0;
    }
};

}

TEST_F(EffectTest, IsPassEquals)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<IEffect> ptr(createEffect(":effects/util.cgfx", scene, applicationContext, effectPtr));
    IEffect::IParameter *parameter = ptr->findUniformParameter("ValueTest");
    const char target[] = "This is string.";
    ASSERT_TRUE(Util::isPassEquals(parameter->annotationRef("NoSuchAnnotation"), target));
    ASSERT_FALSE(Util::isPassEquals(parameter->annotationRef("BooleanTrueValue"), target));
    ASSERT_FALSE(Util::isPassEquals(parameter->annotationRef("BooleanFalseValue"), target));
    ASSERT_FALSE(Util::isPassEquals(parameter->annotationRef("IntegerValue"), target));
    ASSERT_FALSE(Util::isPassEquals(parameter->annotationRef("FloatValue"), target));
    ASSERT_TRUE(Util::isPassEquals(parameter->annotationRef("StringValue"), target));
}

TEST_F(EffectTest, Trim)
{
    ASSERT_EQ("spaces_before_this_string_should_be_trimmed", Util::trim("    spaces_before_this_string_should_be_trimmed"));
    ASSERT_EQ("spaces_after_this_string_should_be_trimmed", Util::trim("spaces_after_this_string_should_be_trimmed     "));
    ASSERT_EQ("spaces inside this string should not be trimmed", Util::trim("spaces inside this string should not be trimmed"));
    ASSERT_EQ("", Util::trim(""));
    ASSERT_EQ("semicolon_should_be_trimmed", Util::trimLastSemicolon("semicolon_should_be_trimmed;"));
    ASSERT_EQ("semicolon_and_after_spaces_should_be_trimmed", Util::trimLastSemicolon("   semicolon_and_after_spaces_should_be_trimmed;       "));
    ASSERT_EQ("spaces_and_inside_semicolon_spaces_should_be_trimmed", Util::trimLastSemicolon("    spaces_and_inside_semicolon_spaces_should_be_trimmed    ;       "));
    ASSERT_EQ("", Util::trimLastSemicolon(""));
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
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<IEffect> ptr(createEffect(":effects/util.cgfx", scene, applicationContext, effectPtr));
    const int nexpects = sizeof(expects) / sizeof(expects[0]);
    for (int i = 0; i < nexpects; i++) {
        Expect &e = expects[i];
        const IEffect::IParameter *parameter = ptr->findUniformParameter(e.name);
        EXPECT_TRUE(parameter);
        EXPECT_EQ(e.expected, Util::isIntegerParameter(parameter));
    }
}

TEST_F(EffectTest, LoadMatrices)
{
    MockIApplicationContext applicationContext;
    MockIModel model, *modelPtr = &model;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/matrices.cgfx", scene, applicationContext, effectPtr));
    setMatrix(applicationContext, modelPtr, IApplicationContext::kCameraMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kLightMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kCameraMatrix | IApplicationContext::kInverseMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kLightMatrix  | IApplicationContext::kInverseMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kCameraMatrix | IApplicationContext::kTransposeMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kLightMatrix  | IApplicationContext::kTransposeMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kCameraMatrix | IApplicationContext::kInverseMatrix | IApplicationContext::kTransposeMatrix);
    setMatrix(applicationContext, modelPtr, IApplicationContext::kLightMatrix  | IApplicationContext::kInverseMatrix | IApplicationContext::kTransposeMatrix);
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    engine.setModelMatrixParameters(modelPtr, 0, 0);
}

TEST_F(EffectTest, LoadMaterialColors)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/materials.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    Vector4 v;
    float f;
    {
        engine.ambient.geometryParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.01, 0.02, 0.03, 0.04), v);
        engine.diffuse.geometryParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.05, 0.06, 0.07, 0.08), v);
        engine.emissive.geometryParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.09, 0.10, 0.11, 0.12), v);
        engine.specular.geometryParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.13, 0.14, 0.15, 0.16), v);
        engine.specularPower.geometryParameter()->getValue(f);
        ASSERT_FLOAT_EQ(0.17, f);
        engine.toonColor.geometryParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.18, 0.19, 0.20, 0.21), v);
    }
    {
        engine.ambient.lightParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.22, 0.23, 0.24, 0.25), v);
        engine.diffuse.lightParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.26, 0.27, 0.28, 0.29), v);
        engine.specular.lightParameter()->getValue(v);
        ASSERT_EQ(Vector4(0.30, 0.31, 0.32, 0.33), v);
    }
}

TEST_F(EffectTest, LoadGeometries)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/geometries.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    Vector4 v;
    engine.direction.cameraParameter()->getValue(v);
    ASSERT_EQ(Vector4(0.01, 0.02, 0.03, 0.04), v);
    engine.position.cameraParameter()->getValue(v);
    ASSERT_EQ(Vector4(0.05, 0.06, 0.07, 0.08), v);
    engine.direction.lightParameter()->getValue(v);
    ASSERT_EQ(Vector4(0.09, 0.10, 0.11, 0.12), v);
    engine.position.lightParameter()->getValue(v);
    ASSERT_EQ(Vector4(0.13, 0.14, 0.15, 0.16), v);
}

TEST_F(EffectTest, LoadControlObjectWithoutAsset)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/controlobjects.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    EXPECT_CALL(applicationContext, findModel(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<IModel *>(0)));
    EXPECT_CALL(applicationContext, toUnicode(_)).Times(AnyNumber()).WillRepeatedly(ReturnNew<String>("asset"));
    engine.controlObject.update(0);
    AssertParameterFloat(ptr.data(), "no_such_asset_bool", 0);
    AssertParameterFloat(ptr.data(), "no_such_asset_float", 0);
    AssertParameterVector(ptr.data(), "no_such_asset_float3", kZeroV3);
    AssertParameterVector(ptr.data(), "no_such_asset_float4", kZeroV4);
    AssertParameterMatrix(ptr.data(), "no_such_asset_float4x4", kIdentity4x4);
    AssertParameterFloat(ptr.data(), "no_such_X_float", 0);
    AssertParameterFloat(ptr.data(), "no_such_Y_float", 0);
    AssertParameterFloat(ptr.data(), "no_such_Z_float", 0);
    AssertParameterVector(ptr.data(), "no_such_XYZ_float", kZeroV3);
    AssertParameterFloat(ptr.data(), "no_such_Rx_float", 0);
    AssertParameterFloat(ptr.data(), "no_such_Ry_float", 0);
    AssertParameterFloat(ptr.data(), "no_such_Rz_float", 0);
    AssertParameterVector(ptr.data(), "no_such_Rxyz_float", kZeroV3);
    AssertParameterFloat(ptr.data(), "no_such_Si_float", 0);
    AssertParameterFloat(ptr.data(), "no_such_Tr_float", 0);
}

static const float kScaleFactor = 0.1;
static const float kOpacity = 0.2;
static const Vector4 kPosition = Vector4(0.01, 0.02, 0.03, 0);
static const Quaternion kRotation = Quaternion(0.11, 0.12, 0.13, 0.14);
static void MatrixSetIdentity(float *value, const IModel * /* model */, int /* flags */)
{
    memcpy(value, kIdentity4x4, sizeof(kIdentity4x4));
}

TEST_F(EffectTest, LoadControlObjectWithAsset)
{
    MockIApplicationContext applicationContext;
    MockIModel model, *modelPtr = &model;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/controlobjects.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    EXPECT_CALL(model, isVisible()).Times(AnyNumber()).WillRepeatedly(Return(true));
    EXPECT_CALL(model, worldPosition()).Times(AnyNumber()).WillRepeatedly(Return(kPosition));
    EXPECT_CALL(model, worldRotation()).Times(AnyNumber()).WillRepeatedly(Return(kRotation));
    EXPECT_CALL(model, scaleFactor()).Times(AnyNumber()).WillRepeatedly(Return(kScaleFactor));
    EXPECT_CALL(model, opacity()).Times(AnyNumber()).WillRepeatedly(Return(kOpacity));
    EXPECT_CALL(model, type()).Times(AnyNumber()).WillRepeatedly(Return(IModel::kAssetModel));
    EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, _)).Times(AnyNumber()).WillRepeatedly(Invoke(MatrixSetIdentity));
    EXPECT_CALL(applicationContext, findModel(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<IModel *>(&model)));
    EXPECT_CALL(applicationContext, toUnicode(_)).Times(AnyNumber()).WillRepeatedly(ReturnNew<String>("asset"));
    engine.controlObject.update(&model);
    AssertParameterFloat(ptr.data(), "asset_bool", 1);
    AssertParameterFloat(ptr.data(), "asset_float", kScaleFactor);
    AssertParameterVector(ptr.data(), "asset_float3", kPosition);
    AssertParameterVector(ptr.data(), "asset_float4", kPosition);
    AssertParameterMatrix(ptr.data(), "asset_float4x4", kIdentity4x4);
    AssertParameterFloat(ptr.data(), "X_float", kPosition.x());
    AssertParameterFloat(ptr.data(), "Y_float", kPosition.y());
    AssertParameterFloat(ptr.data(), "Z_float", kPosition.z());
    AssertParameterVector(ptr.data(), "XYZ_float", kPosition);
    AssertParameterFloat(ptr.data(), "Rx_float", btDegrees(kRotation.x()));
    AssertParameterFloat(ptr.data(), "Ry_float", btDegrees(kRotation.y()));
    AssertParameterFloat(ptr.data(), "Rz_float", btDegrees(kRotation.z()));
    AssertParameterVector(ptr.data(), "Rxyz_float", Vector3(btDegrees(kRotation.x()),
                                                            btDegrees(kRotation.y()),
                                                            btDegrees(kRotation.z())));
    AssertParameterFloat(ptr.data(), "Si_float", kScaleFactor);
    AssertParameterFloat(ptr.data(), "Tr_float", kOpacity);
}

TEST_F(EffectTest, LoadControlObjectWithoutModel)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/controlobjects.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    EXPECT_CALL(applicationContext, findModel(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<IModel *>(0)));
    EXPECT_CALL(applicationContext, toUnicode(_)).Times(AnyNumber()).WillRepeatedly(ReturnNew<String>("model"));
    engine.controlObject.update(0);
    AssertParameterFloat(ptr.data(), "no_such_model_bool", 0);
    AssertParameterFloat(ptr.data(), "no_such_model_float", 0);
    AssertParameterVector(ptr.data(), "no_such_model_float3", kZeroV3);
    AssertParameterVector(ptr.data(), "no_such_model_float4", kZeroV4);
    AssertParameterMatrix(ptr.data(), "no_such_model_float4x4", kIdentity4x4);
}

TEST_F(EffectTest, LoadControlObjectWithModel)
{
    MockIApplicationContext applicationContext;
    MockIModel model, *modelPtr = &model;
    MockIBone bone, *bonePtr = &bone;
    MockIMorph morph, *morphPtr = &morph;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/controlobjects.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    Transform boneTransform;
    boneTransform.setIdentity();
    boneTransform.setOrigin(kPosition);
    EXPECT_CALL(bone, worldTransform()).Times(AnyNumber()).WillRepeatedly(Return(boneTransform));
    EXPECT_CALL(morph, weight()).Times(AnyNumber()).WillRepeatedly(Return(kScaleFactor));
    EXPECT_CALL(model, isVisible()).Times(AnyNumber()).WillRepeatedly(Return(true));
    EXPECT_CALL(model, worldPosition()).Times(AnyNumber()).WillRepeatedly(Return(kPosition));
    EXPECT_CALL(model, scaleFactor()).Times(AnyNumber()).WillRepeatedly(Return(kScaleFactor));
    EXPECT_CALL(model, type()).Times(AnyNumber()).WillRepeatedly(Return(IModel::kPMDModel));
    EXPECT_CALL(model, findBoneRef(_)).Times(AnyNumber()).WillRepeatedly(Return(bonePtr));
    EXPECT_CALL(model, findMorphRef(_)).Times(AnyNumber()).WillRepeatedly(Return(morphPtr));
    EXPECT_CALL(applicationContext, getMatrix(_, modelPtr, _)).Times(AnyNumber()).WillRepeatedly(Invoke(MatrixSetIdentity));
    EXPECT_CALL(applicationContext, findModel(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<IModel *>(&model)));
    EXPECT_CALL(applicationContext, toUnicode(_)).Times(AnyNumber()).WillRepeatedly(ReturnNew<String>("asset"));
    engine.controlObject.update(&model);
    AssertParameterFloat(ptr.data(), "model_bool", 1);
    AssertParameterFloat(ptr.data(), "model_float", kScaleFactor);
    AssertParameterVector(ptr.data(), "model_float3", kPosition);
    AssertParameterVector(ptr.data(), "model_float4", kPosition);
    AssertParameterMatrix(ptr.data(), "model_float4x4", kIdentity4x4);
    AssertParameterVector(ptr.data(), "bone_float3", kPosition);
    AssertParameterVector(ptr.data(), "bone_float4", kPosition);
    float m[16] = { 0 };
    boneTransform.getOpenGLMatrix(m);
    AssertParameterMatrix(ptr.data(), "bone_float4x4", m);
    // TODO: implement here
    // AssertParameterFloat(effectPtr, "model_morph", kScaleFactor);
}

TEST_F(EffectTest, LoadTimes)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/times.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    float f;
    engine.time.syncDisabledParameter()->getValue(f);
    ASSERT_FLOAT_EQ(0.1, f);
    engine.elapsedTime.syncDisabledParameter()->getValue(f);
    ASSERT_FLOAT_EQ(0.2, f);
    engine.time.syncEnabledParameter()->getValue(f);
    ASSERT_FLOAT_EQ(0.3, f);
    engine.elapsedTime.syncEnabledParameter()->getValue(f);
    ASSERT_FLOAT_EQ(0.4, f);
}

TEST_F(EffectTest, LoadSpecials)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/specials.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    float f;
    engine.parthf.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.spadd.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.transp.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.useTexture.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.useSpheremap.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.useToon.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.opadd.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(1.0, f);
    engine.vertexCount.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(2.0, f);
    engine.subsetCount.baseParameter()->getValue(f);
    ASSERT_FLOAT_EQ(3.0, f);
}

TEST_F(EffectTest, LoadSASPreProcess)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/preprocess.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_EQ(EffectEngine::kScene, engine.scriptClass());
    ASSERT_EQ(IEffect::kPreProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.count());
    ASSERT_STREQ("test1", techniques[0]->name());
    ASSERT_STREQ("test2", techniques[1]->name());
    ASSERT_STREQ("test3", techniques[2]->name());
    const EffectEngine::Script *nullScript = engine.findPassScript(techniques[0]->findPass("null"));
    ASSERT_EQ(1, nullScript->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawBuffer, nullScript->at(0).type);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(techniques[1]->findPass("null"));
    ASSERT_FALSE(nullScript2);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(techniques[2]->findPass("null"));
    ASSERT_FALSE(nullScript3);
}

TEST_F(EffectTest, LoadSASStandard)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/standard.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_EQ(EffectEngine::kObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kStandard, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.count());
    ASSERT_STREQ("test1", techniques[0]->name());
    ASSERT_STREQ("test2", techniques[1]->name());
    ASSERT_STREQ("test3", techniques[2]->name());
    const EffectEngine::Script *nullScript = engine.findPassScript(techniques[0]->findPass("null"));
    ASSERT_FALSE(nullScript);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(techniques[1]->findPass("null"));
    ASSERT_EQ(1, nullScript2->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript2->at(0).type);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(techniques[2]->findPass("null"));
    ASSERT_EQ(1, nullScript3->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript3->at(0).type);
}

TEST_F(EffectTest, LoadSASStandard2)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/standard2.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_EQ(EffectEngine::kObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kStandard, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(1, techniques.count());
    ASSERT_STREQ("test1", techniques[0]->name());
}

TEST_F(EffectTest, LoadSASPostProcess)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/postprocess.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_EQ(EffectEngine::kSceneOrObject, engine.scriptClass());
    ASSERT_EQ(IEffect::kPostProcess, engine.scriptOrder());
    ASSERT_EQ(EffectEngine::kColor, engine.scriptOutput());
    const EffectEngine::Techniques &techniques = engine.techniques();
    ASSERT_EQ(3, techniques.count());
    ASSERT_STREQ("test1", techniques[0]->name());
    ASSERT_STREQ("test2", techniques[1]->name());
    ASSERT_STREQ("test3", techniques[2]->name());
    const EffectEngine::Script *nullScript = engine.findPassScript(techniques[0]->findPass("null"));
    ASSERT_EQ(1, nullScript->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawBuffer, nullScript->at(0).type);
    const EffectEngine::Script *nullScript2 = engine.findPassScript(techniques[1]->findPass("null"));
    ASSERT_EQ(1, nullScript2->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript2->at(0).type);
    const EffectEngine::Script *nullScript3 = engine.findPassScript(techniques[2]->findPass("null"));
    ASSERT_EQ(1, nullScript3->size());
    ASSERT_EQ(EffectEngine::ScriptState::kDrawGeometry, nullScript3->at(0).type);
}

TEST_F(EffectTest, FindTechniques)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/techniques.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_FALSE(engine.findTechnique("no_such_object_type", 0, 42, false, false, false));
    ASSERT_STREQ("MainTec7",   engine.findTechnique("object",     1, 42, true,  true,  true)->name());
    ASSERT_STREQ("MainTec6",   engine.findTechnique("object",     2, 42, false, true,  true)->name());
    ASSERT_STREQ("MainTec5",   engine.findTechnique("object",     3, 42, true,  false, true)->name());
    ASSERT_STREQ("MainTec4",   engine.findTechnique("object",     4, 42, false, false, true)->name());
    ASSERT_STREQ("MainTec3",   engine.findTechnique("object",     5, 42, true,  true,  false)->name());
    ASSERT_STREQ("MainTec2",   engine.findTechnique("object",     6, 42, false, true,  false)->name());
    ASSERT_STREQ("MainTec1",   engine.findTechnique("object",     7, 42, true,  false, false)->name());
    ASSERT_STREQ("MainTec0",   engine.findTechnique("object",     8, 42, false, false, false)->name());
    ASSERT_STREQ("MainTecBS7", engine.findTechnique("object_ss",  9, 42, true,  true,  true)->name());
    ASSERT_STREQ("MainTecBS6", engine.findTechnique("object_ss", 10, 42, false, true,  true)->name());
    ASSERT_STREQ("MainTecBS5", engine.findTechnique("object_ss", 11, 42, true,  false, true)->name());
    ASSERT_STREQ("MainTecBS4", engine.findTechnique("object_ss", 12, 42, false, false, true)->name());
    ASSERT_STREQ("MainTecBS3", engine.findTechnique("object_ss", 13, 42, true,  true,  false)->name());
    ASSERT_STREQ("MainTecBS2", engine.findTechnique("object_ss", 14, 42, false, true,  false)->name());
    ASSERT_STREQ("MainTecBS1", engine.findTechnique("object_ss", 15, 42, true,  false, false)->name());
    ASSERT_STREQ("MainTecBS0", engine.findTechnique("object_ss", 16, 42, false, false, false)->name());
}

class FindTechnique : public EffectTest, public WithParamInterface< tuple<int, int, bool, bool, bool> > {};

TEST_P(FindTechnique, TestEdge)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/techniques.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_STREQ("EdgeTec", engine.findTechnique("edge",
                                                 get<0>(GetParam()),
                                                 get<1>(GetParam()),
                                                 get<2>(GetParam()),
                                                 get<3>(GetParam()),
                                                 get<4>(GetParam()))->name());
}

TEST_P(FindTechnique, TestShadow)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/techniques.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    ASSERT_STREQ("ShadowTec", engine.findTechnique("shadow",
                                                   get<0>(GetParam()),
                                                   get<1>(GetParam()),
                                                   get<2>(GetParam()),
                                                   get<3>(GetParam()),
                                                   get<4>(GetParam()))->name());
}

INSTANTIATE_TEST_CASE_P(EffectValueTest, FindTechnique,
                        Combine(Range(-1, 1), Values(2), Bool(), Bool(), Bool()));

TEST_F(EffectTest, ParseSyntaxErrorsScript)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/scripts.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    IEffect::ITechnique *technique = ptr->findTechnique("SyntaxErrors");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(1, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(0).type);
}

TEST_F(EffectTest, ParseRenderTargetsScript)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/scripts.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    IEffect::ITechnique *technique = ptr->findTechnique("RenderTargets");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(15, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget0, script->at(0).type);
    ASSERT_EQ(ptr->findUniformParameter("RT0"), script->at(0).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget1, script->at(1).type);
    ASSERT_EQ(ptr->findUniformParameter("RT1"), script->at(1).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget2, script->at(2).type);
    ASSERT_EQ(ptr->findUniformParameter("RT2"), script->at(2).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget3, script->at(3).type);
    ASSERT_EQ(ptr->findUniformParameter("RT3"), script->at(3).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderDepthStencilTarget, script->at(4).type);
    ASSERT_EQ(ptr->findUniformParameter("RT4"), script->at(4).renderDepthStencilBufferRef->parameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kClearSetColor, script->at(5).type);
    ASSERT_EQ(ptr->findUniformParameter("ClearColor"), script->at(5).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kClearSetDepth, script->at(6).type);
    ASSERT_EQ(ptr->findUniformParameter("ClearDepth"), script->at(6).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kClearColor, script->at(7).type);
    ASSERT_EQ(EffectEngine::ScriptState::kClearDepth, script->at(8).type);
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(9).type);
    ASSERT_EQ(technique->findPass("null"), script->at(9).pass);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget0, script->at(10).type);
    ASSERT_EQ(ptr->findUniformParameter("RT0"), script->at(10).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget1, script->at(11).type);
    ASSERT_EQ(ptr->findUniformParameter("RT1"), script->at(11).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget2, script->at(12).type);
    ASSERT_EQ(ptr->findUniformParameter("RT2"), script->at(12).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderColorTarget3, script->at(13).type);
    ASSERT_EQ(ptr->findUniformParameter("RT3"), script->at(13).renderColorTargetTextureRef->textureParameterRef);
    ASSERT_EQ(EffectEngine::ScriptState::kRenderDepthStencilTarget, script->at(14).type);
    ASSERT_EQ(ptr->findUniformParameter("RT4"), script->at(14).renderDepthStencilBufferRef->parameterRef);
}

TEST_F(EffectTest, ParseInvalidRenderTargetsScript)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/scripts.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    IEffect::ITechnique *technique = ptr->findTechnique("InvalidRenderTargets");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(1, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kPass, script->at(0).type);
    ASSERT_EQ(technique->findPass("null"), script->at(0).pass);
}

TEST_F(EffectTest, ParseLoopScript)
{
    MockIApplicationContext applicationContext;
    Scene scene(true);
    CGeffect effectPtr;
    QScopedPointer<cg::Effect> ptr(createEffect(":effects/scripts.cgfx", scene, applicationContext, effectPtr));
    EXPECT_CALL(applicationContext, findProcedureAddress(_)).Times(AnyNumber()).WillRepeatedly(Return(static_cast<void *>(0)));
    MockEffectEngine engine(&scene, ptr.data(), &applicationContext);
    IEffect::ITechnique *technique = ptr->findTechnique("Loop");
    ASSERT_TRUE(technique);
    const EffectEngine::Script *script = engine.findTechniqueScript(technique);
    ASSERT_TRUE(script);
    ASSERT_EQ(4, script->size());
    ASSERT_EQ(EffectEngine::ScriptState::kLoopByCount, script->at(0).type);
    ASSERT_EQ(ptr->findUniformParameter("LoopCountNum"), script->at(0).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopGetIndex, script->at(1).type);
    ASSERT_EQ(ptr->findUniformParameter("LoopIndexIn"), script->at(1).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopGetIndex, script->at(2).type);
    ASSERT_EQ(ptr->findUniformParameter("LoopIndexIn2"), script->at(2).parameter);
    ASSERT_EQ(EffectEngine::ScriptState::kLoopEnd, script->at(3).type);
    // try executing the script to get the value of LoopIndexIn
    engine.executeTechniquePasses(technique, EffectEngine::DrawPrimitiveCommand(), 0);
    float value;
    ptr->findUniformParameter("LoopIndexIn")->getValue(value);
    ASSERT_FLOAT_EQ(42, value);
    ptr->findUniformParameter("LoopIndexIn2")->getValue(value);
    ASSERT_FLOAT_EQ(42, value);
}

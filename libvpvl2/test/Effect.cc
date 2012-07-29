#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/cg/EffectEngine.h>
#include "Common.h"
#include "mock/Model.h"
#include "mock/RenderDelegate.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::cg;
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
    void setSource(MockIRenderDelegate &delegate, const String &mockPath, const QString &effectPath) {
        QFile file(effectPath);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            String *source = new String(bytes);
            EXPECT_CALL(delegate, loadShaderSource(IRenderDelegate::kModelEffectTechniques, &mockPath))
                    .Times(1).WillRepeatedly(Return(source));
        }
    }
};

TEST_F(EffectTest, LoadMatrices)
{
    MockIRenderDelegate delegate;
    MockIModel model, *modelPtr = &model;
    Scene scene;
    String path("/foo/bar/path");
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
    ASSERT_TRUE(engine.attachEffect(effect, &path));
    engine.setModelMatrixParameters(modelPtr, 0, 0);
}

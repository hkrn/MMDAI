namespace vpvl2 {
namespace VPVL2_VERSION_NS {

class MockIRenderEngine : public IRenderEngine {
 public:
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_METHOD1(upload,
      bool(void *userData));
  MOCK_METHOD0(release,
      void());
  MOCK_METHOD1(renderModel,
      void(IEffect::Pass *overridePass));
  MOCK_METHOD1(renderEdge,
      void(IEffect::Pass *overridePass));
  MOCK_METHOD1(renderShadow,
      void(IEffect::Pass *overridePass));
  MOCK_METHOD1(renderZPlot,
      void(IEffect::Pass *overridePass));
  MOCK_METHOD0(update,
      void());
  MOCK_METHOD1(setUpdateOptions,
      void(int options));
  MOCK_CONST_METHOD0(hasPreProcess,
      bool());
  MOCK_CONST_METHOD0(hasPostProcess,
      bool());
  MOCK_METHOD0(preparePostProcess,
      void());
  MOCK_METHOD0(performPreProcess,
      void());
  MOCK_METHOD1(performPostProcess,
      void(IEffect *nextPostEffect));
  MOCK_CONST_METHOD1(effectRef,
      IEffect*(IEffect::ScriptOrderType type));
  MOCK_METHOD3(setEffect,
      void(IEffect *effectRef, IEffect::ScriptOrderType type, void *userData));
  MOCK_METHOD0(testVisible,
      bool());
};

}  // namespace VPVL2_VERSION_NS
}  // namespace vpvl2

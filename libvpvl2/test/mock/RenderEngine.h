namespace vpvl2 {

class MockIRenderEngine : public IRenderEngine {
 public:
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_METHOD1(upload,
      bool(void *userData));
  MOCK_METHOD0(release,
      void());
  MOCK_METHOD0(renderModel,
      void());
  MOCK_METHOD0(renderEdge,
      void());
  MOCK_METHOD0(renderShadow,
      void());
  MOCK_METHOD0(renderZPlot,
      void());
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
  MOCK_METHOD1(setOverridePass,
      void(IEffect::Pass *pass));
  MOCK_METHOD0(testVisible,
      bool());
};

}  // namespace vpvl2

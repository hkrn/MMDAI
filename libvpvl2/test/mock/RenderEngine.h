namespace vpvl2 {

class MockIRenderEngine : public IRenderEngine {
 public:
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_METHOD1(upload,
      bool(const IString *dir));
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
  MOCK_CONST_METHOD1(effect,
      IEffect*(IEffect::ScriptOrderType type));
  MOCK_METHOD3(setEffect,
      void(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir));
};

}  // namespace vpvl2

namespace vpvl2 {

class MockIRenderDelegate : public IRenderDelegate {
 public:
  MOCK_METHOD2(allocateContext,
      void(const IModel *model, void *&context));
  MOCK_METHOD2(releaseContext,
      void(const IModel *model, void *&context));
  MOCK_METHOD5(uploadTexture,
      bool(const IString *name, const IString *dir, int flags, Texture &texture, void *context));
  MOCK_CONST_METHOD3(getMatrix,
      void(float value[16], const IModel *model, int flags));
  MOCK_METHOD4(log,
      void(void *context, LogLevel level, const char *format, va_list ap));
  MOCK_METHOD2(loadShaderSource,
      IString*(ShaderType type, const IString *path));
  MOCK_METHOD4(loadShaderSource,
      IString*(ShaderType type, const IModel *model, const IString *dir, void *context));
  MOCK_METHOD2(loadKernelSource,
      IString*(KernelType type, void *context));
  MOCK_CONST_METHOD1(toUnicode,
      IString*(const uint8_t *str));
  MOCK_CONST_METHOD1(hasExtension,
      bool(const char *name));
  MOCK_CONST_METHOD1(extensionProcAddress,
      void*(const char *name));
  MOCK_METHOD4(getToonColor,
      void(const IString *name, const IString *dir, Color &value, void *context));
  MOCK_CONST_METHOD1(getViewport,
      void(Vector3 &value));
  MOCK_CONST_METHOD2(getMousePosition,
      void(Vector4 &value, MousePositionType type));
  MOCK_CONST_METHOD2(getTime,
      void(float &value, bool sync));
  MOCK_CONST_METHOD2(getElapsed,
      void(float &value, bool sync));
  MOCK_METHOD4(uploadAnimatedTexture,
      void(float offset, float speed, float seek, void *texture));
  MOCK_CONST_METHOD1(findModel,
      IModel*(const IString *name));
  MOCK_CONST_METHOD1(effectOwner,
      IModel*(const IEffect *effect));
  MOCK_METHOD2(setRenderColorTargets,
      void(const void *targets, const int ntargets));
  MOCK_METHOD5(bindRenderColorTarget,
      void(void *texture, size_t width, size_t height, int index, bool enableAA));
  MOCK_METHOD5(releaseRenderColorTarget,
      void(void *texture, size_t width, size_t height, int index, bool enableAA));
  MOCK_METHOD6(bindRenderDepthStencilTarget,
      void(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA));
  MOCK_METHOD6(releaseRenderDepthStencilTarget,
      void(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA));
};

}  // namespace vpvl2

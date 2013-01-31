namespace vpvl2 {

class MockIRenderContext : public IRenderContext {
 public:
  MOCK_METHOD2(allocateUserData,
      void(const IModel *model, void *&userData));
  MOCK_METHOD2(releaseUserData,
      void(const IModel *model, void *&userData));
  MOCK_METHOD4(uploadTexture,
      bool(const IString *name, const IString *dir, Texture &texture, void *context));
  MOCK_CONST_METHOD3(getMatrix,
      void(float value[16], const IModel *model, int flags));
  MOCK_CONST_METHOD4(log,
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
      bool(const void *namePtr));
  MOCK_CONST_METHOD1(findProcedureAddress,
      void*(const void **candidatesPtr));
  MOCK_METHOD2(startProfileSession,
      void(ProfileType type, const void *arg));
  MOCK_METHOD2(stopProfileSession,
      void(ProfileType type, const void *arg));
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
  MOCK_METHOD0(createFrameBufferObject,
      FrameBufferObject*());
  MOCK_CONST_METHOD1(getEffectCompilerArguments,
      void(Array<IString *> &arguments));
  MOCK_CONST_METHOD2(effectFilePath,
      const IString*(const IModel *model, const IString *dir));
  MOCK_METHOD2(addSharedTextureParameter,
      void(const char *name, const SharedTextureParameter &parameter));
  MOCK_CONST_METHOD2(tryGetSharedTextureParameter,
      bool(const char *name, SharedTextureParameter &parameter));
};

}  // namespace vpvl2

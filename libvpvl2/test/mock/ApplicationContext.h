namespace vpvl2 {

class MockIApplicationContext : public IApplicationContext {
 public:
  MOCK_METHOD3(uploadTexture,
      bool(const IString *name, TextureDataBridge &bridge, void *userData));
  MOCK_CONST_METHOD3(getMatrix,
      void(float32 value[16], const IModel *model, int flags));
  MOCK_METHOD2(loadShaderSource,
      IString*(ShaderType type, const IString *path));
  MOCK_METHOD3(loadShaderSource,
      IString*(ShaderType type, const IModel *model, void *userData));
  MOCK_METHOD2(loadKernelSource,
      IString*(KernelType type, void *userData));
  MOCK_CONST_METHOD1(toUnicode,
      IString*(const uint8 *str));
  MOCK_METHOD2(startProfileSession,
      void(ProfileType type, const void *arg));
  MOCK_METHOD2(stopProfileSession,
      void(ProfileType type, const void *arg));
  MOCK_METHOD3(getToonColor,
      void(const IString *name, Color &value, void *userData));
  MOCK_CONST_METHOD1(getViewport,
      void(Vector3 &value));
  MOCK_CONST_METHOD2(getMousePosition,
      void(Vector4 &value, MousePositionType type));
  MOCK_CONST_METHOD2(getTime,
      void(float &value, bool sync));
  MOCK_CONST_METHOD2(getElapsed,
      void(float &value, bool sync));
  MOCK_METHOD4(uploadAnimatedTexture,
      void(float32 offset, float32 speed, float32 seek, void *texture));
  MOCK_CONST_METHOD1(findModel,
      IModel*(const IString *name));
  MOCK_CONST_METHOD1(effectOwner,
      IModel*(const IEffect *effect));
  MOCK_METHOD0(createFrameBufferObject,
      extensions::gl::FrameBufferObject*());
  MOCK_CONST_METHOD1(getEffectCompilerArguments,
      void(Array<IString *> &arguments));
  MOCK_CONST_METHOD2(effectFilePath,
      const IString*(const IModel *model, const IString *dir));
  MOCK_METHOD2(addSharedTextureParameter,
      void(const char *name, const SharedTextureParameter &parameter));
  MOCK_CONST_METHOD2(tryGetSharedTextureParameter,
      bool(const char *name, SharedTextureParameter &parameter));
  MOCK_CONST_METHOD0(sharedFunctionResolverInstance,
      FunctionResolver*());
};

}  // namespace vpvl2

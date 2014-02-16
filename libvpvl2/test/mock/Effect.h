namespace vpvl2 {
namespace VPVL2_VERSION_NS {

class MockIEffect : public IEffect {
 public:
  MOCK_CONST_METHOD0(internalContext,
      void*());
  MOCK_CONST_METHOD0(internalPointer,
      void*());
  MOCK_CONST_METHOD1(getOffscreenRenderTargets,
      void(Array<OffscreenRenderTarget> &value));
  MOCK_CONST_METHOD1(getInteractiveParameters,
      void(Array<IEffect::Parameter *> &value));
  MOCK_CONST_METHOD0(parentEffectRef,
      IEffect*());
  MOCK_METHOD1(setParentEffectRef,
      void(IEffect *value));
  MOCK_CONST_METHOD0(parentFrameBufferObject,
      gl::FrameBufferObject*());
  MOCK_METHOD0(createFrameBufferObject,
      void());
  MOCK_CONST_METHOD0(scriptOrderType,
      ScriptOrderType());
  MOCK_METHOD3(addOffscreenRenderTarget,
      void(ITexture *textureRef, Parameter *textureParameterRef, Parameter *samplerParameterRef));
  MOCK_METHOD1(addInteractiveParameter,
      void(IEffect::Parameter *value));
  MOCK_METHOD1(addRenderColorTargetIndex,
      void(int targetIndex));
  MOCK_METHOD1(removeRenderColorTargetIndex,
      void(int targetIndex));
  MOCK_METHOD0(clearRenderColorTargetIndices,
      void());
  MOCK_METHOD1(setScriptOrderType,
      void(ScriptOrderType value));
  MOCK_CONST_METHOD1(hasRenderColorTargetIndex,
      bool(int targetIndex));
  MOCK_CONST_METHOD1(findUniformParameter,
      IEffect::Parameter*(const char *name));
  MOCK_CONST_METHOD1(findTechnique,
      IEffect::Technique*(const char *name));
  MOCK_CONST_METHOD1(getParameterRefs,
      void(Array<Parameter *> &parameters));
  MOCK_CONST_METHOD1(getTechniqueRefs,
      void(Array<Technique *> &techniques));
  MOCK_METHOD4(setVertexAttributePointer,
      void(VertexAttributeType vtype, Parameter::Type ptype, vsize stride, const void *ptr));
  MOCK_METHOD1(activateVertexAttribute,
      void(VertexAttributeType vtype));
  MOCK_METHOD1(deactivateVertexAttribute,
      void(VertexAttributeType vtype));
  MOCK_METHOD1(setupOverride,
      void(const IEffect *effectRef));
  MOCK_CONST_METHOD0(name,
      const IString*());
  MOCK_METHOD1(setName,
      void(const IString *value));
  MOCK_CONST_METHOD0(isEnabled,
      bool());
  MOCK_METHOD1(setEnabled,
      void(bool value));
  MOCK_METHOD1(recompileFromFile,
      bool(const char *filePath));
  MOCK_METHOD2(recompileFromSource,
      bool(const char *source, int length));
  MOCK_CONST_METHOD0(isDirty,
      bool());
  MOCK_METHOD1(setDirty,
      void(bool value));
  MOCK_CONST_METHOD0(errorString,
      const char*());
};

}  // namespace VPVL2_VERSION_NS
}  // namespace vpvl2

namespace vpvl2 {

class MockIMotion : public IMotion {
 public:
  MOCK_METHOD2(load,
      bool(const uint8_t *data, size_t size));
  MOCK_CONST_METHOD1(save,
      void(uint8_t *data));
  MOCK_CONST_METHOD0(estimateSize,
      size_t());
  MOCK_CONST_METHOD0(parentModel,
      IModel*());
  MOCK_METHOD1(setParentModel,
      void(IModel *model));
  MOCK_METHOD1(seek,
      void(const IKeyframe::TimeIndex &timeIndex));
  MOCK_METHOD2(seekScene,
      void(const IKeyframe::TimeIndex &timeIndex, Scene *scene));
  MOCK_METHOD1(advance,
      void(const IKeyframe::TimeIndex &deltaTimeIndex));
  MOCK_METHOD2(advanceScene,
      void(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene));
  MOCK_METHOD0(reset,
      void());
  MOCK_CONST_METHOD0(maxTimeIndex,
      IKeyframe::TimeIndex());
  MOCK_CONST_METHOD1(isReachedTo,
      bool(const IKeyframe::TimeIndex &timeIndex));
  MOCK_METHOD1(addKeyframe,
      void(IKeyframe *value));
  MOCK_CONST_METHOD1(countKeyframes,
      int(IKeyframe::Type value));
  MOCK_CONST_METHOD2(countLayers,
      IKeyframe::LayerIndex(const IString *name, IKeyframe::Type type));
  MOCK_METHOD4(getKeyframes,
      void(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex, IKeyframe::Type type, Array<IKeyframe *> &keyframes));
  MOCK_CONST_METHOD3(findBoneKeyframe,
      IBoneKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IString *name, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findBoneKeyframeAt,
      IBoneKeyframe*(int index));
  MOCK_CONST_METHOD2(findCameraKeyframe,
      ICameraKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findCameraKeyframeAt,
      ICameraKeyframe*(int index));
  MOCK_CONST_METHOD3(findEffectKeyframe,
      IEffectKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IString *name, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findEffectKeyframeAt,
      IEffectKeyframe*(int index));
  MOCK_CONST_METHOD2(findLightKeyframe,
      ILightKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findLightKeyframeAt,
      ILightKeyframe*(int index));
  MOCK_CONST_METHOD2(findModelKeyframe,
      IModelKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findModelKeyframeAt,
      IModelKeyframe*(int index));
  MOCK_CONST_METHOD3(findMorphKeyframe,
      IMorphKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IString *name, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findMorphKeyframeAt,
      IMorphKeyframe*(int index));
  MOCK_CONST_METHOD2(findProjectKeyframe,
      IProjectKeyframe*(const IKeyframe::TimeIndex &timeIndex, const IKeyframe::LayerIndex &layerIndex));
  MOCK_CONST_METHOD1(findProjectKeyframeAt,
      IProjectKeyframe*(int index));
  MOCK_METHOD1(replaceKeyframe,
      void(IKeyframe *value));
  MOCK_METHOD1(deleteKeyframe,
      void(IKeyframe *&value));
  MOCK_METHOD1(update,
      void(IKeyframe::Type type));
  MOCK_CONST_METHOD0(clone,
      IMotion*());
  MOCK_CONST_METHOD0(isNullFrameEnabled,
      bool());
  MOCK_METHOD1(setNullFrameEnable,
      void(bool value));
  MOCK_CONST_METHOD0(name,
      const IString*());
  MOCK_CONST_METHOD0(type,
      Type());
};

}  // namespace vpvl2

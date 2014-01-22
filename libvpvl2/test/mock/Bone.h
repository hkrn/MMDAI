namespace vpvl2 {

class MockIBone : public IBone {
 public:
  MOCK_METHOD1(addEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(removeEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(getEventListenerRefs,
      void(Array<PropertyEventListener *> &value));
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_METHOD2(setName,
      void(const IString *value, IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(parentBoneRef,
      IBone*());
  MOCK_CONST_METHOD0(effectorBoneRef,
      IBone*());
  MOCK_CONST_METHOD0(worldTransform,
      Transform());
  MOCK_CONST_METHOD0(localTransform,
      Transform());
  MOCK_CONST_METHOD1(getLocalTransform,
      void(Transform &world2LocalTransform));
  MOCK_METHOD1(setLocalTransform,
      void(const Transform &value));
  MOCK_CONST_METHOD0(origin,
      Vector3());
  MOCK_CONST_METHOD0(destinationOrigin,
      Vector3());
  MOCK_CONST_METHOD0(localTranslation,
      Vector3());
  MOCK_CONST_METHOD0(localOrientation,
      Quaternion());
  MOCK_CONST_METHOD1(getEffectorBones,
      void(Array<IBone *> &value));
  MOCK_METHOD1(setLocalTranslation,
      void(const Vector3 &value));
  MOCK_METHOD1(setLocalOrientation,
      void(const Quaternion &value));
  MOCK_CONST_METHOD0(isMovable,
      bool());
  MOCK_CONST_METHOD0(isRotateable,
      bool());
  MOCK_CONST_METHOD0(isVisible,
      bool());
  MOCK_CONST_METHOD0(isInteractive,
      bool());
  MOCK_CONST_METHOD0(hasInverseKinematics,
      bool());
  MOCK_CONST_METHOD0(hasFixedAxes,
      bool());
  MOCK_CONST_METHOD0(hasLocalAxes,
      bool());
  MOCK_CONST_METHOD0(fixedAxis,
      Vector3());
  MOCK_CONST_METHOD1(getLocalAxes,
      void(Matrix3x3 &value));
  MOCK_METHOD1(setInverseKinematicsEnable,
      void(bool value));
  MOCK_CONST_METHOD0(isInverseKinematicsEnabled,
      bool());
  MOCK_CONST_METHOD0(isInherentTranslationEnabled,
      bool());
  MOCK_CONST_METHOD0(isInherentOrientationEnabled,
      bool());
  MOCK_CONST_METHOD0(coefficient,
      float32());
  MOCK_METHOD1(setParentBoneRef,
      void(IBone *value));
  MOCK_METHOD2(setParentInherentBoneRef,
      void(IBone *value, float32 coefficient));
  MOCK_METHOD3(setEffectorBoneRef,
      void(IBone *effector, int numIteration, float angleLimit));
  MOCK_METHOD1(setDestinationOriginBoneRef,
      void(IBone *value));
  MOCK_METHOD1(setOrigin,
      void(const Vector3 &value));
  MOCK_METHOD1(setDestinationOrigin,
      void(const Vector3 &value));
  MOCK_METHOD1(setFixedAxis,
      void(const Vector3 &value));
  MOCK_METHOD1(setAxisX,
      void(const Vector3 &value));
  MOCK_METHOD1(setAxisZ,
      void(const Vector3 &value));
  MOCK_METHOD1(setLayerIndex,
      void(int value));
  MOCK_METHOD1(setExternalIndex,
      void(int value));
  MOCK_METHOD1(setRotateable,
      void(bool value));
  MOCK_METHOD1(setMovable,
      void(bool value));
  MOCK_METHOD1(setVisible,
      void(bool value));
  MOCK_METHOD1(setInteractive,
      void(bool value));
  MOCK_METHOD1(setHasInverseKinematics,
      void(bool value));
  MOCK_METHOD1(setInherentOrientationEnable,
      void(bool value));
  MOCK_METHOD1(setInherentTranslationEnable,
      void(bool value));
  MOCK_METHOD1(setAxisFixedEnable,
      void(bool value));
  MOCK_METHOD1(setLocalAxesEnable,
      void(bool value));
  MOCK_METHOD1(setTransformAfterPhysicsEnable,
      void(bool value));
  MOCK_METHOD1(setTransformedByExternalParentEnable,
      void(bool value));
};

}  // namespace vpvl2

namespace vpvl2 {

class MockIJoint : public IJoint {
 public:
  MOCK_CONST_METHOD0(constraintPtr,
      void*());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(rigidBody1Ref,
      IRigidBody*());
  MOCK_CONST_METHOD0(rigidBody2Ref,
      IRigidBody*());
  MOCK_CONST_METHOD0(rigidBodyIndex1,
      int());
  MOCK_CONST_METHOD0(rigidBodyIndex2,
      int());
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(position,
      Vector3());
  MOCK_CONST_METHOD0(rotation,
      Vector3());
  MOCK_CONST_METHOD0(positionLowerLimit,
      Vector3());
  MOCK_CONST_METHOD0(positionUpperLimit,
      Vector3());
  MOCK_CONST_METHOD0(rotationLowerLimit,
      Vector3());
  MOCK_CONST_METHOD0(rotationUpperLimit,
      Vector3());
  MOCK_CONST_METHOD0(positionStiffness,
      Vector3());
  MOCK_CONST_METHOD0(rotationStiffness,
      Vector3());
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_METHOD1(setRigidBody1Ref,
      void(IRigidBody *value));
  MOCK_METHOD1(setRigidBody2Ref,
      void(IRigidBody *value));
  MOCK_METHOD1(setName,
      void(const IString *value));
  MOCK_METHOD1(setEnglishName,
      void(const IString *value));
  MOCK_METHOD1(setPosition,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotation,
      void(const Vector3 &value));
  MOCK_METHOD1(setPositionLowerLimit,
      void(const Vector3 &value));
  MOCK_METHOD1(setPositionUpperLimit,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotationLowerLimit,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotationUpperLimit,
      void(const Vector3 &value));
  MOCK_METHOD1(setPositionStiffness,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotationStiffness,
      void(const Vector3 &value));
};

}  // namespace vpvl2

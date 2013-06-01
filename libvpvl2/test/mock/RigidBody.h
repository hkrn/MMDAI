namespace vpvl2 {

class MockIRigidBody : public IRigidBody {
 public:
  MOCK_METHOD0(syncLocalTransform,
      void());
  MOCK_METHOD1(joinWorld,
      void(void *value));
  MOCK_METHOD1(leaveWorld,
      void(void *value));
  MOCK_METHOD2(setKinematic,
      void(bool value, const Vector3 &basePosition));
  MOCK_CONST_METHOD0(createTransform,
      const Transform());
  MOCK_CONST_METHOD0(bodyPtr,
      void*());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(boneRef,
      IBone*());
  MOCK_CONST_METHOD0(name,
      const IString*());
  MOCK_CONST_METHOD0(englishName,
      const IString*());
  MOCK_CONST_METHOD0(size,
      Vector3());
  MOCK_CONST_METHOD0(position,
      Vector3());
  MOCK_CONST_METHOD0(rotation,
      Vector3());
  MOCK_CONST_METHOD0(mass,
      float32_t());
  MOCK_CONST_METHOD0(linearDamping,
      float32_t());
  MOCK_CONST_METHOD0(angularDamping,
      float32_t());
  MOCK_CONST_METHOD0(restitution,
      float32_t());
  MOCK_CONST_METHOD0(friction,
      float32_t());
  MOCK_CONST_METHOD0(groupID,
      uint16_t());
  MOCK_CONST_METHOD0(collisionGroupMask,
      uint16_t());
  MOCK_CONST_METHOD0(collisionGroupID,
      uint8_t());
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_METHOD1(setName,
      void(const IString *value));
  MOCK_METHOD1(setEnglishName,
      void(const IString *value));
  MOCK_METHOD1(setBoneRef,
      void(IBone *value));
  MOCK_METHOD1(setAngularDamping,
      void(float32_t value));
  MOCK_METHOD1(setCollisionGroupID,
      void(uint16_t value));
  MOCK_METHOD1(setCollisionMask,
      void(uint16_t value));
  MOCK_METHOD1(setFriction,
      void(float32_t value));
  MOCK_METHOD1(setLinearDamping,
      void(float32_t value));
  MOCK_METHOD1(setMass,
      void(float32_t value));
  MOCK_METHOD1(setPosition,
      void(const Vector3 &value));
  MOCK_METHOD1(setRestitution,
      void(float32_t value));
  MOCK_METHOD1(setRotation,
      void(const Vector3 &value));
  MOCK_METHOD1(setShapeType,
      void(ShapeType value));
  MOCK_METHOD1(setSize,
      void(const Vector3 &value));
  MOCK_METHOD1(setType,
      void(ObjectType value));
  MOCK_METHOD1(setIndex,
      void(int value));
};

}  // namespace vpvl2

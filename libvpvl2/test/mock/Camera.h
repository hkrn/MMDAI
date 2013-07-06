namespace vpvl2 {

class MockICamera : public ICamera {
 public:
  MOCK_METHOD1(addEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(removeEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(getEventListenerRefs,
      void(Array<PropertyEventListener *> &value));
  MOCK_CONST_METHOD0(modelViewTransform,
      Transform());
  MOCK_CONST_METHOD0(lookAt,
      Vector3());
  MOCK_CONST_METHOD0(position,
      Vector3());
  MOCK_CONST_METHOD0(angle,
      Vector3());
  MOCK_CONST_METHOD0(fov,
      Scalar());
  MOCK_CONST_METHOD0(distance,
      Scalar());
  MOCK_CONST_METHOD0(znear,
      Scalar());
  MOCK_CONST_METHOD0(zfar,
      Scalar());
  MOCK_CONST_METHOD0(motion,
      IMotion*());
  MOCK_METHOD1(setLookAt,
      void(const Vector3 &value));
  MOCK_METHOD1(setAngle,
      void(const Vector3 &value));
  MOCK_METHOD1(setFov,
      void(Scalar value));
  MOCK_METHOD1(setDistance,
      void(Scalar value));
  MOCK_METHOD1(setZNear,
      void(Scalar value));
  MOCK_METHOD1(setZFar,
      void(Scalar value));
  MOCK_METHOD1(setMotion,
      void(IMotion *value));
  MOCK_METHOD1(copyFrom,
      void(const ICamera *value));
  MOCK_METHOD0(resetDefault,
      void());
};

}  // namespace vpvl2

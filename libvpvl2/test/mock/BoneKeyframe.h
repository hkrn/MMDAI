namespace vpvl2 {

class MockIBoneKeyframe : public IBoneKeyframe {
 public:
  MOCK_CONST_METHOD0(clone,
      IBoneKeyframe*());
  MOCK_METHOD0(setDefaultInterpolationParameter,
      void());
  MOCK_METHOD2(setInterpolationParameter,
      void(InterpolationType type, const QuadWord &value));
  MOCK_CONST_METHOD2(getInterpolationParameter,
      void(InterpolationType type, QuadWord &value));
  MOCK_CONST_METHOD0(localTranslation,
      Vector3());
  MOCK_CONST_METHOD0(localOrientation,
      Quaternion());
  MOCK_METHOD1(setLocalTranslation,
      void(const Vector3 &value));
  MOCK_METHOD1(setLocalOrientation,
      void(const Quaternion &value));
};

}  // namespace vpvl2

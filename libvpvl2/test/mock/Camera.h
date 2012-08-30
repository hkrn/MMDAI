#include <vpvl2/ICamera.h>

namespace vpvl2 {

class MockICamera : public ICamera {
 public:
  MOCK_CONST_METHOD0(modelViewTransform,
      const Transform&());
  MOCK_CONST_METHOD0(position,
      const Vector3&());
  MOCK_CONST_METHOD0(angle,
      const Vector3&());
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
  MOCK_METHOD1(setPosition,
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
      void(ICamera *value));
  MOCK_METHOD0(resetDefault,
      void());
};

}  // namespace vpvl2

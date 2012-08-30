#include <vpvl2/IBone.h>

namespace vpvl2 {

class MockIBone : public IBone {
 public:
  MOCK_CONST_METHOD0(name,
      const IString*());
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_CONST_METHOD0(parentBone,
      IBone*());
  MOCK_CONST_METHOD0(targetBone,
      IBone*());
  MOCK_CONST_METHOD0(worldTransform,
      const Transform&());
  MOCK_CONST_METHOD0(origin,
      const Vector3&());
  MOCK_CONST_METHOD0(destinationOrigin,
      const Vector3());
  MOCK_CONST_METHOD0(position,
      const Vector3&());
  MOCK_CONST_METHOD0(rotation,
      const Quaternion&());
  MOCK_CONST_METHOD1(getLinkedBones,
      void(Array<IBone *> &value));
  MOCK_METHOD1(setPosition,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotation,
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
      const Vector3&());
  MOCK_CONST_METHOD1(getLocalAxes,
      void(Matrix3x3 &value));
};

}  // namespace vpvl2

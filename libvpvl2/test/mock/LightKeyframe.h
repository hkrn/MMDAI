#include <vpvl2/ILightKeyframe.h>

namespace vpvl2 {

class MockILightKeyframe : public ILightKeyframe {
 public:
  MOCK_CONST_METHOD0(clone,
      ILightKeyframe*());
  MOCK_CONST_METHOD0(color,
      const Vector3&());
  MOCK_CONST_METHOD0(direction,
      const Vector3&());
  MOCK_METHOD1(setColor,
      void(const Vector3 &value));
  MOCK_METHOD1(setDirection,
      void(const Vector3 &value));
};

}  // namespace vpvl2

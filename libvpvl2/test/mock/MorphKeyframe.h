#include <vpvl2/IMorphKeyframe.h>

namespace vpvl2 {

class MockIMorphKeyframe : public IMorphKeyframe {
 public:
  MOCK_CONST_METHOD0(clone,
      IMorphKeyframe*());
  MOCK_CONST_METHOD0(weight,
      const IMorph::WeightPrecision&());
  MOCK_METHOD1(setWeight,
      void(const IMorph::WeightPrecision &value));
};

}  // namespace vpvl2

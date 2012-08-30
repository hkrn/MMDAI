#include <vpvl2/IEffect.h>

namespace vpvl2 {

class MockIEffect : public IEffect {
 public:
  MOCK_CONST_METHOD0(internalContext,
      void*());
  MOCK_CONST_METHOD0(internalPointer,
      void*());
  MOCK_CONST_METHOD1(getOffscreenRenderTargets,
      void(Array<OffscreenRenderTarget> &value));
  MOCK_CONST_METHOD1(getInteractiveParameters,
      void(Array<void *> &value));
  MOCK_CONST_METHOD0(parentEffect,
      IEffect*());
  MOCK_METHOD1(setParentEffect,
      void(IEffect *value));
};

}  // namespace vpvl2

#include <vpvl2/ILight.h>

namespace vpvl2 {

class MockILight : public ILight {
 public:
  MOCK_CONST_METHOD0(color,
      const Vector3&());
  MOCK_CONST_METHOD0(direction,
      const Vector3&());
  MOCK_CONST_METHOD0(depthTextureSize,
      const Vector3&());
  MOCK_CONST_METHOD0(depthTexture,
      void*());
  MOCK_CONST_METHOD0(hasFloatTexture,
      bool());
  MOCK_CONST_METHOD0(isToonEnabled,
      bool());
  MOCK_CONST_METHOD0(isSoftShadowEnabled,
      bool());
  MOCK_CONST_METHOD0(motion,
      IMotion*());
  MOCK_METHOD1(setColor,
      void(const Vector3 &value));
  MOCK_METHOD1(setDirection,
      void(const Vector3 &value));
  MOCK_METHOD1(setDepthTextureSize,
      void(const Vector3 &value));
  MOCK_METHOD1(setMotion,
      void(IMotion *value));
  MOCK_METHOD1(setDepthTexture,
      void(void *value));
  MOCK_METHOD1(setHasFloatTexture,
      void(bool value));
  MOCK_METHOD1(setToonEnable,
      void(bool value));
  MOCK_METHOD1(setSoftShadowEnable,
      void(bool value));
  MOCK_METHOD1(copyFrom,
      void(ILight *value));
  MOCK_METHOD0(resetDefault,
      void());
};

}  // namespace vpvl2

namespace vpvl2 {

class MockIVertex : public IVertex {
 public:
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD2(performSkinning,
      void(Vector3 &position, Vector3 &normal));
  MOCK_METHOD0(reset,
      void());
  MOCK_CONST_METHOD0(origin,
      Vector3());
  MOCK_CONST_METHOD0(normal,
      Vector3());
  MOCK_CONST_METHOD0(textureCoord,
      Vector3());
  MOCK_CONST_METHOD1(uv,
      Vector4(int index));
  MOCK_CONST_METHOD0(delta,
      Vector3());
  MOCK_CONST_METHOD0(type,
      Type());
  MOCK_CONST_METHOD0(edgeSize,
      EdgeSizePrecision());
  MOCK_CONST_METHOD1(weight,
      WeightPrecision(int index));
  MOCK_CONST_METHOD1(boneRef,
      IBone*(int index));
  MOCK_CONST_METHOD0(materialRef,
      IMaterial*());
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_METHOD1(setOrigin,
      void(const Vector3 &value));
  MOCK_METHOD1(setNormal,
      void(const Vector3 &value));
  MOCK_METHOD1(setTextureCoord,
      void(const Vector3 &value));
  MOCK_METHOD2(setUV,
      void(int index, const Vector4 &value));
  MOCK_METHOD1(setType,
      void(Type value));
  MOCK_METHOD1(setEdgeSize,
      void(const EdgeSizePrecision &value));
  MOCK_METHOD2(setWeight,
      void(int index, const WeightPrecision &weight));
  MOCK_METHOD2(setBoneRef,
      void(int index, IBone *value));
  MOCK_METHOD1(setMaterialRef,
      void(IMaterial *value));
};

}  // namespace vpvl2

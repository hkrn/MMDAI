namespace vpvl2 {

class MockIMaterial : public IMaterial {
 public:
  MOCK_METHOD1(addEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(removeEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(getEventListenerRefs,
      void(Array<PropertyEventListener *> &value));
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(userDataArea,
      const IString*());
  MOCK_CONST_METHOD0(mainTexture,
      const IString*());
  MOCK_CONST_METHOD0(sphereTexture,
      const IString*());
  MOCK_CONST_METHOD0(toonTexture,
      const IString*());
  MOCK_CONST_METHOD0(sphereTextureRenderMode,
      SphereTextureRenderMode());
  MOCK_CONST_METHOD0(ambient,
      Color());
  MOCK_CONST_METHOD0(diffuse,
      Color());
  MOCK_CONST_METHOD0(specular,
      Color());
  MOCK_CONST_METHOD0(edgeColor,
      Color());
  MOCK_CONST_METHOD0(mainTextureBlend,
      Color());
  MOCK_CONST_METHOD0(sphereTextureBlend,
      Color());
  MOCK_CONST_METHOD0(toonTextureBlend,
      Color());
  MOCK_CONST_METHOD0(indexRange,
      IndexRange());
  MOCK_CONST_METHOD0(shininess,
      float32());
  MOCK_CONST_METHOD0(edgeSize,
      IVertex::EdgeSizePrecision());
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_CONST_METHOD0(mainTextureIndex,
      int());
  MOCK_CONST_METHOD0(sphereTextureIndex,
      int());
  MOCK_CONST_METHOD0(toonTextureIndex,
      int());
  MOCK_CONST_METHOD0(isSharedToonTextureUsed,
      bool());
  MOCK_CONST_METHOD0(isCullingDisabled,
      bool());
  MOCK_CONST_METHOD0(isCastingShadowEnabled,
      bool());
  MOCK_CONST_METHOD0(isCastingShadowMapEnabled,
      bool());
  MOCK_CONST_METHOD0(isShadowMapEnabled,
      bool());
  MOCK_CONST_METHOD0(isEdgeEnabled,
      bool());
  MOCK_CONST_METHOD0(isVertexColorEnabled,
      bool());
  MOCK_METHOD2(setName,
      void(const IString *value, IEncoding::LanguageType type));
  MOCK_METHOD1(setUserDataArea,
      void(const IString *value));
  MOCK_METHOD1(setMainTexture,
      void(const IString *value));
  MOCK_METHOD1(setSphereTexture,
      void(const IString *value));
  MOCK_METHOD1(setToonTexture,
      void(const IString *value));
  MOCK_METHOD1(setSphereTextureRenderMode,
      void(SphereTextureRenderMode value));
  MOCK_METHOD1(setAmbient,
      void(const Color &value));
  MOCK_METHOD1(setDiffuse,
      void(const Color &value));
  MOCK_METHOD1(setSpecular,
      void(const Color &value));
  MOCK_METHOD1(setEdgeColor,
      void(const Color &value));
  MOCK_METHOD1(setIndexRange,
      void(const IndexRange &value));
  MOCK_METHOD1(setShininess,
      void(float32 value));
  MOCK_METHOD1(setEdgeSize,
      void(const IVertex::EdgeSizePrecision &value));
  MOCK_METHOD1(setMainTextureIndex,
      void(int value));
  MOCK_METHOD1(setSphereTextureIndex,
      void(int value));
  MOCK_METHOD1(setToonTextureIndex,
      void(int value));
  MOCK_METHOD1(setSharedToonTextureUsed,
      void(bool value));
  MOCK_METHOD1(setCullingDisabled,
      void(bool value));
  MOCK_METHOD1(setCastingShadowEnabled,
      void(bool value));
  MOCK_METHOD1(setCastingShadowMapEnabled,
      void(bool value));
  MOCK_METHOD1(setShadowMapEnabled,
      void(bool value));
  MOCK_METHOD1(setEdgeEnabled,
      void(bool value));
  MOCK_METHOD1(setVertexColorEnabled,
      void(bool value));
  MOCK_METHOD1(setFlags,
      void(int value));
};

}  // namespace vpvl2

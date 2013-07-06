namespace vpvl2 {

class MockIMorph : public IMorph {
 public:
  MOCK_METHOD1(addEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(removeEventListenerRef,
      void(PropertyEventListener *value));
  MOCK_METHOD1(getEventListenerRefs,
      void(Array<PropertyEventListener *> &value));
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(index,
      int());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(category,
      Category());
  MOCK_CONST_METHOD0(type,
      Type());
  MOCK_CONST_METHOD0(hasParent,
      bool());
  MOCK_CONST_METHOD0(weight,
      WeightPrecision());
  MOCK_METHOD1(setWeight,
      void(const WeightPrecision &value));
};

}  // namespace vpvl2

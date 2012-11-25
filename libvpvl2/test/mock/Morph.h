namespace vpvl2 {

class MockIMorph : public IMorph {
 public:
  MOCK_CONST_METHOD0(name,
      const IString*());
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

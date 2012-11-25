namespace vpvl2 {

class MockIMorphKeyframe : public IMorphKeyframe {
 public:
  MOCK_CONST_METHOD0(clone,
      IMorphKeyframe*());
  MOCK_CONST_METHOD0(weight,
      IMorph::WeightPrecision());
  MOCK_METHOD1(setWeight,
      void(const IMorph::WeightPrecision &value));
};

}  // namespace vpvl2

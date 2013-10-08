namespace vpvl2 {

class MockILabel : public ILabel {
 public:
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(isSpecial,
      bool());
  MOCK_CONST_METHOD0(count,
      int());
  MOCK_CONST_METHOD1(boneRef,
      IBone*(int index));
  MOCK_CONST_METHOD1(morphRef,
      IMorph*(int index));
  MOCK_METHOD2(setName,
      void(const IString *value, IEncoding::LanguageType type));
  MOCK_METHOD1(setSpecial,
      void(bool value));
  MOCK_METHOD1(addBoneRef,
      void(IBone *value));
  MOCK_METHOD1(addMorphRef,
      void(IMorph *value));
  MOCK_METHOD1(removeBoneRef,
      void(IBone *value));
  MOCK_METHOD1(removeMorphRef,
      void(IMorph *value));
  MOCK_CONST_METHOD0(index,
      int());
};

}  // namespace vpvl2

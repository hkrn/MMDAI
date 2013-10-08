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
  MOCK_METHOD2(setName,
      void(const IString *value, IEncoding::LanguageType type));
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
  MOCK_METHOD0(markDirty,
      void());
  MOCK_METHOD1(addBoneMorph,
      void(Bone *value));
  MOCK_METHOD1(removeBoneMorph,
      void(Bone *value));
  MOCK_METHOD1(addGroupMorph,
      void(Group *value));
  MOCK_METHOD1(removeGroupMorph,
      void(Group *value));
  MOCK_METHOD1(addMaterialMorph,
      void(Material *value));
  MOCK_METHOD1(removeMaterialMorph,
      void(Material *value));
  MOCK_METHOD1(addUVMorph,
      void(UV *value));
  MOCK_METHOD1(removeUVMorph,
      void(UV *value));
  MOCK_METHOD1(addVertexMorph,
      void(Vertex *value));
  MOCK_METHOD1(removeVertexMorph,
      void(Vertex *value));
  MOCK_METHOD1(addFlipMorph,
      void(Flip *value));
  MOCK_METHOD1(removeFlipMorph,
      void(Flip *value));
  MOCK_METHOD1(addImpulseMorph,
      void(Impulse *value));
  MOCK_METHOD1(removeImpulseMorph,
      void(Impulse *value));
  MOCK_METHOD1(setType,
      void(Type value));
  MOCK_CONST_METHOD1(getBoneMorphs,
      void(Array<Bone *> &morphs));
  MOCK_CONST_METHOD1(getGroupMorphs,
      void(Array<Group *> &morphs));
  MOCK_CONST_METHOD1(getMaterialMorphs,
      void(Array<Material *> &morphs));
  MOCK_CONST_METHOD1(getUVMorphs,
      void(Array<UV *> &morphs));
  MOCK_CONST_METHOD1(getVertexMorphs,
      void(Array<Vertex *> &morphs));
  MOCK_CONST_METHOD1(getFlipMorphs,
      void(Array<Flip *> &morphs));
  MOCK_CONST_METHOD1(getImpulseMorphs,
      void(Array<Impulse *> &morphs));
};

}  // namespace vpvl2

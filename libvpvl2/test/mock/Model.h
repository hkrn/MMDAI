namespace vpvl2 {

class MockIModel : public IModel {
 public:
  MOCK_CONST_METHOD0(type,
      Type());
  MOCK_CONST_METHOD0(name,
      const IString*());
  MOCK_CONST_METHOD0(englishName,
      const IString*());
  MOCK_CONST_METHOD0(comment,
      const IString*());
  MOCK_CONST_METHOD0(englishComment,
      const IString*());
  MOCK_CONST_METHOD0(isVisible,
      bool());
  MOCK_CONST_METHOD0(error,
      ErrorType());
  MOCK_METHOD2(load,
      bool(const uint8_t *data, size_t size));
  MOCK_CONST_METHOD2(save,
      void(uint8_t *data, size_t &written));
  MOCK_CONST_METHOD0(estimateSize,
      size_t());
  MOCK_METHOD0(resetVertices,
      void());
  MOCK_METHOD1(joinWorld,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD1(leaveWorld,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD1(resetMotionState,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD0(performUpdate,
      void());
  MOCK_CONST_METHOD1(findBone,
      IBone*(const IString *value));
  MOCK_CONST_METHOD1(findMorph,
      IMorph*(const IString *value));
  MOCK_CONST_METHOD1(count,
      int(ObjectType value));
  MOCK_CONST_METHOD1(getBoneRefs,
      void(Array<IBone *> &value));
  MOCK_CONST_METHOD1(getLabelRefs,
      void(Array<ILabel *> &value));
  MOCK_CONST_METHOD1(getMaterialRefs,
      void(Array<IMaterial *> &value));
  MOCK_CONST_METHOD1(getMorphRefs,
      void(Array<IMorph *> &value));
  MOCK_CONST_METHOD1(getVertexRefs,
      void(Array<IVertex *> &value));
  MOCK_CONST_METHOD1(edgeScaleFactor,
      float(const Vector3 &cameraPosition));
  MOCK_CONST_METHOD0(worldPosition,
      Vector3());
  MOCK_CONST_METHOD0(worldRotation,
      Quaternion());
  MOCK_CONST_METHOD0(opacity,
      Scalar());
  MOCK_CONST_METHOD0(scaleFactor,
      Scalar());
  MOCK_CONST_METHOD0(edgeColor,
      Vector3());
  MOCK_CONST_METHOD0(edgeWidth,
      Scalar());
  MOCK_CONST_METHOD0(parentSceneRef,
      Scene*());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(parentBoneRef,
      IBone*());
  MOCK_METHOD1(setName,
      void(const IString *value));
  MOCK_METHOD1(setEnglishName,
      void(const IString *value));
  MOCK_METHOD1(setComment,
      void(const IString *value));
  MOCK_METHOD1(setEnglishComment,
      void(const IString *value));
  MOCK_METHOD1(setWorldPosition,
      void(const Vector3 &value));
  MOCK_METHOD1(setWorldRotation,
      void(const Quaternion &value));
  MOCK_METHOD1(setOpacity,
      void(const Scalar &value));
  MOCK_METHOD1(setScaleFactor,
      void(const Scalar &value));
  MOCK_METHOD1(setEdgeColor,
      void(const Vector3 &value));
  MOCK_METHOD1(setEdgeWidth,
      void(const Scalar &value));
  MOCK_METHOD1(setParentModelRef,
      void(IModel *value));
  MOCK_METHOD1(setParentBoneRef,
      void(IBone *value));
  MOCK_METHOD1(setVisible,
      void(bool value));
  MOCK_CONST_METHOD0(isPhysicsEnabled,
      bool());
  MOCK_METHOD1(setPhysicsEnable,
      void(bool value));
  MOCK_CONST_METHOD1(getIndexBuffer,
      void(IIndexBuffer *&indexBuffer));
  MOCK_CONST_METHOD1(getStaticVertexBuffer,
      void(IStaticVertexBuffer *&staticBuffer));
  MOCK_CONST_METHOD2(getDynamicVertexBuffer,
      void(IDynamicVertexBuffer *&dynamicBuffer, const IIndexBuffer *indexBuffer));
  MOCK_CONST_METHOD3(getMatrixBuffer,
      void(IMatrixBuffer *&matrixBuffer, IDynamicVertexBuffer *dynamicBuffer, const IIndexBuffer *indexBuffer));
  MOCK_METHOD2(setAabb,
      void(const Vector3 &min, const Vector3 &max));
  MOCK_CONST_METHOD2(getAabb,
      void(Vector3 &min, Vector3 &max));
  MOCK_METHOD0(createBone,
      IBone*());
  MOCK_METHOD0(createLabel,
      ILabel*());
  MOCK_METHOD0(createMaterial,
      IMaterial*());
  MOCK_METHOD0(createMorph,
      IMorph*());
  MOCK_METHOD0(createVertex,
      IVertex*());
  MOCK_CONST_METHOD1(findBoneAt,
      IBone*(int value));
  MOCK_CONST_METHOD1(findLabelAt,
      ILabel*(int value));
  MOCK_CONST_METHOD1(findMaterialAt,
      IMaterial*(int value));
  MOCK_CONST_METHOD1(findMorphAt,
      IMorph*(int value));
  MOCK_CONST_METHOD1(findVertexAt,
      IVertex*(int value));
  MOCK_METHOD1(addBone,
      void(IBone *value));
  MOCK_METHOD1(addLabel,
      void(ILabel *value));
  MOCK_METHOD1(addMaterial,
      void(IMaterial *value));
  MOCK_METHOD1(addMorph,
      void(IMorph *value));
  MOCK_METHOD1(addVertex,
      void(IVertex *value));
  MOCK_METHOD1(removeBone,
      void(IBone *value));
  MOCK_METHOD1(removeLabel,
      void(ILabel *value));
  MOCK_METHOD1(removeMaterial,
      void(IMaterial *value));
  MOCK_METHOD1(removeMorph,
      void(IMorph *value));
  MOCK_METHOD1(removeVertex,
      void(IVertex *value));
};

}  // namespace vpvl2

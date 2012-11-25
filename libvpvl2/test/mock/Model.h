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
  MOCK_CONST_METHOD1(save,
      void(uint8_t *data));
  MOCK_CONST_METHOD0(estimateSize,
      size_t());
  MOCK_METHOD0(resetVertices,
      void());
  MOCK_METHOD0(resetMotionState,
      void());
  MOCK_METHOD0(performUpdate,
      void());
  MOCK_METHOD1(joinWorld,
      void(btDiscreteDynamicsWorld *world));
  MOCK_METHOD1(leaveWorld,
      void(btDiscreteDynamicsWorld *world));
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
  MOCK_CONST_METHOD0(parentModel,
      IModel*());
  MOCK_CONST_METHOD0(parentBone,
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
  MOCK_METHOD1(setParentModel,
      void(IModel *value));
  MOCK_METHOD1(setParentBone,
      void(IBone *value));
  MOCK_METHOD1(setVisible,
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
};

}  // namespace vpvl2

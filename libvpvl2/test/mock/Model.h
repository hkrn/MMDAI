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
  MOCK_METHOD2(performUpdate,
      void(const Vector3 &cameraPosition, const Vector3 &lightDirection));
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
  MOCK_CONST_METHOD1(getBones,
      void(Array<IBone *> &value));
  MOCK_CONST_METHOD1(getMorphs,
      void(Array<IMorph *> &value));
  MOCK_CONST_METHOD1(getLabels,
      void(Array<ILabel *> &value));
  MOCK_CONST_METHOD2(getBoundingBox,
      void(Vector3 &min, Vector3 &max));
  MOCK_CONST_METHOD2(getBoundingSphere,
      void(Vector3 &center, Scalar &radius));
  MOCK_CONST_METHOD0(indexType,
      IndexType());
  MOCK_CONST_METHOD0(position,
      const Vector3&());
  MOCK_CONST_METHOD0(rotation,
      const Quaternion&());
  MOCK_CONST_METHOD0(opacity,
      const Scalar&());
  MOCK_CONST_METHOD0(scaleFactor,
      const Scalar&());
  MOCK_CONST_METHOD0(edgeColor,
      const Vector3&());
  MOCK_CONST_METHOD0(edgeWidth,
      const Scalar&());
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
  MOCK_METHOD1(setPosition,
      void(const Vector3 &value));
  MOCK_METHOD1(setRotation,
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
};

}  // namespace vpvl2

namespace vpvl2 {

class MockIModel : public IModel {
 public:
  MOCK_CONST_METHOD0(type,
      Type());
  MOCK_CONST_METHOD1(name,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD1(comment,
      const IString*(IEncoding::LanguageType type));
  MOCK_CONST_METHOD0(isVisible,
      bool());
  MOCK_CONST_METHOD0(error,
      ErrorType());
  MOCK_METHOD2(load,
      bool(const uint8 *data, vsize size));
  MOCK_CONST_METHOD2(save,
      void(uint8 *data, vsize &written));
  MOCK_CONST_METHOD0(estimateSize,
      vsize());
  MOCK_METHOD1(joinWorld,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD1(leaveWorld,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD1(resetMotionState,
      void(btDiscreteDynamicsWorld *worldRef));
  MOCK_METHOD0(performUpdate,
      void());
  MOCK_CONST_METHOD1(findBoneRef,
      IBone*(const IString *value));
  MOCK_CONST_METHOD1(findMorphRef,
      IMorph*(const IString *value));
  MOCK_CONST_METHOD1(count,
      int(ObjectType value));
  MOCK_CONST_METHOD1(getBoneRefs,
      void(Array<IBone *> &value));
  MOCK_CONST_METHOD1(getJointRefs,
      void(Array<IJoint *> &value));
  MOCK_CONST_METHOD1(getLabelRefs,
      void(Array<ILabel *> &value));
  MOCK_CONST_METHOD1(getMaterialRefs,
      void(Array<IMaterial *> &value));
  MOCK_CONST_METHOD1(getMorphRefs,
      void(Array<IMorph *> &value));
  MOCK_CONST_METHOD1(getRigidBodyRefs,
      void(Array<IRigidBody *> &value));
  MOCK_CONST_METHOD1(getTextureRefs,
      void(Array<const IString *> &value));
  MOCK_CONST_METHOD1(getVertexRefs,
      void(Array<IVertex *> &value));
  MOCK_CONST_METHOD1(getIndices,
      void(Array<int> &value));
  MOCK_CONST_METHOD1(edgeScaleFactor,
      IVertex::EdgeSizePrecision(const Vector3 &cameraPosition));
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
      IVertex::EdgeSizePrecision());
  MOCK_CONST_METHOD0(parentSceneRef,
      Scene*());
  MOCK_CONST_METHOD0(parentModelRef,
      IModel*());
  MOCK_CONST_METHOD0(parentBoneRef,
      IBone*());
  MOCK_METHOD2(setName,
      void(const IString *value, IEncoding::LanguageType type));
  MOCK_METHOD2(setComment,
      void(const IString *value, IEncoding::LanguageType type));
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
      void(const IVertex::EdgeSizePrecision &value));
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
      void(IndexBuffer *&indexBuffer));
  MOCK_CONST_METHOD1(getStaticVertexBuffer,
      void(StaticVertexBuffer *&staticBuffer));
  MOCK_CONST_METHOD2(getDynamicVertexBuffer,
      void(DynamicVertexBuffer *&dynamicBuffer, const IndexBuffer *indexBuffer));
  MOCK_CONST_METHOD3(getMatrixBuffer,
      void(MatrixBuffer *&matrixBuffer, DynamicVertexBuffer *dynamicBuffer, const IndexBuffer *indexBuffer));
  MOCK_METHOD2(setAabb,
      void(const Vector3 &min, const Vector3 &max));
  MOCK_CONST_METHOD2(getAabb,
      void(Vector3 &min, Vector3 &max));
  MOCK_CONST_METHOD0(version,
      float32());
  MOCK_METHOD1(setVersion,
      void(float32 value));
  MOCK_METHOD0(createBone,
      IBone*());
  MOCK_METHOD0(createJoint,
      IJoint*());
  MOCK_METHOD0(createLabel,
      ILabel*());
  MOCK_METHOD0(createMaterial,
      IMaterial*());
  MOCK_METHOD0(createMorph,
      IMorph*());
  MOCK_METHOD0(createRigidBody,
      IRigidBody*());
  MOCK_METHOD0(createVertex,
      IVertex*());
  MOCK_CONST_METHOD1(findBoneRefAt,
      IBone*(int value));
  MOCK_CONST_METHOD1(findJointRefAt,
      IJoint*(int value));
  MOCK_CONST_METHOD1(findLabelRefAt,
      ILabel*(int value));
  MOCK_CONST_METHOD1(findMaterialRefAt,
      IMaterial*(int value));
  MOCK_CONST_METHOD1(findMorphRefAt,
      IMorph*(int value));
  MOCK_CONST_METHOD1(findRigidBodyRefAt,
      IRigidBody*(int value));
  MOCK_CONST_METHOD1(findVertexRefAt,
      IVertex*(int value));
  MOCK_METHOD1(setIndices,
      void(const Array<int> &value));
  MOCK_METHOD1(addBone,
      void(IBone *value));
  MOCK_METHOD1(addJoint,
      void(IJoint *value));
  MOCK_METHOD1(addLabel,
      void(ILabel *value));
  MOCK_METHOD1(addMaterial,
      void(IMaterial *value));
  MOCK_METHOD1(addMorph,
      void(IMorph *value));
  MOCK_METHOD1(addRigidBody,
      void(IRigidBody *value));
  MOCK_METHOD1(addVertex,
      void(IVertex *value));
  MOCK_METHOD1(removeBone,
      void(IBone *value));
  MOCK_METHOD1(removeJoint,
      void(IJoint *value));
  MOCK_METHOD1(removeLabel,
      void(ILabel *value));
  MOCK_METHOD1(removeMaterial,
      void(IMaterial *value));
  MOCK_METHOD1(removeMorph,
      void(IMorph *value));
  MOCK_METHOD1(removeRigidBody,
      void(IRigidBody *value));
  MOCK_METHOD1(removeVertex,
      void(IVertex *value));
};

}  // namespace vpvl2

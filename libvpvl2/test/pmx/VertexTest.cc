#include "Common.h"

TEST_P(PMXFragmentTest, ReadWriteVertexBdef1)
{
    vsize indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.append(&bone1);
    SetVertex(expected, Vertex::kBdef1, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(PMXFragmentTest, ReadWriteVertexBdef2)
{
    vsize indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.append(&bone1);
    bone2.setIndex(1);
    bones.append(&bone2);
    SetVertex(expected, Vertex::kBdef2, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(PMXFragmentTest, ReadWriteVertexBdef4)
{
    vsize indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0), bone3(0), bone4(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.append(&bone1);
    bone2.setIndex(1);
    bones.append(&bone2);
    bone3.setIndex(2);
    bones.append(&bone3);
    bone4.setIndex(3);
    bones.append(&bone4);
    SetVertex(expected, Vertex::kBdef4, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(PMXFragmentTest, ReadWriteVertexSdef)
{
    vsize indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.append(&bone1);
    bone2.setIndex(1);
    bones.append(&bone2);
    SetVertex(expected, Vertex::kSdef, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST(PMXVertexTest, Boundary)
{
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0));
    vertex.setOriginUV(-1, Vector4(1, 1, 1, 1));
    vertex.setOriginUV(4, Vector4(1, 1, 1, 1));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight(Vertex::kMaxBones, 0.1);
    vertex.setBoneRef(-1, bone.get());
    vertex.setBoneRef(Vertex::kMaxBones, bone.get());
    ASSERT_EQ(vertex.uv(-1).x(), 0.0f);
    ASSERT_EQ(vertex.uv(4).x(), 0.0f);
    ASSERT_EQ(vertex.boneRef(-1), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.boneRef(Vertex::kMaxBones), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.weight(-1), 0.0f);
    ASSERT_EQ(vertex.weight(Vertex::kMaxBones), 0.0f);
}

TEST(PMXVertexTest, NullRef)
{
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0));
    std::unique_ptr<Material> material(new Material(0));
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
    vertex.setBoneRef(0, bone.get());
    vertex.setBoneRef(0, 0);
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    vertex.setMaterialRef(material.get());
    vertex.setMaterialRef(0);
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
}

TEST(PMXVertexTest, PerformSkinningBdef1)
{
    pmx::Vertex v(0);
    MockIBone bone;
    Transform transform(Matrix3x3::getIdentity().scaled(Vector3(0.5, 0.5, 0.5)), Vector3(1, 2, 3));
    EXPECT_CALL(bone, localTransform()).Times(1).WillRepeatedly(Return(transform));
    EXPECT_CALL(bone, index()).Times(1).WillRepeatedly(Return(0));
    v.setType(pmx::Vertex::kBdef1);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    v.setBoneRef(0, &bone);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.05, 2.1, 3.15), position));
    ASSERT_TRUE(CompareVector(Vector3(0.2, 0.25, 0.3), normal));
}

TEST(PMXVertexTest, PerformSkinningBdef2WeightZero)
{
    pmx::Vertex v(0);
    MockIBone bone1, bone2;
    //Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    //EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform1));
    EXPECT_CALL(bone1, index()).Times(1).WillRepeatedly(Return(0));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    EXPECT_CALL(bone2, index()).Times(1).WillRepeatedly(Return(1));
    v.setType(pmx::Vertex::kBdef2);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    v.setBoneRef(0, &bone1);
    v.setBoneRef(1, &bone2);
    v.setWeight(0, 0);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(4.025, 5.05, 6.075), position));
    ASSERT_TRUE(CompareVector(Vector3(0.1, 0.125, 0.15), normal));
}

TEST(PMXVertexTest, PerformSkinningBdef2WeightOne)
{
    pmx::Vertex v(0);
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    EXPECT_CALL(bone1, index()).Times(1).WillRepeatedly(Return(0));
    //Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    //EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform2));
    EXPECT_CALL(bone2, index()).Times(1).WillRepeatedly(Return(1));
    v.setType(pmx::Vertex::kBdef2);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    v.setBoneRef(0, &bone1);
    v.setBoneRef(1, &bone2);
    v.setWeight(0, 1);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.075, 2.15, 3.225), position));
    ASSERT_TRUE(CompareVector(Vector3(0.3, 0.375, 0.45), normal));
}

TEST(PMXVertexTest, PerformSkinningBdef2WeightHalf)
{
    pmx::Vertex v(0);
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    EXPECT_CALL(bone1, index()).Times(1).WillRepeatedly(Return(0));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    EXPECT_CALL(bone2, index()).Times(1).WillRepeatedly(Return(1));
    v.setType(pmx::Vertex::kBdef2);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    v.setBoneRef(0, &bone1);
    v.setBoneRef(1, &bone2);
    v.setWeight(0, 0.5);
    v.setWeight(1, 0.5);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    const Vector3 &v2 = (Vector3(1.075, 2.15, 3.225) + Vector3(4.025, 5.05, 6.075)) * 0.5;
    const Vector3 &n2 = (Vector3(0.1, 0.125, 0.15) + Vector3(0.3, 0.375, 0.45)) * 0.5;
    ASSERT_TRUE(CompareVector(v2, position));
    ASSERT_TRUE(CompareVector(n2, normal));
}

TEST(PMXModelTest, AddAndRemoveVertex)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IVertex> vertex(model.createVertex());
    ASSERT_EQ(-1, vertex->index());
    model.addVertex(0); /* should not be crashed */
    model.addVertex(vertex.get());
    model.addVertex(vertex.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.vertices().count());
    ASSERT_EQ(vertex.get(), model.findVertexRefAt(0));
    ASSERT_EQ(vertex->index(), model.findVertexRefAt(0)->index());
    model.removeVertex(0); /* should not be crashed */
    model.removeVertex(vertex.get());
    ASSERT_EQ(0, model.vertices().count());
    ASSERT_EQ(-1, vertex->index());
    MockIVertex mockedVertex;
    EXPECT_CALL(mockedVertex, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedVertex, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addVertex(&mockedVertex);
    ASSERT_EQ(0, model.vertices().count());
}

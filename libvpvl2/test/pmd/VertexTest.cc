#include "Common.h"

TEST(PMDVertexTest, Boundary)
{
    Encoding encoding(0);
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0, &encoding));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight(Vertex::kMaxBones, 0.1);
    vertex.setBoneRef(-1, bone.get());
    vertex.setBoneRef(Vertex::kMaxBones, bone.get());
    ASSERT_EQ(vertex.boneRef(-1), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.boneRef(Vertex::kMaxBones), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.weight(-1), 0.0f);
    ASSERT_EQ(vertex.weight(Vertex::kMaxBones), 0.0f);
}

TEST(PMDVertexTest, NullRef)
{
    Encoding encoding(0);
    Vertex vertex(0);
    std::unique_ptr<Bone> bone(new Bone(0, &encoding));
    std::unique_ptr<Material> material(new Material(0, &encoding));
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
    vertex.setBoneRef(0, bone.get());
    vertex.setBoneRef(0, 0);
    ASSERT_EQ(vertex.boneRef(0), Factory::sharedNullBoneRef());
    vertex.setMaterialRef(material.get());
    vertex.setMaterialRef(0);
    ASSERT_EQ(vertex.materialRef(), Factory::sharedNullMaterialRef());
}


TEST(PMDPropertyEventListener, HandleVertexPropertyEvents)
{
    Vertex vertex(0);
    MockVertexPropertyEventListener listener;
    TestHandleEvents<IVertex::PropertyEventListener>(listener, vertex);
    Bone bone(0, 0);
    Material material(0, 0);
    IVertex::EdgeSizePrecision edgeSize(0.42);
    IVertex::WeightPrecision weightSize(0.84);
    IVertex::Type type(IVertex::kBdef4);
    Vector3 origin(1, 2, 3), normal(origin.normalized()), texcoord(0.4, 0.5, 0.6);
    Vector4 uv(0.6, 0.7, 0.8, 0.9);
    /* type/UV should not be called */
    EXPECT_CALL(listener, boneRefWillChange(0, &bone, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, edgeSizeWillChange(edgeSize, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, materialRefWillChange(&material, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, normalWillChange(normal, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, originWillChange(origin, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, textureCoordWillChange(texcoord, &vertex)).WillOnce(Return());
    EXPECT_CALL(listener, weightWillChange(0, weightSize, &vertex)).WillOnce(Return());
    vertex.addEventListenerRef(&listener);
    vertex.setBoneRef(0, &bone);
    vertex.setBoneRef(0, &bone);
    vertex.setEdgeSize(edgeSize);
    vertex.setEdgeSize(edgeSize);
    vertex.setMaterialRef(&material);
    vertex.setMaterialRef(&material);
    vertex.setNormal(normal);
    vertex.setNormal(normal);
    vertex.setOrigin(origin);
    vertex.setOrigin(origin);
    vertex.setTextureCoord(texcoord);
    vertex.setTextureCoord(texcoord);
    vertex.setType(type);
    vertex.setType(type);
    vertex.setOriginUV(0, uv);
    vertex.setOriginUV(0, uv);
    vertex.setWeight(0, weightSize);
    vertex.setWeight(0, weightSize);
}

TEST(PMDModelTest, AddAndRemoveVertex)
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

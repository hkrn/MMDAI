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

TEST(PMDModelTest, RemoveVertexReferences)
{
    Encoding encoding(0);
    Model model(&encoding);
    Vertex vertex(&model);
    model.addVertex(&vertex);
    Morph::Vertex vertexMorph;
    vertexMorph.vertex = &vertex;
    Morph parentVertexMorph(&model, &encoding);
    parentVertexMorph.setType(IMorph::kVertexMorph);
    parentVertexMorph.addVertexMorph(&vertexMorph);
    model.addMorph(&parentVertexMorph);
    model.removeVertex(&vertex);
    parentVertexMorph.removeVertexMorph(&vertexMorph);
    model.removeMorph(&parentVertexMorph);
    ASSERT_EQ(static_cast<IVertex *>(0), vertexMorph.vertex);
}

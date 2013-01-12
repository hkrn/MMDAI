#include "Common.h"

#include <btBulletDynamicsCommon.h>

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu/Encoding.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Vertex.h"
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

#include "mock/Bone.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::pmx;
using namespace vpvl2::extensions::icu;

namespace
{

static void SetVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones)
{
    vertex.setOrigin(Vector3(0.01, 0.02, 0.03));
    vertex.setNormal(Vector3(0.11, 0.12, 0.13));
    vertex.setTextureCoord(Vector3(0.21, 0.22, 0.0));
    vertex.setUV(0, Vector4(0.31, 0.32, 0.33, 0.34));
    vertex.setType(type);
    vertex.setEdgeSize(0.1);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vertex.setBone(i, bones[i]);
        vertex.setWeight(i, 0.2 + 0.1 * i);
    }
    vertex.setSdefC(Vector3(0.41, 0.42, 0.43));
    vertex.setSdefR0(Vector3(0.51, 0.52, 0.53));
    vertex.setSdefR1(Vector3(0.61, 0.62, 0.63));
}

class FragmentTest : public TestWithParam<size_t> {};

class FragmentWithUVTest : public TestWithParam< tuple<size_t, pmx::Morph::Type > > {};

}

TEST_P(FragmentTest, ReadWriteBone)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Bone bone(0), bone2(0), parent(0);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    // construct bone
    parent.setIndex(0);
    bone.setName(&name);
    bone.setEnglishName(&englishName);
    bone.setOrigin(Vector3(0.11, 0.12, 0.13));
    bone.setIndex(1);
    bone.setDestinationOrigin(Vector3(0.21, 0.22, 0.23));
    bone.setFixedAxis(Vector3(0.31, 0.32, 0.33));
    bone.setAxisX(Vector3(0.41, 0.42, 0.43));
    bone.setAxisZ(Vector3(0.51, 0.52, 0.53));
    bone.setExternalIndex(3);
    bone.setParentBone(&parent);
    bone.setParentInherenceBone(&parent, 0.61);
    bone.setTargetBone(&parent, 3, 0.71);
    bone.setRotateable(true);
    bone.setMovable(true);
    bone.setVisible(true);
    bone.setOperatable(true);
    bone.setIKEnable(true);
    bone.setPositionInherenceEnable(true);
    bone.setRotationInherenceEnable(true);
    bone.setAxisFixedEnable(true);
    bone.setLocalAxisEnable(true);
    bone.setTransformAfterPhysicsEnable(true);
    bone.setTransformedByExternalParentEnable(true);
    // write constructed bone and read it
    size_t size = bone.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    bone.write(data.data(), info);
    bone2.read(data.data(), info, read);
    // compare read bone
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareBone(bone, bone2));
    Array<Bone *> bones, apb, bpb;
    bones.add(&parent);
    bones.add(&bone2);
    Bone::loadBones(bones, bpb, apb);
    ASSERT_EQ(&parent, bone2.parentBoneRef());
    ASSERT_EQ(&parent, bone2.parentInherenceBoneRef());
    ASSERT_EQ(&parent, bone2.targetBoneRef());
}

TEST_P(FragmentTest, ReadWriteJoint)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Joint expected, actual;
    RigidBody body, body2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.rigidBodyIndexSize = indexSize;
    // construct joint
    body.setIndex(0);
    body2.setIndex(1);
    expected.setName(&name);
    expected.setEnglishName(&englishName);
    expected.setRigidBody1(&body);
    expected.setRigidBody2(&body2);
    expected.setPosition(Vector3(0.01, 0.02, 0.03));
    expected.setRotation(Vector3(0.11, 0.12, 0.13));
    expected.setPositionLowerLimit(Vector3(0.21, 0.22, 0.23));
    expected.setRotationLowerLimit(Vector3(0.31, 0.32, 0.33));
    expected.setPositionUpperLimit(Vector3(0.41, 0.42, 0.43));
    expected.setRotationUpperLimit(Vector3(0.51, 0.52, 0.53));
    expected.setPositionStiffness(Vector3(0.61, 0.62, 0.63));
    expected.setRotationStiffness(Vector3(0.71, 0.72, 0.73));
    // write constructed joint and read it
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareJoint(expected, actual, body, body2));
}

TEST_P(FragmentTest, ReadWriteMaterial)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Material expected(0), actual(0);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.textureIndexSize = indexSize;
    // construct material
    expected.setName(&name);
    expected.setEnglishName(&englishName);
    expected.setSphereTextureRenderMode(Material::kSubTexture);
    expected.setAmbient(Color(0.01, 0.02, 0.03, 1.0));
    expected.setDiffuse(Color(0.11, 0.12, 0.13, 0.14));
    expected.setSpecular(Color(0.21, 0.22, 0.23, 1.0));
    expected.setEdgeColor(Color(0.31, 0.32, 0.33, 0.34));
    expected.setShininess(0.1);
    expected.setEdgeSize(0.2);
    expected.setMainTextureIndex(1);
    expected.setSphereTextureIndex(2);
    expected.setToonTextureIndex(3);
    expected.setIndices(4);
    expected.setFlags(5);
    // write contructed material and read it
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    // compare read material
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareMaterialInterface(expected, actual));
}

TEST_P(FragmentTest, ReadWriteBoneMorph)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Morph morph(0), morph2(0);
    QScopedPointer<Morph::Bone> bone1(new Morph::Bone()), bone2(new Morph::Bone());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.boneIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // bone morph1
    bone1->index = 0;
    bone1->position.setValue(0.11, 0.12, 0.13);
    bone1->rotation.setValue(0.21, 0.22, 0.23, 0.24);
    morph.addBoneMorph(bone1.data());
    // bone morph2
    bone2->index = 1;
    bone2->position.setValue(0.31, 0.32, 0.33);
    bone2->rotation.setValue(0.41, 0.42, 0.43, 0.44);
    morph.addBoneMorph(bone2.data());
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kEyeblow);
    morph.setType(pmx::Morph::kBoneMorph);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(morph2.name()->equals(morph.name()));
    ASSERT_TRUE(morph2.englishName()->equals(morph.englishName()));
    ASSERT_EQ(morph.category(), morph2.category());
    ASSERT_EQ(morph.type(), morph2.type());
    const Array<Morph::Bone *> &bones = morph2.bones();
    ASSERT_EQ(bones.count(), 2);
    ASSERT_TRUE(CompareVector(bone1->position, bones[0]->position));
    ASSERT_TRUE(CompareVector(bone1->rotation, bones[0]->rotation));
    ASSERT_EQ(bone1->index, bones[0]->index);
    ASSERT_TRUE(CompareVector(bone2->position, bones[1]->position));
    ASSERT_TRUE(CompareVector(bone2->rotation, bones[1]->rotation));
    ASSERT_EQ(bone2->index, bones[1]->index);
    bone1.take();
    bone2.take();
}

TEST_P(FragmentTest, ReadWriteGroupMorph)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Morph morph(0), morph2(0);
    QScopedPointer<Morph::Group> group1(new Morph::Group()), group2(new Morph::Group());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.morphIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // group morph1
    group1->index = 0;
    group1->weight = 0.1;
    morph.addGroupMorph(group1.data());
    // group morph2
    group2->index = 1;
    group2->weight = 0.2;
    morph.addGroupMorph(group2.data());
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kEye);
    morph.setType(pmx::Morph::kGroupMorph);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(morph2.name()->equals(morph.name()));
    ASSERT_TRUE(morph2.englishName()->equals(morph.englishName()));
    ASSERT_EQ(morph.category(), morph2.category());
    ASSERT_EQ(morph.type(), morph2.type());
    const Array<Morph::Group *> &groups = morph2.groups();
    ASSERT_EQ(groups.count(), 2);
    ASSERT_EQ(group1->weight, groups[0]->weight);
    ASSERT_EQ(group1->index, groups[0]->index);
    ASSERT_EQ(group2->weight, groups[1]->weight);
    ASSERT_EQ(group2->index, groups[1]->index);
    group1.take();
    group2.take();
}

TEST_P(FragmentTest, ReadWriteMaterialMorph)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Morph morph(0), morph2(0);
    QScopedPointer<Morph::Material> material1(new Morph::Material()), material2(new Morph::Material());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.materialIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // material morph1
    material1->index = 0;
    material1->ambient.setValue(0.01, 0.02, 0.03);
    material1->diffuse.setValue(0.11, 0.12, 0.13, 0.14);
    material1->specular.setValue(0.21, 0.22, 0.23);
    material1->edgeColor.setValue(0.31, 0.32, 0.33, 0.34);
    material1->sphereTextureWeight.setValue(0.41, 0.42, 0.43, 0.44);
    material1->textureWeight.setValue(0.51, 0.52, 0.53, 0.54);
    material1->toonTextureWeight.setValue(0.61, 0.62, 0.63, 0.64);
    material1->edgeSize = 0.1;
    material1->shininess = 0.2;
    material1->operation = 1;
    // material morph2
    morph.addMaterialMorph(material1.data());
    material2->index = 1;
    material2->ambient.setValue(0.61, 0.62, 0.63);
    material2->diffuse.setValue(0.51, 0.52, 0.53, 0.54);
    material2->specular.setValue(0.41, 0.42, 0.43);
    material2->edgeColor.setValue(0.31, 0.32, 0.33, 0.34);
    material2->sphereTextureWeight.setValue(0.21, 0.22, 0.23, 0.24);
    material2->textureWeight.setValue(0.11, 0.12, 0.13, 0.14);
    material2->toonTextureWeight.setValue(0.01, 0.02, 0.03, 0.04);
    material2->edgeSize = 0.2;
    material2->shininess = 0.1;
    material2->operation = 2;
    morph.addMaterialMorph(material2.data());
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kLip);
    morph.setType(pmx::Morph::kMaterialMorph);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(morph2.name()->equals(morph.name()));
    ASSERT_TRUE(morph2.englishName()->equals(morph.englishName()));
    ASSERT_EQ(morph.category(), morph2.category());
    ASSERT_EQ(morph.type(), morph2.type());
    const Array<Morph::Material *> &materials = morph2.materials();
    ASSERT_EQ(materials.count(), 2);
    ASSERT_TRUE(CompareVector(material1->ambient, materials[0]->ambient));
    ASSERT_TRUE(CompareVector(material1->diffuse, materials[0]->diffuse));
    ASSERT_TRUE(CompareVector(material1->specular, materials[0]->specular));
    ASSERT_TRUE(CompareVector(material1->edgeColor, materials[0]->edgeColor));
    ASSERT_TRUE(CompareVector(material1->sphereTextureWeight, materials[0]->sphereTextureWeight));
    ASSERT_TRUE(CompareVector(material1->textureWeight, materials[0]->textureWeight));
    ASSERT_TRUE(CompareVector(material1->toonTextureWeight, materials[0]->toonTextureWeight));
    ASSERT_EQ(material1->edgeSize, materials[0]->edgeSize);
    ASSERT_EQ(material1->shininess, materials[0]->shininess);
    ASSERT_EQ(material1->operation, materials[0]->operation);
    ASSERT_EQ(material1->index, materials[0]->index);
    ASSERT_TRUE(CompareVector(material2->ambient, materials[1]->ambient));
    ASSERT_TRUE(CompareVector(material2->diffuse, materials[1]->diffuse));
    ASSERT_TRUE(CompareVector(material2->specular, materials[1]->specular));
    ASSERT_TRUE(CompareVector(material2->edgeColor, materials[1]->edgeColor));
    ASSERT_TRUE(CompareVector(material2->sphereTextureWeight, materials[1]->sphereTextureWeight));
    ASSERT_TRUE(CompareVector(material2->textureWeight, materials[1]->textureWeight));
    ASSERT_TRUE(CompareVector(material2->toonTextureWeight, materials[1]->toonTextureWeight));
    ASSERT_EQ(material2->edgeSize, materials[1]->edgeSize);
    ASSERT_EQ(material2->shininess, materials[1]->shininess);
    ASSERT_EQ(material2->operation, materials[1]->operation);
    ASSERT_EQ(material2->index, materials[1]->index);
    material1.take();
    material2.take();
}


TEST_P(FragmentTest, ReadWriteRigidBody)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    RigidBody expected, actual;
    Bone bone(0);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    bone.setIndex(1);
    expected.setName(&name);
    expected.setEnglishName(&englishName);
    expected.setBone(&bone);
    expected.setAngularDamping(0.01);
    expected.setCollisionGroupID(1);
    expected.setCollisionMask(2);
    expected.setFriction(0.11);
    expected.setLinearDamping(0.21);
    expected.setMass(0.31);
    expected.setPosition(Vector3(0.41, 0.42, 0.43));
    expected.setRestitution(0.51);
    expected.setRotation(Vector3(0.61, 0.62, 0.63));
    expected.setShapeType(RigidBody::kCapsureShape);
    expected.setSize(Vector3(0.71, 0.72, 0.73));
    expected.setType(RigidBody::kAlignedObject);
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareRigidBody(expected, actual, bone));
}

TEST_P(FragmentTest, ReadWriteVertexMorph)
{
    size_t indexSize = GetParam();
    Encoding encoding;
    Morph morph(0), morph2(0);
    QScopedPointer<Morph::Vertex> vertex1(new Morph::Vertex()), vertex2(new Morph::Vertex());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // vertex morph1
    vertex1->index = 0;
    vertex1->position.setValue(0.1, 0.2, 0.3);
    morph.addVertexMorph(vertex1.data());
    // vertex morph2
    vertex2->index = 1;
    vertex2->position.setValue(0.4, 0.5, 0.6);
    morph.addVertexMorph(vertex2.data());
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kOther);
    morph.setType(pmx::Morph::kVertexMorph);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(morph2.name()->equals(morph.name()));
    ASSERT_TRUE(morph2.englishName()->equals(morph.englishName()));
    ASSERT_EQ(morph.category(), morph2.category());
    ASSERT_EQ(morph.type(), morph2.type());
    const Array<Morph::Vertex *> &vertices = morph2.vertices();
    ASSERT_EQ(vertices.count(), 2);
    ASSERT_TRUE(CompareVector(vertex1->position, vertices[0]->position));
    ASSERT_EQ(vertex1->index, vertices[0]->index);
    ASSERT_TRUE(CompareVector(vertex2->position, vertices[1]->position));
    ASSERT_EQ(vertex2->index, vertices[1]->index);
    vertex1.take();
    vertex2.take();
}

TEST_P(FragmentTest, ReadWriteVertexBdef1)
{
    size_t indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    SetVertex(expected, Vertex::kBdef1, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(FragmentTest, ReadWriteVertexBdef2)
{
    size_t indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    SetVertex(expected, Vertex::kBdef2, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(FragmentTest, ReadWriteVertexBdef4)
{
    size_t indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0), bone3(0), bone4(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    bone3.setIndex(2);
    bones.add(&bone3);
    bone4.setIndex(3);
    bones.add(&bone4);
    SetVertex(expected, Vertex::kBdef4, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(FragmentTest, ReadWriteVertexSdef)
{
    size_t indexSize = GetParam();
    Array<Bone *> bones;
    Vertex expected(0), actual(0);
    Bone bone1(0), bone2(0);
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    SetVertex(expected, Vertex::kSdef, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = expected.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    expected.write(data.data(), info);
    actual.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareVertex(expected, actual, bones));
}

TEST_P(FragmentWithUVTest, ReadWriteUVMorph)
{
    size_t indexSize = get<0>(GetParam());
    pmx::Morph::Type type = get<1>(GetParam());
    Encoding encoding;
    Morph morph(0), morph2(0);
    QScopedPointer<Morph::UV> uv1(new Morph::UV()), uv2(new Morph::UV());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // UV morph1
    uv1->index = 0;
    uv1->position.setValue(0.1, 0.2, 0.3, 0.4);
    morph.addUVMorph(uv1.data());
    // UV morph2
    uv2->index = 1;
    uv2->position.setValue(0.5, 0.6, 0.7, 0.8);
    morph.addUVMorph(uv2.data());
    morph.setName(&name);
    morph.setEnglishName(&englishName);
    morph.setCategory(IMorph::kOther);
    morph.setType(type);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(morph2.name()->equals(morph.name()));
    ASSERT_TRUE(morph2.englishName()->equals(morph.englishName()));
    ASSERT_EQ(morph.category(), morph2.category());
    ASSERT_EQ(morph.type(), morph2.type());
    const Array<Morph::UV *> &uvs = morph2.uvs();
    ASSERT_EQ(uvs.count(), 2);
    ASSERT_TRUE(CompareVector(uv1->position, uvs[0]->position));
    ASSERT_EQ(type - pmx::Morph::kTexCoordMorph, uvs[0]->offset);
    ASSERT_EQ(uv1->index, uvs[0]->index);
    ASSERT_TRUE(CompareVector(uv2->position, uvs[1]->position));
    ASSERT_EQ(type - pmx::Morph::kTexCoordMorph, uvs[1]->offset);
    ASSERT_EQ(uv2->index, uvs[1]->index);
    uv1.take();
    uv2.take();
}

INSTANTIATE_TEST_CASE_P(ModelInstance, FragmentTest, Values(1, 2, 4));
INSTANTIATE_TEST_CASE_P(ModelInstance, FragmentWithUVTest, Combine(Values(1, 2, 4),
                                                                   Values(pmx::Morph::kTexCoordMorph,
                                                                          pmx::Morph::kUVA1Morph,
                                                                          pmx::Morph::kUVA2Morph,
                                                                          pmx::Morph::kUVA3Morph,
                                                                          pmx::Morph::kUVA4Morph)));

TEST(BoneTest, DefaultFlags)
{
    Bone bone(0);
    ASSERT_FALSE(bone.isMovable());
    ASSERT_FALSE(bone.isRotateable());
    ASSERT_FALSE(bone.isVisible());
    ASSERT_FALSE(bone.isInteractive());
    ASSERT_FALSE(bone.hasInverseKinematics());
    ASSERT_FALSE(bone.hasPositionInherence());
    ASSERT_FALSE(bone.hasRotationInherence());
    ASSERT_FALSE(bone.hasFixedAxes());
    ASSERT_FALSE(bone.hasLocalAxes());
    ASSERT_FALSE(bone.isTransformedAfterPhysicsSimulation());
    ASSERT_FALSE(bone.isTransformedByExternalParent());
}

TEST(VertexTest, Boundary)
{
    Vertex vertex(0);
    QScopedPointer<Bone> bone(new Bone(0));
    vertex.setUV(-1, Vector4(1, 1, 1, 1));
    vertex.setUV( 4, Vector4(1, 1, 1, 1));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight( 4, 0.1);
    vertex.setBone(-1, bone.data());
    vertex.setBone( 4, bone.data());
    ASSERT_EQ(vertex.uv(-1).x(), 0.0f);
    ASSERT_EQ(vertex.uv(4).x(), 0.0f);
    ASSERT_EQ(vertex.bone(-1), static_cast<IBone *>(0));
    ASSERT_EQ(vertex.bone(4), static_cast<IBone *>(0));
    ASSERT_EQ(vertex.weight(-1), 0.0f);
    ASSERT_EQ(vertex.weight(4), 0.0f);
}

TEST(MaterialTest, MergeAmbientColor)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.ambient.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setAmbient(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.resetMorph();
    // mod (0.0)
    morph.ambient.setValue(0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.6, 0.6, 0.6, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.4, 0.4, 0.4, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.2, 0.2, 0.2, 1.0), material.ambient()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.0, 0.0, 0.0, 1.0), material.ambient()));
    material.resetMorph();
    // add (0.2)
    morph.ambient.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.85, 0.85, 0.85, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.9, 0.9, 0.9, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 1.0), material.ambient()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.0, 1.0, 1.0, 1.0), material.ambient()));
    material.resetMorph();
    // add (0.6)
    morph.ambient.setValue(0.6, 0.6, 0.6);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(1.1, 1.1, 1.1, 1.0), material.ambient()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(1.25, 1.25, 1.25, 1.0), material.ambient()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.4, 1.4, 1.4, 1.0), material.ambient()));
}

TEST(MaterialTest, MergeDiffuseColor)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.diffuse.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setDiffuse(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.resetMorph();
    // mod (0.0)
    morph.diffuse.setValue(0.0, 0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.6, 0.6, 0.6, 0.6), material.diffuse()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.4, 0.4, 0.4, 0.4), material.diffuse()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.2, 0.2, 0.2, 0.2), material.diffuse()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.0, 0.0, 0.0, 0.0), material.diffuse()));
    material.resetMorph();
    // add (0.2)
    morph.diffuse.setValue(0.2, 0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.85, 0.85, 0.85, 0.85), material.diffuse()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.9, 0.9, 0.9, 0.9), material.diffuse()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 0.95), material.diffuse()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.0, 1.0, 1.0, 1.0), material.diffuse()));
    material.resetMorph();
    // add (0.6)
    morph.diffuse.setValue(0.6, 0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.diffuse()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 0.95), material.diffuse()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(1.1, 1.1, 1.1, 1.1), material.diffuse()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(1.25, 1.25, 1.25, 1.25), material.diffuse()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.4, 1.4, 1.4, 1.4), material.diffuse()));
}

TEST(MaterialTest, MergeSpecularColor)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.specular.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setSpecular(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.resetMorph();
    // mod (0.0)
    morph.specular.setValue(0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.6, 0.6, 0.6, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.4, 0.4, 0.4, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.2, 0.2, 0.2, 1.0), material.specular()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.0, 0.0, 0.0, 1.0), material.specular()));
    material.resetMorph();
    // add (0.2)
    morph.specular.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.85, 0.85, 0.85, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.9, 0.9, 0.9, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 1.0), material.specular()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.0, 1.0, 1.0, 1.0), material.specular()));
    material.resetMorph();
    // add (0.6)
    morph.specular.setValue(0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(1.1, 1.1, 1.1, 1.0), material.specular()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(1.25, 1.25, 1.25, 1.0), material.specular()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.4, 1.4, 1.4, 1.0), material.specular()));
}

TEST(MaterialTest, MergeShininess)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.shininess = 1.0;
    morph.operation = 0;
    material.setShininess(0.8);
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.resetMorph();
    // mod (0.0)
    morph.shininess = 0.0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.shininess(), 0.6f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.shininess(), 0.4f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.shininess(), 0.2f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.shininess(), 0.0f);
    material.resetMorph();
    // add (0.2)
    morph.shininess = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.shininess(), 0.85f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.shininess(), 0.9f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.shininess(), 0.95f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.shininess(), 1.0f);
    // add (0.6)
    morph.shininess = 0.6;
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.shininess(), 0.95f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.shininess(), 1.1f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.shininess(), 1.25f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.shininess(), 1.4f);
}

TEST(MaterialTest, MergeEdgeColor)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.edgeColor.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setEdgeColor(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.resetMorph();
    // mod (0.0)
    morph.edgeColor.setValue(0.0, 0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.6, 0.6, 0.6, 0.6), material.edgeColor()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.4, 0.4, 0.4, 0.4), material.edgeColor()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.2, 0.2, 0.2, 0.2), material.edgeColor()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(0.0, 0.0, 0.0, 0.0), material.edgeColor()));
    material.resetMorph();
    // add (0.2)
    morph.edgeColor.setValue(0.2, 0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.85, 0.85, 0.85, 0.85), material.edgeColor()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(0.9, 0.9, 0.9, 0.9), material.edgeColor()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 0.95), material.edgeColor()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.0, 1.0, 1.0, 1.0), material.edgeColor()));
    material.resetMorph();
    // add (0.6)
    morph.edgeColor.setValue(0.6, 0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    ASSERT_TRUE(CompareVector(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor()));
    material.mergeMorph(&morph, 0.25);
    ASSERT_TRUE(CompareVector(Color(0.95, 0.95, 0.95, 0.95), material.edgeColor()));
    material.mergeMorph(&morph, 0.5);
    ASSERT_TRUE(CompareVector(Color(1.1, 1.1, 1.1, 1.1), material.edgeColor()));
    material.mergeMorph(&morph, 0.75);
    ASSERT_TRUE(CompareVector(Color(1.25, 1.25, 1.25, 1.25), material.edgeColor()));
    material.mergeMorph(&morph, 1.0);
    ASSERT_TRUE(CompareVector(Color(1.4, 1.4, 1.4, 1.4), material.edgeColor()));
}

TEST(MaterialTest, MergeEdgeSize)
{
    Material material(0);
    Morph::Material morph;
    // mod (1.0)
    morph.edgeSize = 1.0;
    morph.operation = 0;
    material.setEdgeSize(0.8);
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.resetMorph();
    // mod (1.0)
    morph.edgeSize = 0.0;
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.6f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.4f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.2f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.0f);
    material.resetMorph();
    // add (0.2)
    morph.edgeSize = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.85f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.9f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.95f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 1.0f);
    // add (0.6)
    morph.edgeSize = 0.6;
    material.mergeMorph(&morph, 0.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    ASSERT_FLOAT_EQ(material.edgeSize(), 0.95f);
    material.mergeMorph(&morph, 0.5);
    ASSERT_FLOAT_EQ(material.edgeSize(), 1.1f);
    material.mergeMorph(&morph, 0.75);
    ASSERT_FLOAT_EQ(material.edgeSize(), 1.25f);
    material.mergeMorph(&morph, 1.0);
    ASSERT_FLOAT_EQ(material.edgeSize(), 1.4f);
}

TEST(VertexTest, PerformSkinningBdef1)
{
    pmx::Vertex v(0);
    MockIBone bone;
    Transform transform(Matrix3x3::getIdentity().scaled(Vector3(0.5, 0.5, 0.5)), Vector3(1, 2, 3));
    EXPECT_CALL(bone, localTransform()).Times(1).WillRepeatedly(Return(transform));
    EXPECT_CALL(bone, index()).Times(1).WillRepeatedly(Return(0));
    v.setType(pmx::Vertex::kBdef1);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    v.setBone(0, &bone);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.05, 2.1, 3.15), position));
    ASSERT_TRUE(CompareVector(Vector3(0.2, 0.25, 0.3), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightZero)
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
    v.setBone(0, &bone1);
    v.setBone(1, &bone2);
    v.setWeight(0, 0);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(4.025, 5.05, 6.075), position));
    ASSERT_TRUE(CompareVector(Vector3(0.1, 0.125, 0.15), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightOne)
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
    v.setBone(0, &bone1);
    v.setBone(1, &bone2);
    v.setWeight(0, 1);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.075, 2.15, 3.225), position));
    ASSERT_TRUE(CompareVector(Vector3(0.3, 0.375, 0.45), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightHalf)
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
    v.setBone(0, &bone1);
    v.setBone(1, &bone2);
    v.setWeight(0, 0.5);
    v.setWeight(1, 0.5);
    Vector3 position, normal;
    v.performSkinning(position, normal);
    const Vector3 &v2 = (Vector3(1.075, 2.15, 3.225) + Vector3(4.025, 5.05, 6.075)) * 0.5;
    const Vector3 &n2 = (Vector3(0.1, 0.125, 0.15) + Vector3(0.3, 0.375, 0.45)) * 0.5;
    ASSERT_TRUE(CompareVector(v2, position));
    ASSERT_TRUE(CompareVector(n2, normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightZeroPMDCompat)
{
    MockIBone bone1, bone2;
    //Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    //EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.add(&bone1);
    bones.add(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(0);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(4.025, 5.05, 6.075), position));
    ASSERT_TRUE(CompareVector(Vector3(0.1, 0.125, 0.15), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightOnePMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    //Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    //EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(ReturnRef(transform2));
    Array<IBone *> bones;
    bones.add(&bone1);
    bones.add(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(100);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    ASSERT_TRUE(CompareVector(Vector3(1.075, 2.15, 3.225), position));
    ASSERT_TRUE(CompareVector(Vector3(0.3, 0.375, 0.45), normal));
}

TEST(VertexTest, PerformSkinningBdef2WeightHalfPMDCompat)
{
    MockIBone bone1, bone2;
    Transform transform1(Matrix3x3::getIdentity().scaled(Vector3(0.75, 0.75, 0.75)), Vector3(1, 2, 3));
    EXPECT_CALL(bone1, localTransform()).Times(1).WillRepeatedly(Return(transform1));
    Transform transform2(Matrix3x3::getIdentity().scaled(Vector3(0.25, 0.25, 0.25)), Vector3(4, 5, 6));
    EXPECT_CALL(bone2, localTransform()).Times(1).WillRepeatedly(Return(transform2));
    Array<IBone *> bones;
    bones.add(&bone1);
    bones.add(&bone2);
    vpvl::Vertex vv;
    vv.setTexCoord(0, 1);
    vv.setBones(0, 1);
    vv.setWeight(50);
    pmd::Vertex v(0, &vv, &bones, 0);
    v.setOrigin(Vector3(0.1, 0.2, 0.3));
    v.setNormal(Vector3(0.4, 0.5, 0.6));
    Vector3 position, normal;
    v.performSkinning(position, normal);
    const Vector3 &v2 = (Vector3(1.075, 2.15, 3.225) + Vector3(4.025, 5.05, 6.075)) * 0.5;
    const Vector3 &n2 = (Vector3(0.1, 0.125, 0.15) + Vector3(0.3, 0.375, 0.45)) * 0.5;
    ASSERT_TRUE(CompareVector(v2, position));
    ASSERT_TRUE(CompareVector(n2, normal));
}

TEST(ModelTest, ParseEmpty)
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    ASSERT_FALSE(model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    ASSERT_EQ(Model::kInvalidHeaderError, model.error());
}

TEST(ModelTest, ParseRealPMD)
{
    QFile file("miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding encoding;
        pmd::Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMDModel, model.type());
    }
    else {
        // skip
    }
}

TEST(ModelTest, ParseRealPMX)
{
    QFile file("miku.pmx");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding encoding;
        pmx::Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMXModel, model.type());
    }
    else {
        // skip
    }
}

#include <gtest/gtest.h>

#include <btBulletDynamicsCommon.h>
#include <vpvl2/internal/util.h>

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

#include "Common.h"

using namespace ::testing;
using namespace vpvl2::pmx;

namespace
{

static void Compare(const Vector3 &expected, const Vector3 &actual)
{
    EXPECT_FLOAT_EQ(expected.x(), actual.x());
    EXPECT_FLOAT_EQ(expected.y(), actual.y());
    EXPECT_FLOAT_EQ(expected.z(), actual.z());
}

static void Compare(const Vector4 &expected, const Vector4 &actual)
{
    EXPECT_FLOAT_EQ(expected.x(), actual.x());
    EXPECT_FLOAT_EQ(expected.y(), actual.y());
    EXPECT_FLOAT_EQ(expected.z(), actual.z());
    EXPECT_FLOAT_EQ(expected.w(), actual.w());
}

static void Compare(const Quaternion &expected, const Quaternion &actual)
{
    EXPECT_FLOAT_EQ(expected.x(), actual.x());
    EXPECT_FLOAT_EQ(expected.y(), actual.y());
    EXPECT_FLOAT_EQ(expected.z(), actual.z());
    EXPECT_FLOAT_EQ(expected.w(), actual.w());
}

static void SetVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones)
{
    vertex.setOrigin(Vector3(0.01, 0.02, 0.03));
    vertex.setNormal(Vector3(0.11, 0.12, 0.13));
    vertex.setTexCoord(Vector3(0.21, 0.22, 0.0));
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

static void CompareVertex(const Vertex &expected, const Vertex &vertex2, const Array<Bone *> &bones)
{
    Compare(vertex2.origin(), expected.origin());
    Compare(vertex2.normal(), expected.normal());
    Compare(vertex2.texcoord(), expected.texcoord());
    Compare(vertex2.uv(0), expected.uv(0));
    Compare(kZeroV4, expected.uv(1));
    EXPECT_EQ(expected.type(), vertex2.type());
    EXPECT_EQ(expected.edgeSize(), vertex2.edgeSize());
    if (expected.type() == Vertex::kSdef) {
        Compare(vertex2.sdefC(), expected.sdefC());
        Compare(vertex2.sdefR0(), expected.sdefR0());
        Compare(vertex2.sdefR1(), expected.sdefR1());
    }
    else {
        Compare(kZeroV3, vertex2.sdefC());
        Compare(kZeroV3, vertex2.sdefR0());
        Compare(kZeroV3, vertex2.sdefR1());
    }
    Array<Vertex *> vertices;
    vertices.add(const_cast<Vertex *>(&expected));
    Vertex::loadVertices(vertices, bones);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        EXPECT_EQ(expected.bone(i), bones[i]);
        EXPECT_TRUE(bones[i]->index() != -1);
        if (nbones == 4)
            EXPECT_FLOAT_EQ(vertex2.weight(i), 0.2f + 0.1f * i);
    }
    if (nbones == 2)
        EXPECT_FLOAT_EQ(vertex2.weight(0), 0.2f);
}

static void TestReadWriteBone(size_t indexSize)
{
    Encoding encoding;
    Bone bone, bone2, parent;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    // construct bone
    parent.setIndex(0);
    bone.setName(&name);
    bone.setEnglishName(&englishName);
    bone.setDestinationOrigin(Vector3(0.01, 0.02, 0.03));
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
    bone.setTransformedAfterPhysicsSimulationEnable(true);
    bone.setTransformedByExternalParentEnable(true);
    // write constructed bone and read it
    size_t size = bone.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    bone.write(data.data(), info);
    bone2.read(data.data(), info, read);
    // compare read bone
    EXPECT_EQ(size, read);
    EXPECT_TRUE(bone2.name()->equals(bone.name()));
    EXPECT_TRUE(bone2.englishName()->equals(bone.englishName()));
    Compare(bone2.origin(), bone.origin());
    Compare(bone2.destinationOrigin(), bone.destinationOrigin());
    Compare(bone2.axis(), bone.axis());
    Compare(bone2.axisX(), bone.axisX());
    Compare(bone2.axisZ(), bone.axisZ());
    EXPECT_EQ(bone.layerIndex(), bone2.layerIndex());
    EXPECT_EQ(bone.externalIndex(), bone2.externalIndex());
    EXPECT_TRUE(bone2.isRotateable());
    EXPECT_TRUE(bone2.isMovable());
    EXPECT_TRUE(bone2.isVisible());
    EXPECT_TRUE(bone2.isInteractive());
    EXPECT_TRUE(bone2.hasInverseKinematics());
    EXPECT_TRUE(bone2.hasPositionInherence());
    EXPECT_TRUE(bone2.hasRotationInherence());
    EXPECT_TRUE(bone2.hasFixedAxes());
    EXPECT_TRUE(bone2.hasLocalAxes());
    EXPECT_TRUE(bone2.isTransformedAfterPhysicsSimulation());
    EXPECT_TRUE(bone2.isTransformedByExternalParent());
    Array<Bone *> bones, apb, bpb;
    bones.add(&parent);
    bones.add(&bone2);
    Bone::loadBones(bones, bpb, apb);
    EXPECT_EQ(&parent, bone2.parentBone());
    EXPECT_EQ(&parent, bone2.parentInherenceBone());
    EXPECT_EQ(&parent, bone2.targetBone());
}

static void TestReadWriteJoint(size_t indexSize)
{
    Encoding encoding;
    Joint joint, joint2;
    RigidBody body, body2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.rigidBodyIndexSize = indexSize;
    // construct joint
    body.setIndex(0);
    body2.setIndex(1);
    joint.setName(&name);
    joint.setEnglishName(&englishName);
    joint.setRigidBody1(&body);
    joint.setRigidBody2(&body2);
    joint.setPosition(Vector3(0.01, 0.02, 0.03));
    joint.setRotation(Vector3(0.11, 0.12, 0.13));
    joint.setPositionLowerLimit(Vector3(0.21, 0.22, 0.23));
    joint.setRotationLowerLimit(Vector3(0.31, 0.32, 0.33));
    joint.setPositionUpperLimit(Vector3(0.41, 0.42, 0.43));
    joint.setRotationUpperLimit(Vector3(0.51, 0.52, 0.53));
    joint.setPositionStiffness(Vector3(0.61, 0.62, 0.63));
    joint.setRotationStiffness(Vector3(0.71, 0.72, 0.73));
    // write constructed joint and read it
    size_t size = joint.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    joint.write(data.data(), info);
    joint2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    // compare read joint
    EXPECT_TRUE(joint2.name()->equals(joint.name()));
    EXPECT_TRUE(joint2.englishName()->equals(joint.englishName()));
    EXPECT_EQ(joint.position(), joint2.position());
    EXPECT_EQ(joint.rotation(), joint2.rotation());
    EXPECT_EQ(joint.positionLowerLimit(), joint2.positionLowerLimit());
    EXPECT_EQ(joint.rotationLowerLimit(), joint2.rotationLowerLimit());
    EXPECT_EQ(joint.positionUpperLimit(), joint2.positionUpperLimit());
    EXPECT_EQ(joint.rotationUpperLimit(), joint2.rotationUpperLimit());
    EXPECT_EQ(joint.positionStiffness(), joint2.positionStiffness());
    EXPECT_EQ(joint.rotationStiffness(), joint2.rotationStiffness());
    EXPECT_EQ(body.index(), joint2.rigidBodyIndex1());
    EXPECT_EQ(body2.index(), joint2.rigidBodyIndex2());
}

static void TestReadWriteMaterial(size_t indexSize)
{
    Encoding encoding;
    Material material, material2;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.textureIndexSize = indexSize;
    // construct material
    material.setName(&name);
    material.setEnglishName(&englishName);
    material.setSphereTextureRenderMode(Material::kSubTexture);
    material.setAmbient(Color(0.01, 0.02, 0.03, 1.0));
    material.setDiffuse(Color(0.11, 0.12, 0.13, 0.14));
    material.setSpecular(Color(0.21, 0.22, 0.23, 1.0));
    material.setEdgeColor(Color(0.31, 0.32, 0.33, 0.34));
    material.setShininess(0.1);
    material.setEdgeSize(0.2);
    material.setMainTextureIndex(1);
    material.setSphereTextureIndex(2);
    material.setToonTextureIndex(3);
    material.setIndices(4);
    material.setFlags(5);
    // write contructed material and read it
    size_t size = material.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    material.write(data.data(), info);
    material2.read(data.data(), info, read);
    // compare read material
    EXPECT_EQ(size, read);
    EXPECT_TRUE(material2.name()->equals(material.name()));
    EXPECT_TRUE(material2.englishName()->equals(material.englishName()));
    Compare(material2.ambient(), material.ambient());
    Compare(material2.diffuse(), material.diffuse());
    Compare(material2.specular(), material.specular());
    Compare(material2.edgeColor(), material.edgeColor());
    EXPECT_EQ(material.sphereTextureRenderMode(), material2.sphereTextureRenderMode());
    EXPECT_EQ(material.shininess(), material2.shininess());
    EXPECT_EQ(material.edgeSize(), material2.edgeSize());
    EXPECT_EQ(material.textureIndex(), material2.textureIndex());
    EXPECT_EQ(material.sphereTextureIndex(), material2.sphereTextureIndex());
    EXPECT_EQ(material.toonTextureIndex(), material2.toonTextureIndex());
    EXPECT_EQ(material.indices(), material2.indices());
}

static void TestReadWriteBoneMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
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
    morph.setType(pmx::Morph::kBone);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    EXPECT_TRUE(morph2.name()->equals(morph.name()));
    EXPECT_TRUE(morph2.englishName()->equals(morph.englishName()));
    EXPECT_EQ(morph.category(), morph2.category());
    EXPECT_EQ(morph.type(), morph2.type());
    const Array<Morph::Bone *> &bones = morph2.bones();
    EXPECT_EQ(bones.count(), 2);
    Compare(bones[0]->position, bone1->position);
    Compare(bones[0]->rotation, bone1->rotation);
    EXPECT_EQ(bone1->index, bones[0]->index);
    Compare(bones[1]->position, bone2->position);
    Compare(bones[1]->rotation, bone2->rotation);
    EXPECT_EQ(bone2->index, bones[1]->index);
    // delete bone1 and bone2 at Morph destructor
    bone1.take();
    bone2.take();
}

static void TestReadWriteGroupMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
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
    morph.setType(pmx::Morph::kGroup);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    EXPECT_TRUE(morph2.name()->equals(morph.name()));
    EXPECT_TRUE(morph2.englishName()->equals(morph.englishName()));
    EXPECT_EQ(morph.category(), morph2.category());
    EXPECT_EQ(morph.type(), morph2.type());
    const Array<Morph::Group *> &groups = morph2.groups();
    EXPECT_EQ(groups.count(), 2);
    EXPECT_EQ(group1->weight, groups[0]->weight);
    EXPECT_EQ(group1->index, groups[0]->index);
    EXPECT_EQ(group2->weight, groups[1]->weight);
    EXPECT_EQ(group2->index, groups[1]->index);
    // delete group1 and group2 at Morph destructor
    group1.take();
    group2.take();
}

static void TestReadWriteMaterialMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
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
    morph.setType(pmx::Morph::kMaterial);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    EXPECT_TRUE(morph2.name()->equals(morph.name()));
    EXPECT_TRUE(morph2.englishName()->equals(morph.englishName()));
    EXPECT_EQ(morph.category(), morph2.category());
    EXPECT_EQ(morph.type(), morph2.type());
    const Array<Morph::Material *> &materials = morph2.materials();
    EXPECT_EQ(materials.count(), 2);
    Compare(materials[0]->ambient, material1->ambient);
    Compare(materials[0]->diffuse, material1->diffuse);
    Compare(materials[0]->specular, material1->specular);
    Compare(materials[0]->edgeColor, material1->edgeColor);
    Compare(materials[0]->sphereTextureWeight, material1->sphereTextureWeight);
    Compare(materials[0]->textureWeight, material1->textureWeight);
    Compare(materials[0]->toonTextureWeight, material1->toonTextureWeight);
    EXPECT_EQ(material1->edgeSize, materials[0]->edgeSize);
    EXPECT_EQ(material1->shininess, materials[0]->shininess);
    EXPECT_EQ(material1->operation, materials[0]->operation);
    EXPECT_EQ(material1->index, materials[0]->index);
    Compare(materials[1]->ambient, material2->ambient);
    Compare(materials[1]->diffuse, material2->diffuse);
    Compare(materials[1]->specular, material2->specular);
    Compare(materials[1]->edgeColor, material2->edgeColor);
    Compare(materials[1]->sphereTextureWeight, material2->sphereTextureWeight);
    Compare(materials[1]->textureWeight, material2->textureWeight);
    Compare(materials[1]->toonTextureWeight, material2->toonTextureWeight);
    EXPECT_EQ(material2->edgeSize, materials[1]->edgeSize);
    EXPECT_EQ(material2->shininess, materials[1]->shininess);
    EXPECT_EQ(material2->operation, materials[1]->operation);
    EXPECT_EQ(material2->index, materials[1]->index);
    // delete material1 and mateiral2 at Morph destructor
    material1.take();
    material2.take();
}

static void TestReadWriteRigidBody(size_t indexSize)
{
    Encoding encoding;
    RigidBody body, body2;
    Bone bone;
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.boneIndexSize = indexSize;
    bone.setIndex(1);
    body.setName(&name);
    body.setEnglishName(&englishName);
    body.setBone(&bone);
    body.setAngularDamping(0.01);
    body.setCollisionGroupID(1);
    body.setCollisionMask(2);
    body.setFriction(0.11);
    body.setLinearDamping(0.21);
    body.setMass(0.31);
    body.setPosition(Vector3(0.41, 0.42, 0.43));
    body.setRestitution(0.51);
    body.setRotation(Vector3(0.61, 0.62, 0.63));
    body.setShapeType(4);
    body.setSize(Vector3(0.71, 0.72, 0.73));
    body.setType(5);
    size_t size = body.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    body.write(data.data(), info);
    body2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    EXPECT_TRUE(body2.name()->equals(body.name()));
    EXPECT_TRUE(body2.englishName()->equals(body.englishName()));
    EXPECT_EQ(bone.index(), body2.boneIndex());
    EXPECT_EQ(body.angularDamping(), body2.angularDamping());
    EXPECT_EQ(body.collisionGroupID(), body2.collisionGroupID());
    EXPECT_EQ(body.collisionGroupMask(), body2.collisionGroupMask());
    EXPECT_EQ(body.friction(), body2.friction());
    EXPECT_EQ(body.linearDamping(), body2.linearDamping());
    EXPECT_EQ(body.mass(), body2.mass());
    EXPECT_EQ(body.position(), body2.position());
    EXPECT_EQ(body.restitution(), body2.restitution());
    EXPECT_EQ(body.rotation(), body2.rotation());
    EXPECT_EQ(body.size(), body2.size());
    EXPECT_EQ(bone.index(), body2.boneIndex());
}

static void TestReadWriteUVMorph(size_t indexSize, pmx::Morph::Type type)
{
    Encoding encoding;
    Morph morph, morph2;
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
    EXPECT_EQ(size, read);
    EXPECT_TRUE(morph2.name()->equals(morph.name()));
    EXPECT_TRUE(morph2.englishName()->equals(morph.englishName()));
    EXPECT_EQ(morph.category(), morph2.category());
    EXPECT_EQ(morph.type(), morph2.type());
    const Array<Morph::UV *> &uvs = morph2.uvs();
    EXPECT_EQ(uvs.count(), 2);
    Compare(uvs[0]->position, uv1->position);
    EXPECT_EQ(type - pmx::Morph::kTexCoord, uvs[0]->offset);
    EXPECT_EQ(uv1->index, uvs[0]->index);
    Compare(uvs[1]->position, uv2->position);
    EXPECT_EQ(type - pmx::Morph::kTexCoord, uvs[1]->offset);
    EXPECT_EQ(uv2->index, uvs[1]->index);
    // delete uv1 and uv2 at Morph destructor
    uv1.take();
    uv2.take();
}

static void TestReadWriteVertexMorph(size_t indexSize)
{
    Encoding encoding;
    Morph morph, morph2;
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
    morph.setType(pmx::Morph::kVertex);
    size_t size = morph.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    morph.write(data.data(), info);
    morph2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    EXPECT_TRUE(morph2.name()->equals(morph.name()));
    EXPECT_TRUE(morph2.englishName()->equals(morph.englishName()));
    EXPECT_EQ(morph.category(), morph2.category());
    EXPECT_EQ(morph.type(), morph2.type());
    const Array<Morph::Vertex *> &vertices = morph2.vertices();
    EXPECT_EQ(vertices.count(), 2);
    Compare(vertices[0]->position, vertex1->position);
    EXPECT_EQ(vertex1->index, vertices[0]->index);
    Compare(vertices[1]->position, vertex2->position);
    EXPECT_EQ(vertex2->index, vertices[1]->index);
    // delete vertex1 and vertex2 at Morph destructor
    vertex1.take();
    vertex2.take();
}

static void TestReadWriteVertexBdef1(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1;
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    SetVertex(vertex, Vertex::kBdef1, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    CompareVertex(vertex, vertex2, bones);
}

static void TestReadWriteVertexBdef2(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    SetVertex(vertex, Vertex::kBdef2, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    CompareVertex(vertex, vertex2, bones);
}

static void TestReadWriteVertexBdef4(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2, bone3, bone4;
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    bone3.setIndex(2);
    bones.add(&bone3);
    bone4.setIndex(3);
    bones.add(&bone4);
    SetVertex(vertex, Vertex::kBdef4, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    uint8_t *data = new uint8_t[size];
    vertex.write(data, info);
    vertex2.read(data, info, read);
    EXPECT_EQ(size, read);
    CompareVertex(vertex, vertex2, bones);
    delete[] data;
}

static void TestReadWriteVertexSdef(size_t indexSize)
{
    Array<Bone *> bones;
    Vertex vertex, vertex2;
    Bone bone1, bone2;
    Model::DataInfo info;
    bone1.setIndex(0);
    bones.add(&bone1);
    bone2.setIndex(1);
    bones.add(&bone2);
    SetVertex(vertex, Vertex::kSdef, bones);
    info.additionalUVSize = indexSize;
    info.boneIndexSize = indexSize;
    size_t size = vertex.estimateSize(info), read;
    QScopedArrayPointer<uint8_t> data(new uint8_t[size]);
    vertex.write(data.data(), info);
    vertex2.read(data.data(), info, read);
    EXPECT_EQ(size, read);
    CompareVertex(vertex, vertex2, bones);
}

}

TEST(Bone, ReadWrite)
{
    TestReadWriteBone(1);
    TestReadWriteBone(2);
    TestReadWriteBone(4);
}

TEST(Joint, ReadWrite)
{
    TestReadWriteJoint(1);
    TestReadWriteJoint(2);
    TestReadWriteJoint(4);
}

TEST(Material, ReadWrite)
{
    TestReadWriteMaterial(1);
    TestReadWriteMaterial(2);
    TestReadWriteMaterial(4);
}

TEST(Morph, ReadWriteBone)
{
    TestReadWriteBoneMorph(1);
    TestReadWriteBoneMorph(2);
    TestReadWriteBoneMorph(4);
}

TEST(Morph, ReadWriteGroup)
{
    TestReadWriteGroupMorph(1);
    TestReadWriteGroupMorph(2);
    TestReadWriteGroupMorph(4);
}

TEST(Morph, ReadWriteMaterial)
{
    TestReadWriteMaterialMorph(1);
    TestReadWriteMaterialMorph(2);
    TestReadWriteMaterialMorph(4);
}

TEST(RigidBody, ReadWrite)
{
    TestReadWriteRigidBody(1);
    TestReadWriteRigidBody(2);
    TestReadWriteRigidBody(4);
}

TEST(Morph, ReadWriteTexCoord)
{
    TestReadWriteUVMorph(1, pmx::Morph::kTexCoord);
    TestReadWriteUVMorph(2, pmx::Morph::kTexCoord);
    TestReadWriteUVMorph(4, pmx::Morph::kTexCoord);
}

TEST(Morph, ReadWriteUVA1)
{
    TestReadWriteUVMorph(1, pmx::Morph::kUVA1);
    TestReadWriteUVMorph(2, pmx::Morph::kUVA1);
    TestReadWriteUVMorph(4, pmx::Morph::kUVA1);
}

TEST(Morph, ReadWriteUVA2)
{
    TestReadWriteUVMorph(1, pmx::Morph::kUVA2);
    TestReadWriteUVMorph(2, pmx::Morph::kUVA2);
    TestReadWriteUVMorph(4, pmx::Morph::kUVA2);
}

TEST(Morph, ReadWriteUVA3)
{
    TestReadWriteUVMorph(1, pmx::Morph::kUVA3);
    TestReadWriteUVMorph(2, pmx::Morph::kUVA3);
    TestReadWriteUVMorph(4, pmx::Morph::kUVA3);
}

TEST(Morph, ReadWriteUVA4)
{
    TestReadWriteUVMorph(1, pmx::Morph::kUVA4);
    TestReadWriteUVMorph(2, pmx::Morph::kUVA4);
    TestReadWriteUVMorph(4, pmx::Morph::kUVA4);
}

TEST(Morph, ReadWriteVertex)
{
    TestReadWriteVertexMorph(1);
    TestReadWriteVertexMorph(2);
    TestReadWriteVertexMorph(4);
}

TEST(Vertex, ReadWriteBdef1)
{
    TestReadWriteVertexBdef1(1);
    TestReadWriteVertexBdef1(2);
    TestReadWriteVertexBdef1(4);
}

TEST(Vertex, ReadWriteBdef2)
{
    TestReadWriteVertexBdef2(1);
    TestReadWriteVertexBdef2(2);
    TestReadWriteVertexBdef2(4);
}

TEST(Vertex, ReadWriteBdef4)
{
    TestReadWriteVertexBdef4(1);
    TestReadWriteVertexBdef4(2);
    TestReadWriteVertexBdef4(4);
}

TEST(Vertex, ReadWriteSdef)
{
    TestReadWriteVertexSdef(1);
    TestReadWriteVertexSdef(2);
    TestReadWriteVertexSdef(4);
}

TEST(Model, ParseEmpty)
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    EXPECT_FALSE(model.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    EXPECT_EQ(Model::kInvalidHeaderError, model.error());
}

TEST(Model, ParseFile)
{
    Encoding encoding;
    Model model(&encoding);
    Model::DataInfo info;
    QFile file("miku.pmx");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        const size_t size = file.size();
        EXPECT_TRUE(model.preparse(reinterpret_cast<const uint8_t *>(data), size, info));
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8_t *>(data), size));
    }
    else {
        // QSKIP("Require a model to test this", SkipSingle);
    }
}

TEST(Bone, DefaultFlags)
{
    Bone bone;
    EXPECT_FALSE(bone.isMovable());
    EXPECT_FALSE(bone.isRotateable());
    EXPECT_FALSE(bone.isVisible());
    EXPECT_FALSE(bone.isInteractive());
    EXPECT_FALSE(bone.hasInverseKinematics());
    EXPECT_FALSE(bone.hasPositionInherence());
    EXPECT_FALSE(bone.hasRotationInherence());
    EXPECT_FALSE(bone.hasFixedAxes());
    EXPECT_FALSE(bone.hasLocalAxes());
    EXPECT_FALSE(bone.isTransformedAfterPhysicsSimulation());
    EXPECT_FALSE(bone.isTransformedByExternalParent());
}

TEST(Vertex, Boundary)
{
    Vertex vertex;
    Bone *bone = new Bone();
    vertex.setUV(-1, Vector4(1, 1, 1, 1));
    vertex.setUV( 4, Vector4(1, 1, 1, 1));
    vertex.setWeight(-1, 0.1);
    vertex.setWeight( 4, 0.1);
    vertex.setBone(-1, bone);
    vertex.setBone( 4, bone);
    EXPECT_EQ(vertex.uv(-1).x(), 0.0f);
    EXPECT_EQ(vertex.uv(4).x(), 0.0f);
    EXPECT_EQ(vertex.bone(-1), static_cast<IBone *>(0));
    EXPECT_EQ(vertex.bone(4), static_cast<IBone *>(0));
    EXPECT_EQ(vertex.weight(-1), 0.0f);
    EXPECT_EQ(vertex.weight(4), 0.0f);
    delete bone;
}

TEST(Material, MergeAmbientColor)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.ambient.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setAmbient(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.resetMorph();
    // mod (0.0)
    morph.ambient.setValue(0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.6, 0.6, 0.6, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.4, 0.4, 0.4, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.2, 0.2, 0.2, 1.0), material.ambient());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.0, 0.0, 0.0, 1.0), material.ambient());
    material.resetMorph();
    // add (0.2)
    morph.ambient.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.85, 0.85, 0.85, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.9, 0.9, 0.9, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.95, 0.95, 0.95, 1.0), material.ambient());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.0, 1.0, 1.0, 1.0), material.ambient());
    material.resetMorph();
    // add (0.6)
    morph.ambient.setValue(0.6, 0.6, 0.6);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.95, 0.95, 0.95, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(1.1, 1.1, 1.1, 1.0), material.ambient());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(1.25, 1.25, 1.25, 1.0), material.ambient());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.4, 1.4, 1.4, 1.0), material.ambient());
}

TEST(Material, MergeDiffuseColor)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.diffuse.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setDiffuse(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.resetMorph();
    // mod (0.0)
    morph.diffuse.setValue(0.0, 0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.6, 0.6, 0.6, 0.6), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.4, 0.4, 0.4, 0.4), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.2, 0.2, 0.2, 0.2), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.0, 0.0, 0.0, 0.0), material.diffuse());
    material.resetMorph();
    // add (0.2)
    morph.diffuse.setValue(0.2, 0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.85, 0.85, 0.85, 0.85), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.9, 0.9, 0.9, 0.9), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.95, 0.95, 0.95, 0.95), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.0, 1.0, 1.0, 1.0), material.diffuse());
    material.resetMorph();
    // add (0.6)
    morph.diffuse.setValue(0.6, 0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.diffuse());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.95, 0.95, 0.95, 0.95), material.diffuse());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(1.1, 1.1, 1.1, 1.1), material.diffuse());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(1.25, 1.25, 1.25, 1.25), material.diffuse());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.4, 1.4, 1.4, 1.4), material.diffuse());
}

TEST(Material, MergeSpecularColor)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.specular.setValue(1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setSpecular(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.resetMorph();
    // mod (0.0)
    morph.specular.setValue(0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.6, 0.6, 0.6, 1.0), material.specular());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.4, 0.4, 0.4, 1.0), material.specular());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.2, 0.2, 0.2, 1.0), material.specular());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.0, 0.0, 0.0, 1.0), material.specular());
    material.resetMorph();
    // add (0.2)
    morph.specular.setValue(0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.85, 0.85, 0.85, 1.0), material.specular());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.9, 0.9, 0.9, 1.0), material.specular());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.95, 0.95, 0.95, 1.0), material.specular());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.0, 1.0, 1.0, 1.0), material.specular());
    material.resetMorph();
    // add (0.6)
    morph.specular.setValue(0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 1.0), material.specular());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.95, 0.95, 0.95, 1.0), material.specular());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(1.1, 1.1, 1.1, 1.0), material.specular());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(1.25, 1.25, 1.25, 1.0), material.specular());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.4, 1.4, 1.4, 1.0), material.specular());
}

TEST(Material, MergeShininess)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.shininess = 1.0;
    morph.operation = 0;
    material.setShininess(0.8);
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.resetMorph();
    // mod (0.0)
    morph.shininess = 0.0;
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.shininess(), 0.6f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.shininess(), 0.4f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.shininess(), 0.2f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.shininess(), 0.0f);
    material.resetMorph();
    // add (0.2)
    morph.shininess = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    EXPECT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.shininess(), 0.85f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.shininess(), 0.9f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.shininess(), 0.95f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.shininess(), 1.0f);
    // add (0.6)
    morph.shininess = 0.6;
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.shininess(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.shininess(), 0.95f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.shininess(), 1.1f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.shininess(), 1.25f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.shininess(), 1.4f);
}

TEST(Material, MergeEdgeColor)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.edgeColor.setValue(1.0, 1.0, 1.0, 1.0);
    morph.operation = 0;
    material.setEdgeColor(Color(0.8, 0.8, 0.8, 0.8));
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.resetMorph();
    // mod (0.0)
    morph.edgeColor.setValue(0.0, 0.0, 0.0, 0.0);
    morph.operation = 0;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.6, 0.6, 0.6, 0.6), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.4, 0.4, 0.4, 0.4), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.2, 0.2, 0.2, 0.2), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(0.0, 0.0, 0.0, 0.0), material.edgeColor());
    material.resetMorph();
    // add (0.2)
    morph.edgeColor.setValue(0.2, 0.2, 0.2, 0.2);
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.85, 0.85, 0.85, 0.85), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(0.9, 0.9, 0.9, 0.9), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(0.95, 0.95, 0.95, 0.95), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.0, 1.0, 1.0, 1.0), material.edgeColor());
    material.resetMorph();
    // add (0.6)
    morph.edgeColor.setValue(0.6, 0.6, 0.6, 0.6);
    material.mergeMorph(&morph, 0.0);
    Compare(Color(0.8, 0.8, 0.8, 0.8), material.edgeColor());
    material.mergeMorph(&morph, 0.25);
    Compare(Color(0.95, 0.95, 0.95, 0.95), material.edgeColor());
    material.mergeMorph(&morph, 0.5);
    Compare(Color(1.1, 1.1, 1.1, 1.1), material.edgeColor());
    material.mergeMorph(&morph, 0.75);
    Compare(Color(1.25, 1.25, 1.25, 1.25), material.edgeColor());
    material.mergeMorph(&morph, 1.0);
    Compare(Color(1.4, 1.4, 1.4, 1.4), material.edgeColor());
}

TEST(Material, MergeEdgeSize)
{
    Material material;
    Morph::Material morph;
    // mod (1.0)
    morph.edgeSize = 1.0;
    morph.operation = 0;
    material.setEdgeSize(0.8);
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.resetMorph();
    // mod (1.0)
    morph.edgeSize = 0.0;
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.6f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.4f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.2f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.0f);
    material.resetMorph();
    // add (0.2)
    morph.edgeSize = 0.2;
    morph.operation = 1;
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.85f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.9f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.95f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 1.0f);
    // add (0.6)
    morph.edgeSize = 0.6;
    material.mergeMorph(&morph, 0.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.8f);
    material.mergeMorph(&morph, 0.25);
    EXPECT_FLOAT_EQ(material.edgeSize(), 0.95f);
    material.mergeMorph(&morph, 0.5);
    EXPECT_FLOAT_EQ(material.edgeSize(), 1.1f);
    material.mergeMorph(&morph, 0.75);
    EXPECT_FLOAT_EQ(material.edgeSize(), 1.25f);
    material.mergeMorph(&morph, 1.0);
    EXPECT_FLOAT_EQ(material.edgeSize(), 1.4f);
}

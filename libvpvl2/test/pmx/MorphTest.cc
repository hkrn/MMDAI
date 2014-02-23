#include "Common.h"

TEST_P(PMXFragmentTest, ReadWriteBoneMorph)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Morph expected(&model), actual(&model);
    MockIBone mockBone;
    EXPECT_CALL(mockBone, parentModelRef()).Times(AnyNumber()).WillRepeatedly(Return(&model));
    std::unique_ptr<Morph::Bone> bone1(new Morph::Bone()), bone2(new Morph::Bone());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.boneIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // bone morph1
    bone1->bone = &mockBone;
    bone1->index = 0;
    bone1->position.setValue(0.11, 0.12, 0.13);
    bone1->rotation.setValue(0.21, 0.22, 0.23, 0.24);
    expected.addBoneMorph(bone1.get());
    // bone morph2
    bone2->bone = &mockBone;
    bone2->index = 1;
    bone2->position.setValue(0.31, 0.32, 0.33);
    bone2->rotation.setValue(0.41, 0.42, 0.43, 0.44);
    expected.addBoneMorph(bone2.get());
    Array<IMorph::Bone *> boneMorphs;
    expected.getBoneMorphs(boneMorphs);
    ASSERT_EQ(2, boneMorphs.count());
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setCategory(IMorph::kEyeblow);
    expected.setType(pmx::Morph::kBoneMorph);
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(actual.name(IEncoding::kJapanese)->equals(expected.name(IEncoding::kJapanese)));
    ASSERT_TRUE(actual.name(IEncoding::kEnglish)->equals(expected.name(IEncoding::kEnglish)));
    ASSERT_EQ(expected.category(), actual.category());
    ASSERT_EQ(expected.type(), actual.type());
    const Array<Morph::Bone *> &bones = actual.bones();
    ASSERT_EQ(bones.count(), 2);
    ASSERT_TRUE(CompareVector(bone1->position, bones[0]->position));
    ASSERT_TRUE(CompareVector(bone1->rotation, bones[0]->rotation));
    ASSERT_EQ(bone1->index, bones[0]->index);
    ASSERT_TRUE(CompareVector(bone2->position, bones[1]->position));
    ASSERT_TRUE(CompareVector(bone2->rotation, bones[1]->rotation));
    ASSERT_EQ(bone2->index, bones[1]->index);
    expected.removeBoneMorph(bone2.get());
    expected.removeBoneMorph(bone1.get());
    expected.getBoneMorphs(boneMorphs);
    ASSERT_EQ(0, boneMorphs.count());
}

TEST_P(PMXFragmentTest, ReadWriteGroupMorph)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Morph expected(&model), actual(&model);
    MockIMorph mockMorph;
    EXPECT_CALL(mockMorph, parentModelRef()).Times(AnyNumber()).WillRepeatedly(Return(&model));
    std::unique_ptr<Morph::Group> group1(new Morph::Group()), group2(new Morph::Group());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.morphIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // group morph1
    group1->morph = &mockMorph;
    group1->index = 0;
    group1->fixedWeight = 0.1;
    expected.addGroupMorph(group1.get());
    // group morph2
    group2->morph = &mockMorph;
    group2->index = 1;
    group2->fixedWeight = 0.2;
    expected.addGroupMorph(group2.get());
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setCategory(IMorph::kEye);
    expected.setType(pmx::Morph::kGroupMorph);
    Array<IMorph::Group *> groupMorphs;
    expected.getGroupMorphs(groupMorphs);
    ASSERT_EQ(2, groupMorphs.count());
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(actual.name(IEncoding::kJapanese)->equals(expected.name(IEncoding::kJapanese)));
    ASSERT_TRUE(actual.name(IEncoding::kEnglish)->equals(expected.name(IEncoding::kEnglish)));
    ASSERT_EQ(expected.category(), actual.category());
    ASSERT_EQ(expected.type(), actual.type());
    const Array<Morph::Group *> &groups = actual.groups();
    ASSERT_EQ(groups.count(), 2);
    ASSERT_FLOAT_EQ(group1->fixedWeight, groups[0]->fixedWeight);
    ASSERT_EQ(group1->index, groups[0]->index);
    ASSERT_FLOAT_EQ(group2->fixedWeight, groups[1]->fixedWeight);
    ASSERT_EQ(group2->index, groups[1]->index);
    expected.removeGroupMorph(group1.get());
    expected.removeGroupMorph(group2.get());
    expected.getGroupMorphs(groupMorphs);
    ASSERT_EQ(0, groupMorphs.count());
}

TEST_P(PMXFragmentTest, ReadWriteMaterialMorph)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Morph expected(&model), actual(&model);
    MockIMaterial mockMaterial;
    EXPECT_CALL(mockMaterial, parentModelRef()).Times(AnyNumber()).WillRepeatedly(Return(&model));
    std::unique_ptr<Morph::Material> material1(new Morph::Material()), material2(new Morph::Material());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.materialIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // material morph1
    material1->index = 0;
    material1->materials = new Array<IMaterial *>();
    material1->materials->append(&mockMaterial);
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
    expected.addMaterialMorph(material1.get());
    material2->index = 1;
    material2->materials = new Array<IMaterial *>();
    material2->materials->append(&mockMaterial);
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
    expected.addMaterialMorph(material2.get());
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setCategory(IMorph::kLip);
    expected.setType(pmx::Morph::kMaterialMorph);
    Array<IMorph::Material *> materialMorphs;
    expected.getMaterialMorphs(materialMorphs);
    ASSERT_EQ(2, materialMorphs.count());
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(actual.name(IEncoding::kJapanese)->equals(expected.name(IEncoding::kJapanese)));
    ASSERT_TRUE(actual.name(IEncoding::kEnglish)->equals(expected.name(IEncoding::kEnglish)));
    ASSERT_EQ(expected.category(), actual.category());
    ASSERT_EQ(expected.type(), actual.type());
    const Array<Morph::Material *> &materials = actual.materials();
    ASSERT_EQ(materials.count(), 2);
    ASSERT_TRUE(CompareVector(material1->ambient, materials[0]->ambient));
    ASSERT_TRUE(CompareVector(material1->diffuse, materials[0]->diffuse));
    ASSERT_TRUE(CompareVector(material1->specular, materials[0]->specular));
    ASSERT_TRUE(CompareVector(material1->edgeColor, materials[0]->edgeColor));
    ASSERT_TRUE(CompareVector(material1->sphereTextureWeight, materials[0]->sphereTextureWeight));
    ASSERT_TRUE(CompareVector(material1->textureWeight, materials[0]->textureWeight));
    ASSERT_TRUE(CompareVector(material1->toonTextureWeight, materials[0]->toonTextureWeight));
    ASSERT_FLOAT_EQ(material1->edgeSize, materials[0]->edgeSize);
    ASSERT_FLOAT_EQ(material1->shininess, materials[0]->shininess);
    ASSERT_EQ(material1->operation, materials[0]->operation);
    ASSERT_EQ(material1->index, materials[0]->index);
    ASSERT_TRUE(CompareVector(material2->ambient, materials[1]->ambient));
    ASSERT_TRUE(CompareVector(material2->diffuse, materials[1]->diffuse));
    ASSERT_TRUE(CompareVector(material2->specular, materials[1]->specular));
    ASSERT_TRUE(CompareVector(material2->edgeColor, materials[1]->edgeColor));
    ASSERT_TRUE(CompareVector(material2->sphereTextureWeight, materials[1]->sphereTextureWeight));
    ASSERT_TRUE(CompareVector(material2->textureWeight, materials[1]->textureWeight));
    ASSERT_TRUE(CompareVector(material2->toonTextureWeight, materials[1]->toonTextureWeight));
    ASSERT_FLOAT_EQ(material2->edgeSize, materials[1]->edgeSize);
    ASSERT_FLOAT_EQ(material2->shininess, materials[1]->shininess);
    ASSERT_EQ(material2->operation, materials[1]->operation);
    ASSERT_EQ(material2->index, materials[1]->index);
    expected.removeMaterialMorph(material1.get());
    expected.removeMaterialMorph(material2.get());
    expected.getMaterialMorphs(materialMorphs);
    ASSERT_EQ(0, materialMorphs.count());
}

TEST_P(PMXFragmentTest, ReadWriteVertexMorph)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Morph expected(&model), actual(&model);
    MockIVertex mockVertex;
    EXPECT_CALL(mockVertex, parentModelRef()).Times(AnyNumber()).WillRepeatedly(Return(&model));
    std::unique_ptr<Morph::Vertex> vertex1(new Morph::Vertex()), vertex2(new Morph::Vertex());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // vertex morph1
    vertex1->vertex = &mockVertex;
    vertex1->index = 0;
    vertex1->position.setValue(0.1, 0.2, 0.3);
    expected.addVertexMorph(vertex1.get());
    // vertex morph2
    vertex2->vertex = &mockVertex;
    vertex2->index = 1;
    vertex2->position.setValue(0.4, 0.5, 0.6);
    expected.addVertexMorph(vertex2.get());
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setCategory(IMorph::kOther);
    expected.setType(pmx::Morph::kVertexMorph);
    Array<IMorph::Vertex *> vertexMorphs;
    expected.getVertexMorphs(vertexMorphs);
    ASSERT_EQ(2, vertexMorphs.count());
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(actual.name(IEncoding::kJapanese)->equals(expected.name(IEncoding::kJapanese)));
    ASSERT_TRUE(actual.name(IEncoding::kEnglish)->equals(expected.name(IEncoding::kEnglish)));
    ASSERT_EQ(expected.category(), actual.category());
    ASSERT_EQ(expected.type(), actual.type());
    const Array<Morph::Vertex *> &vertices = actual.vertices();
    ASSERT_EQ(vertices.count(), 2);
    ASSERT_TRUE(CompareVector(vertex1->position, vertices[0]->position));
    ASSERT_EQ(vertex1->index, vertices[0]->index);
    ASSERT_TRUE(CompareVector(vertex2->position, vertices[1]->position));
    ASSERT_EQ(vertex2->index, vertices[1]->index);
    expected.removeVertexMorph(vertex1.get());
    expected.removeVertexMorph(vertex2.get());
    expected.getVertexMorphs(vertexMorphs);
    ASSERT_EQ(0, vertexMorphs.count());
}

TEST_P(PMXFragmentWithUVTest, ReadWriteUVMorph)
{
    vsize indexSize = get<0>(GetParam());
    pmx::Morph::Type type = get<1>(GetParam());
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Morph expected(&model), actual(&model);
    MockIVertex mockVertex;
    EXPECT_CALL(mockVertex, parentModelRef()).Times(AnyNumber()).WillRepeatedly(Return(&model));
    std::unique_ptr<Morph::UV> uv1(new Morph::UV()), uv2(new Morph::UV());
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.vertexIndexSize = indexSize;
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    // UV morph1
    uv1->vertex = &mockVertex;
    uv1->index = 0;
    uv1->position.setValue(0.1, 0.2, 0.3, 0.4);
    expected.addUVMorph(uv1.get());
    // UV morph2
    uv2->vertex = &mockVertex;
    uv2->index = 1;
    uv2->position.setValue(0.5, 0.6, 0.7, 0.8);
    expected.addUVMorph(uv2.get());
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setCategory(IMorph::kOther);
    expected.setType(type);
    Array<IMorph::UV *> uvMorphs;
    expected.getUVMorphs(uvMorphs);
    ASSERT_EQ(2, uvMorphs.count());
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    ASSERT_EQ(size, read);
    ASSERT_TRUE(actual.name(IEncoding::kJapanese)->equals(expected.name(IEncoding::kJapanese)));
    ASSERT_TRUE(actual.name(IEncoding::kEnglish)->equals(expected.name(IEncoding::kEnglish)));
    ASSERT_EQ(expected.category(), actual.category());
    ASSERT_EQ(expected.type(), actual.type());
    const Array<Morph::UV *> &uvs = actual.uvs();
    ASSERT_EQ(uvs.count(), 2);
    ASSERT_TRUE(CompareVector(uv1->position, uvs[0]->position));
    ASSERT_EQ(type - pmx::Morph::kTexCoordMorph, uvs[0]->offset);
    ASSERT_EQ(uv1->index, uvs[0]->index);
    ASSERT_TRUE(CompareVector(uv2->position, uvs[1]->position));
    ASSERT_EQ(type - pmx::Morph::kTexCoordMorph, uvs[1]->offset);
    ASSERT_EQ(uv2->index, uvs[1]->index);
    expected.removeUVMorph(uv1.get());
    expected.removeUVMorph(uv2.get());
    expected.getUVMorphs(uvMorphs);
    ASSERT_EQ(0, uvMorphs.count());
}

TEST(PMXModelTest, AddAndRemoveMorph)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IMorph> morph(model.createMorph());
    ASSERT_EQ(-1, morph->index());
    model.addMorph(0); /* should not be crashed */
    model.addMorph(morph.get());
    model.addMorph(morph.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.morphs().count());
    ASSERT_EQ(morph.get(), model.findMorphRefAt(0));
    ASSERT_EQ(morph->index(), model.findMorphRefAt(0)->index());
    model.removeMorph(0); /* should not be crashed */
    model.removeMorph(morph.get());
    ASSERT_EQ(0, model.morphs().count());
    ASSERT_EQ(-1, morph->index());
    MockIMorph mockedMorph;
    EXPECT_CALL(mockedMorph, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMorph, name(_)).Times(2).WillRepeatedly(Return(static_cast<const IString *>(0))); /* should not be crashed */
    EXPECT_CALL(mockedMorph, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMorph(&mockedMorph);
    ASSERT_EQ(0, model.morphs().count());
}

TEST_P(PMXLanguageTest, RenameMorph)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IMorph> morph(model.createMorph());
    String oldName("OldBoneName"), newName("NewBoneName");
    IEncoding::LanguageType language = GetParam();
    morph->setName(&oldName, language);
    model.addMorph(morph.get());
    ASSERT_EQ(morph.get(), model.findMorphRef(&oldName));
    ASSERT_EQ(0, model.findMorphRef(&newName));
    morph->setName(&newName, language);
    ASSERT_EQ(0, model.findMorphRef(&oldName));
    ASSERT_EQ(morph.get(), model.findMorphRef(&newName));
    morph.release();
}

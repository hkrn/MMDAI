#include "Common.h"

TEST_P(PMXFragmentTest, ReadWriteMaterial)
{
    vsize indexSize = GetParam();
    Encoding encoding(0);
    pmx::Model model(&encoding);
    Material expected(&model), actual(&model);
    Model::DataInfo info;
    String name("Japanese"), englishName("English");
    info.encoding = &encoding;
    info.codec = IString::kUTF8;
    info.textureIndexSize = indexSize;
    // construct material
    expected.setName(&name, IEncoding::kJapanese);
    expected.setName(&englishName, IEncoding::kEnglish);
    expected.setSphereTextureRenderMode(Material::kSubTexture);
    expected.setAmbient(Color(0.01, 0.02, 0.03, 1.0));
    expected.setDiffuse(Color(0.11, 0.12, 0.13, 0.14));
    expected.setSpecular(Color(0.21, 0.22, 0.23, 1.0));
    expected.setEdgeColor(Color(0.31, 0.32, 0.33, 0.34));
    expected.setShininess(0.1);
    expected.setEdgeSize(0.2);
    IMaterial::IndexRange expectedRange;
    expectedRange.count = 4;
    expected.setIndexRange(expectedRange);
    expected.setFlags(5);
    // write contructed material and read it
    vsize size = expected.estimateSize(info), read;
    std::unique_ptr<uint8[]> bytes(new uint8[size]);
    uint8 *ptr = bytes.get();
    expected.write(ptr, info);
    actual.read(bytes.get(), info, read);
    // compare read material
    ASSERT_EQ(size, read);
    ASSERT_TRUE(CompareMaterialInterface(expected, actual));
}

TEST_P(PMXFragmentTest, ReadWriteMaterialFlags)
{
    Material material(0);
    ASSERT_FALSE(material.isCullingDisabled());
    ASSERT_FALSE(material.isCastingShadowEnabled());
    ASSERT_FALSE(material.isCastingShadowMapEnabled());
    ASSERT_FALSE(material.isShadowMapEnabled());
    ASSERT_FALSE(material.isEdgeEnabled());
    ASSERT_FALSE(material.isVertexColorEnabled());
    ASSERT_FALSE(material.isPointDrawEnabled());
    ASSERT_FALSE(material.isLineDrawEnabled());
    material.setFlags(IMaterial::kDisableCulling);
    ASSERT_TRUE(material.isCullingDisabled());
    material.setFlags(IMaterial::kCastingShadow);
    ASSERT_TRUE(material.isCastingShadowEnabled());
    material.setFlags(IMaterial::kCastingShadowMap);
    ASSERT_TRUE(material.isCastingShadowMapEnabled());
    material.setFlags(IMaterial::kEnableShadowMap);
    ASSERT_TRUE(material.isShadowMapEnabled());
    material.setFlags(IMaterial::kEnableEdge);
    ASSERT_TRUE(material.isEdgeEnabled());
    material.setFlags(IMaterial::kEnableVertexColor);
    ASSERT_TRUE(material.isVertexColorEnabled());
    material.setFlags(IMaterial::kEnablePointDraw);
    ASSERT_TRUE(material.isPointDrawEnabled());
    material.setFlags(IMaterial::kEnableLineDraw);
    ASSERT_TRUE(material.isLineDrawEnabled());
    material.setFlags(IMaterial::kCastingShadow | IMaterial::kEnablePointDraw);
    ASSERT_FALSE(material.isCastingShadowEnabled());
    material.setFlags(IMaterial::kCastingShadow | IMaterial::kEnablePointDraw);
    ASSERT_FALSE(material.isCastingShadowMapEnabled());
    material.setFlags(IMaterial::kEnableShadowMap | IMaterial::kEnablePointDraw);
    ASSERT_FALSE(material.isShadowMapEnabled());
    material.setFlags(IMaterial::kEnableVertexColor | IMaterial::kEnablePointDraw | IMaterial::kEnableLineDraw);
    ASSERT_FALSE(material.isEdgeEnabled());
}

TEST(PMXMaterialTest, MergeAmbientColor)
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

TEST(PMXMaterialTest, MergeDiffuseColor)
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

TEST(PMXMaterialTest, MergeSpecularColor)
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

TEST(PMXMaterialTest, MergeShininess)
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

TEST(PMXMaterialTest, MergeEdgeColor)
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

TEST(PMXMaterialTest, MergeEdgeSize)
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

TEST(PMXModelTest, AddAndRemoveMaterial)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<IMaterial> material(model.createMaterial());
    ASSERT_EQ(-1, material->index());
    model.addMaterial(0); /* should not be crashed */
    model.addMaterial(material.get());
    model.addMaterial(material.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.materials().count());
    ASSERT_EQ(material.get(), model.findMaterialRefAt(0));
    ASSERT_EQ(material->index(), model.findMaterialRefAt(0)->index());
    model.removeMaterial(0); /* should not be crashed */
    model.removeMaterial(material.get());
    ASSERT_EQ(0, model.materials().count());
    ASSERT_EQ(-1, material->index());
    MockIMaterial mockedMaterial;
    EXPECT_CALL(mockedMaterial, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedMaterial, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMaterial(&mockedMaterial);
    ASSERT_EQ(0, model.materials().count());
}

TEST(PMXModelTest, RemoveMaterialReferences)
{
    Encoding encoding(0);
    Model model(&encoding);
    Material material(&model);
    model.addMaterial(&material);
    Vertex vertex(&model);
    vertex.setMaterialRef(&material);
    model.addVertex(&vertex);
    Morph::Material materialMorph;
    materialMorph.materials = new Array<IMaterial *>();
    materialMorph.materials->append(&material);
    Morph parentMaterialMorph(&model);
    parentMaterialMorph.setType(IMorph::kMaterialMorph);
    parentMaterialMorph.addMaterialMorph(&materialMorph);
    model.addMorph(&parentMaterialMorph);
    model.removeMaterial(&material);
    model.removeVertex(&vertex);
    parentMaterialMorph.removeMaterialMorph(&materialMorph);
    model.removeMorph(&parentMaterialMorph);
    ASSERT_EQ(Factory::sharedNullMaterialRef(), vertex.materialRef());
    ASSERT_EQ(0, materialMorph.materials->count());
}

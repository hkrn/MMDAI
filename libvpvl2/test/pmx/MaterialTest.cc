#include "Common.h"

TEST(PMXPropertyEventListener, HandleMaterialPropertyEvents)
{
    Model model(0); /* not to crash at setting texture */
    Material material(&model);
    MockMaterialPropertyEventListener listener;
    TestHandleEvents<IMaterial::PropertyEventListener>(listener, material);
    Color ambient(0.11, 0.12, 0.13, 1), diffuse(0.14, 0.15, 0.16, 1), edgeColor(0.17, 0.18, 0.19, 1), specular(0.20, 0.21, 0.22, 1);
    IVertex::EdgeSizePrecision edgeSize(0.1);
    IMaterial::IndexRange indexRange; indexRange.count = 3, indexRange.end = 4, indexRange.start = 1;
    IMaterial::SphereTextureRenderMode renderMode(IMaterial::kSubTexture);
    String mainTexture("MainTexture"), japaneseName("Japanese Name"), englishName("English Name"),
            sphereTexture("SphereTexture"), toonTexture("ToonTexture"), userDataArea("UserDataArea");
    Scalar shininess(0.2);
    int flags = 16;
    EXPECT_CALL(listener, ambientWillChange(ambient, &material)).WillOnce(Return());
    EXPECT_CALL(listener, diffuseWillChange(diffuse, &material)).WillOnce(Return());
    EXPECT_CALL(listener, edgeColorWillChange(edgeColor, &material)).WillOnce(Return());
    EXPECT_CALL(listener, edgeSizeWillChange(edgeSize, &material)).WillOnce(Return());
    EXPECT_CALL(listener, flagsWillChange(flags, &material)).WillOnce(Return());
    EXPECT_CALL(listener, indexRangeWillChange(indexRange, &material)).WillOnce(Return());
    EXPECT_CALL(listener, mainTextureWillChange(&mainTexture, &material)).WillOnce(Return());
    EXPECT_CALL(listener, mainTextureWillChange(0, &material)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &material)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &material)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &material)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &material)).WillOnce(Return());
    EXPECT_CALL(listener, shininessWillChange(shininess, &material)).WillOnce(Return());
    EXPECT_CALL(listener, specularWillChange(specular, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureRenderModeWillChange(renderMode, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureWillChange(&sphereTexture, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureWillChange(0, &material)).WillOnce(Return());
    EXPECT_CALL(listener, toonTextureWillChange(&toonTexture, &material)).WillOnce(Return());
    EXPECT_CALL(listener, toonTextureWillChange(0, &material)).WillOnce(Return());
    EXPECT_CALL(listener, userDataAreaWillChange(&userDataArea, &material)).WillOnce(Return());
    EXPECT_CALL(listener, userDataAreaWillChange(0, &material)).WillOnce(Return());
    EXPECT_CALL(listener, visibleWillChange(false, &material)).WillOnce(Return());
    material.addEventListenerRef(&listener);
    material.setAmbient(ambient);
    material.setAmbient(ambient);
    material.setDiffuse(diffuse);
    material.setDiffuse(diffuse);
    material.setEdgeColor(edgeColor);
    material.setEdgeColor(edgeColor);
    material.setEdgeSize(edgeSize);
    material.setEdgeSize(edgeSize);
    material.setFlags(flags);
    material.setFlags(flags);
    material.setIndexRange(indexRange);
    material.setIndexRange(indexRange);
    material.setMainTexture(&mainTexture);
    material.setMainTexture(&mainTexture);
    material.setMainTexture(0);
    material.setMainTexture(0);
    material.setName(&japaneseName, IEncoding::kJapanese);
    material.setName(&japaneseName, IEncoding::kJapanese);
    material.setName(0, IEncoding::kJapanese);
    material.setName(0, IEncoding::kJapanese);
    material.setName(&englishName, IEncoding::kEnglish);
    material.setName(&englishName, IEncoding::kEnglish);
    material.setName(0, IEncoding::kEnglish);
    material.setName(0, IEncoding::kEnglish);
    material.setShininess(shininess);
    material.setShininess(shininess);
    material.setSpecular(specular);
    material.setSpecular(specular);
    material.setSphereTexture(&sphereTexture);
    material.setSphereTexture(&sphereTexture);
    material.setSphereTexture(0);
    material.setSphereTexture(0);
    material.setSphereTextureRenderMode(renderMode);
    material.setSphereTextureRenderMode(renderMode);
    material.setToonTexture(&toonTexture);
    material.setToonTexture(&toonTexture);
    material.setToonTexture(0);
    material.setToonTexture(0);
    material.setUserDataArea(&userDataArea);
    material.setUserDataArea(&userDataArea);
    material.setUserDataArea(0);
    material.setUserDataArea(0);
    material.setVisible(false);
    material.setVisible(false);
}

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

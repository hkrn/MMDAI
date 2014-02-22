#include "Common.h"

TEST(PMDPropertyEventListener, HandleMaterialPropertyEvents)
{
    Model model(0); /* not to crash at setting texture */
    Material material(&model, 0);
    MockMaterialPropertyEventListener listener;
    TestHandleEvents<IMaterial::PropertyEventListener>(listener, material);
    Color ambient(0.11, 0.12, 0.13, 1), diffuse(0.14, 0.15, 0.16, 1), edgeColor(0.17, 0.18, 0.19, 1), specular(0.20, 0.21, 0.22, 1);
    IVertex::EdgeSizePrecision edgeSize(0.1);
    IMaterial::IndexRange indexRange; indexRange.count = 3, indexRange.end = 4, indexRange.start = 1;
    IMaterial::SphereTextureRenderMode renderMode(IMaterial::kSubTexture);
    Scalar shininess(0.2);
    int flags = 16;
    /* name/flags/edgeSize/userData should not be called */
    EXPECT_CALL(listener, ambientWillChange(ambient, &material)).WillOnce(Return());
    EXPECT_CALL(listener, diffuseWillChange(diffuse, &material)).WillOnce(Return());
    EXPECT_CALL(listener, edgeColorWillChange(edgeColor, &material)).WillOnce(Return());
    EXPECT_CALL(listener, indexRangeWillChange(indexRange, &material)).WillOnce(Return());
    EXPECT_CALL(listener, mainTextureWillChange(_, &material)).WillOnce(Return());
    EXPECT_CALL(listener, shininessWillChange(shininess, &material)).WillOnce(Return());
    EXPECT_CALL(listener, specularWillChange(specular, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureRenderModeWillChange(renderMode, &material)).WillOnce(Return());
    EXPECT_CALL(listener, sphereTextureWillChange(_, &material)).WillOnce(Return());
    EXPECT_CALL(listener, toonTextureWillChange(_, &material)).WillOnce(Return());
    EXPECT_CALL(listener, visibleWillChange(false, &material)).WillOnce(Return());
    String mainTexture("MainTexture"), japaneseName("Japanese Name"), englishName("English Name"),
            sphereTexture("SphereTexture"), toonTexture("ToonTexture"), userDataArea("UserDataArea");
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
    material.setSphereTextureRenderMode(renderMode);
    material.setSphereTextureRenderMode(renderMode);
    material.setToonTexture(&toonTexture);
    material.setToonTexture(&toonTexture);
    material.setUserDataArea(&userDataArea);
    material.setUserDataArea(&userDataArea);
    material.setVisible(false);
    material.setVisible(false);
}

TEST(PMDModelTest, AddAndRemoveMaterial)
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

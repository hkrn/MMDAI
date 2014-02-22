#include "Common.h"

TEST(PMDPropertyEventListener, HandleModelPropertyEvents)
{
    Model model(0), parentModel(0);
    MockModelPropertyEventListener listener;
    TestHandleEvents<IModel::PropertyEventListener>(listener, model);
    Color edgeColor(0.1, 0.2, 0.3, 1.0);
    Vector3 aabbMin(1, 2, 3), aabbMax(4, 5, 6), position(7, 8, 9);
    Quaternion rotation(0.4, 0.5, 0.6, 1);
    IVertex::EdgeSizePrecision edgeSize(0.4);
    Scalar opacity(0.5), scaleFactor(0.6), version(2.1);
    Bone parentBone(0, 0);
    String japaneseName("Japanese Name"), englishName("English Name"),
            japaneseComment("Japanese Comment"), englishComemnt("English Comment");
    bool physics = !model.isPhysicsEnabled(), visible = !model.isVisible();
    /* version should not be called */
    EXPECT_CALL(listener, aabbWillChange(aabbMin, aabbMax, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(&japaneseComment, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(0, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(&englishComemnt, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, commentWillChange(0, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, edgeColorWillChange(edgeColor, &model)).WillOnce(Return());
    EXPECT_CALL(listener, edgeWidthWillChange(edgeSize, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &model)).WillOnce(Return());
    EXPECT_CALL(listener, opacityWillChange(opacity, &model)).WillOnce(Return());
    EXPECT_CALL(listener, parentBoneRefWillChange(&parentBone, &model)).WillOnce(Return());
    EXPECT_CALL(listener, parentModelRefWillChange(&parentModel, &model)).WillOnce(Return());
    EXPECT_CALL(listener, physicsEnableWillChange(physics, &model)).WillOnce(Return());
    EXPECT_CALL(listener, scaleFactorWillChange(scaleFactor, &model)).WillOnce(Return());
    EXPECT_CALL(listener, visibleWillChange(visible, &model)).WillOnce(Return());
    EXPECT_CALL(listener, worldTranslationWillChange(position, &model)).WillOnce(Return());
    EXPECT_CALL(listener, worldOrientationWillChange(rotation, &model)).WillOnce(Return());
    model.addEventListenerRef(&listener);
    model.setAabb(aabbMin, aabbMax);
    model.setAabb(aabbMin, aabbMax);
    model.setComment(&japaneseComment, IEncoding::kJapanese);
    model.setComment(&japaneseComment, IEncoding::kJapanese);
    model.setComment(0, IEncoding::kJapanese);
    model.setComment(0, IEncoding::kJapanese);
    model.setComment(&englishComemnt, IEncoding::kEnglish);
    model.setComment(&englishComemnt, IEncoding::kEnglish);
    model.setComment(0, IEncoding::kEnglish);
    model.setComment(0, IEncoding::kEnglish);
    model.setEdgeColor(edgeColor);
    model.setEdgeColor(edgeColor);
    model.setEdgeWidth(edgeSize);
    model.setEdgeWidth(edgeSize);
    model.setName(&japaneseName, IEncoding::kJapanese);
    model.setName(&japaneseName, IEncoding::kJapanese);
    model.setName(0, IEncoding::kJapanese);
    model.setName(0, IEncoding::kJapanese);
    model.setName(&englishName, IEncoding::kEnglish);
    model.setName(&englishName, IEncoding::kEnglish);
    model.setName(0, IEncoding::kEnglish);
    model.setName(0, IEncoding::kEnglish);
    model.setOpacity(opacity);
    model.setOpacity(opacity);
    model.setParentBoneRef(&parentBone);
    model.setParentBoneRef(&parentBone);
    model.setParentModelRef(&parentModel);
    model.setParentModelRef(&parentModel);
    model.setPhysicsEnable(physics);
    model.setPhysicsEnable(physics);
    model.setScaleFactor(scaleFactor);
    model.setScaleFactor(scaleFactor);
    model.setVersion(version);
    model.setVersion(version);
    model.setVisible(visible);
    model.setVisible(visible);
    model.setWorldTranslation(position);
    model.setWorldTranslation(position);
    model.setWorldOrientation(rotation);
    model.setWorldOrientation(rotation);
}

TEST(PMDModelTest, UnknownLanguageTest)
{
    Encoding encoding(0);
    Model model(&encoding);
    String name("This is a name."), comment("This is a comment.");
    model.setName(&name, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.name(IEncoding::kUnknownLanguageType));
    model.setComment(&comment, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.comment(IEncoding::kUnknownLanguageType));
}

TEST(PMDModelTest, DefaultLanguageSameAsJapanese)
{
    Encoding encoding(0);
    Model model(&encoding);
    String name1("This is a Japanese name type 1."),
            name2("This is a Japanese name type 2."),
            comment1("This is a comment type 1."),
            comment2("This is a comment type 2.");
    model.setName(&name1, IEncoding::kJapanese);
    ASSERT_TRUE(model.name(IEncoding::kDefaultLanguage)->equals(&name1));
    model.setName(&name2, IEncoding::kDefaultLanguage);
    ASSERT_TRUE(model.name(IEncoding::kJapanese)->equals(&name2));
    model.setComment(&comment1, IEncoding::kJapanese);
    ASSERT_TRUE(model.comment(IEncoding::kDefaultLanguage)->equals(&comment1));
    model.setComment(&comment2, IEncoding::kDefaultLanguage);
    ASSERT_TRUE(model.comment(IEncoding::kJapanese)->equals(&comment2));
}

TEST_P(PMDLanguageTest, ReadWriteName)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a name.");
    model.setName(&s, language);
    ASSERT_TRUE(model.name(language)->equals(&s));
}

TEST_P(PMDLanguageTest, ReadWriteComment)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a comment.");
    model.setComment(&s, language);
    ASSERT_TRUE(model.comment(language)->equals(&s));
}

TEST_P(PMDLanguageTest, RenameMorph)
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

TEST(PMDModelTest, ParseRealPMD)
{
    QFile file("miku.pmd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding::Dictionary dict;
        Encoding encoding(&dict);
        Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8 *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMDModel, model.type());
    }
    else {
        // skip
    }
}

INSTANTIATE_TEST_CASE_P(PMDModelInstance, PMDLanguageTest, Values(IEncoding::kDefaultLanguage,
                                                                  IEncoding::kJapanese,
                                                                  IEncoding::kEnglish));

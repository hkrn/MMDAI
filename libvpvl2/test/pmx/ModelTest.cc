#include "Common.h"

TEST(PMXPropertyEventListener, HandleModelPropertyEvents)
{
    Model model(0), parentModel(0);
    MockModelPropertyEventListener listener;
    TestHandleEvents<IModel::PropertyEventListener>(listener, model);
    Color edgeColor(0.1, 0.2, 0.3, 1.0);
    Vector3 aabbMin(1, 2, 3), aabbMax(4, 5, 6), position(7, 8, 9);
    Quaternion rotation(0.4, 0.5, 0.6, 1);
    IVertex::EdgeSizePrecision edgeSize(0.4);
    Scalar opacity(0.5), scaleFactor(0.6), version(2.1);
    Bone parentBone(0);
    String japaneseName("Japanese Name"), englishName("English Name"),
            japaneseComment("Japanese Comment"), englishComemnt("English Comment");
    bool physics = !model.isPhysicsEnabled(), visible = !model.isVisible();
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
    EXPECT_CALL(listener, versionWillChange(version, &model)).WillOnce(Return());
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

TEST(PMXModelTest, UnknownLanguageTest)
{
    Encoding encoding(0);
    Model model(&encoding);
    String name("This is a name."), comment("This is a comment.");
    model.setName(&name, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.name(IEncoding::kUnknownLanguageType));
    model.setComment(&comment, IEncoding::kUnknownLanguageType);
    ASSERT_EQ(0, model.comment(IEncoding::kUnknownLanguageType));
}

TEST(PMXModelTest, DefaultLanguageSameAsJapanese)
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

TEST_P(PMXLanguageTest, ReadWriteName)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a name.");
    model.setName(&s, language);
    ASSERT_TRUE(model.name(language)->equals(&s));
}

TEST_P(PMXLanguageTest, ReadWriteComment)
{
    Encoding encoding(0);
    Model model(&encoding);
    IEncoding::LanguageType language = GetParam();
    String s("This is a comment.");
    model.setComment(&s, language);
    ASSERT_TRUE(model.comment(language)->equals(&s));
}

TEST(PMXModelTest, ParseEmpty)
{
    Encoding encoding(0);
    Model model(&encoding);
    Model::DataInfo info;
    ASSERT_FALSE(model.preparse(reinterpret_cast<const uint8 *>(""), 0, info));
    ASSERT_EQ(Model::kInvalidHeaderError, model.error());
}

TEST(PMXModelTest, ParseRealPMX)
{
    QFile file("miku.pmx");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        Encoding::Dictionary dict;
        Encoding encoding(&dict);
        pmx::Model model(&encoding);
        EXPECT_TRUE(model.load(reinterpret_cast<const uint8 *>(bytes.constData()), bytes.size()));
        EXPECT_EQ(IModel::kNoError, model.error());
        EXPECT_EQ(IModel::kPMXModel, model.type());

        QByteArray bytes2;
        bytes2.resize(model.estimateSize());;
        vsize written;
        model.save(reinterpret_cast<uint8 *>(bytes2.data()), written);
        QFile file2(QDir::home().absoluteFilePath(QFileInfo(file.fileName()).fileName()));
        qDebug() << file2.fileName() << file.size() << model.estimateSize() << written;
        file2.open(QFile::WriteOnly);
        file2.write(bytes2);
        QFile file3(file2.fileName());
        file3.open(QFile::ReadOnly);
        const QByteArray &bytes3 = file3.readAll();
        pmx::Model model2(&encoding);
        qDebug() << "result:" << model2.load(reinterpret_cast<const uint8 *>(bytes3.constData()), bytes3.size())
                 << model2.error() << "estimated:" << model.estimateSize() << "actual:" << written;
    }
    else {
        // skip
    }
}

INSTANTIATE_TEST_CASE_P(PMXModelInstance, PMXFragmentTest, Values(1, 2, 4));
INSTANTIATE_TEST_CASE_P(PMXModelInstance, PMXFragmentWithUVTest, Combine(Values(1, 2, 4),
                                                                         Values(pmx::Morph::kTexCoordMorph,
                                                                                pmx::Morph::kUVA1Morph,
                                                                                pmx::Morph::kUVA2Morph,
                                                                                pmx::Morph::kUVA3Morph,
                                                                                pmx::Morph::kUVA4Morph)));
INSTANTIATE_TEST_CASE_P(PMXModelInstance, PMXLanguageTest, Values(IEncoding::kDefaultLanguage,
                                                                  IEncoding::kJapanese,
                                                                  IEncoding::kEnglish));

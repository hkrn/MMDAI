#include "Common.h"

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
#if 0
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
#endif
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

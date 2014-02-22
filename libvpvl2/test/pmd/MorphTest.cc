#include "Common.h"

TEST(PMDPropertyEventListener, HandleMorphPropertyEvents)
{
    pmd2::Model model(0);
    Morph morph(&model, 0);
    MockMorphPropertyEventListener listener;
    TestHandleEvents<IMorph::PropertyEventListener>(listener, morph);
    String japaneseName("Japanese"), englishName("English");
    IMorph::WeightPrecision weight(0.42);
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &morph)).WillOnce(Return());
    EXPECT_CALL(listener, weightWillChange(weight, &morph)).WillOnce(Return());
    morph.addEventListenerRef(&listener);
    morph.setName(&japaneseName, IEncoding::kJapanese);
    morph.setName(&japaneseName, IEncoding::kJapanese);
    morph.setName(0, IEncoding::kJapanese);
    morph.setName(0, IEncoding::kJapanese);
    morph.setName(&englishName, IEncoding::kEnglish);
    morph.setName(&englishName, IEncoding::kEnglish);
    morph.setName(0, IEncoding::kEnglish);
    morph.setName(0, IEncoding::kEnglish);
    morph.setWeight(weight);
    morph.setWeight(weight);
}

TEST(PMDModelTest, AddAndRemoveMorph)
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
    EXPECT_CALL(mockedMorph, name(_)).WillRepeatedly(Return(static_cast<const IString *>(0)));  /* should not be crashed */
    EXPECT_CALL(mockedMorph, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addMorph(&mockedMorph);
    ASSERT_EQ(0, model.morphs().count());
}

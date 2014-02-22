#include "Common.h"

TEST(PMXPropertyEventListener, HandleLabelPropertyEvents)
{
    Label label(0);
    MockLabelPropertyEventListener listener;
    TestHandleEvents<ILabel::PropertyEventListener>(listener, label);
    String japaneseName("Japanese"), englishName("English");
    EXPECT_CALL(listener, nameWillChange(&japaneseName, IEncoding::kJapanese, &label)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kJapanese, &label)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(&englishName, IEncoding::kEnglish, &label)).WillOnce(Return());
    EXPECT_CALL(listener, nameWillChange(0, IEncoding::kEnglish, &label)).WillOnce(Return());
    label.addEventListenerRef(&listener);
    label.setName(&japaneseName, IEncoding::kJapanese);
    label.setName(&japaneseName, IEncoding::kJapanese);
    label.setName(0, IEncoding::kJapanese);
    label.setName(0, IEncoding::kJapanese);
    label.setName(&englishName, IEncoding::kEnglish);
    label.setName(&englishName, IEncoding::kEnglish);
    label.setName(0, IEncoding::kEnglish);
    label.setName(0, IEncoding::kEnglish);
}

TEST(PMXModelTest, AddAndRemoveLabel)
{
    Encoding encoding(0);
    Model model(&encoding);
    std::unique_ptr<ILabel> label(model.createLabel());
    ASSERT_EQ(-1, label->index());
    model.addLabel(0); /* should not be crashed */
    model.addLabel(label.get());
    model.addLabel(label.get()); /* no effect because it's already added */
    ASSERT_EQ(1, model.labels().count());
    ASSERT_EQ(label.get(), model.findLabelRefAt(0));
    ASSERT_EQ(label->index(), model.findLabelRefAt(0)->index());
    model.removeLabel(0); /* should not be crashed */
    model.removeLabel(label.get());
    ASSERT_EQ(0, model.labels().count());
    ASSERT_EQ(-1, label->index());
    MockILabel mockedLabel;
    EXPECT_CALL(mockedLabel, index()).WillOnce(Return(-1));
    EXPECT_CALL(mockedLabel, parentModelRef()).WillOnce(Return(static_cast<IModel *>(0)));
    model.addLabel(&mockedLabel);
    ASSERT_EQ(0, model.labels().count());
}

#include "Common.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/extensions/icu4c/Encoding.h"

#define TO_STR_C(s) reinterpret_cast<const char *>(s)
#define TO_CSTRING(s) static_cast<const String *>(s)
#define TO_BYTES(s) TO_STR_C(TO_CSTRING(s)->toByteArray())

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

namespace {

static const char * GetCodecString(IString::Codec value)
{
    switch (value) {
    case IString::kUTF16:
        return "UTF-16";
    case IString::kUTF8:
        return "UTF-8";
    case IString::kShiftJIS:
        return "Shift-JIS";
    default:
        return "";
    }
}

class ConvertTest : public TestWithParam<IString::Codec> {};

}

TEST_P(ConvertTest, ToString)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const QString source("東京特許許可局局長");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(source.toUtf8().constData(), String::toStdString(TO_CSTRING(result.data())->value()).c_str());
}

TEST_P(ConvertTest, ToByteArray)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
#ifdef VPVL2_ICU_ENCODING_H_
    const String source(UnicodeString::fromUTF8("東京特許許可局局長"));
#else
    const String source("東京特許許可局局長");
#endif
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    uint8_t *stringInBytes = encoding.toByteArray(&source, codecEnum);
    const QString &s = QString::fromStdString(String::toStdString(source.value()));
    EXPECT_STREQ(codec->fromUnicode(s).constData(), TO_STR_C(stringInBytes));
    encoding.disposeByteArray(stringInBytes);
}

TEST(EncodingTest, StringConstant)
{
    Encoding encoding(0);
#if 0
    ASSERT_STREQ("左", TO_BYTES(encoding.stringConstant(IEncoding::kLeft)));
    ASSERT_STREQ("右", TO_BYTES(encoding.stringConstant(IEncoding::kRight)));
    ASSERT_STREQ("指", TO_BYTES(encoding.stringConstant(IEncoding::kFinger)));
    ASSERT_STREQ("ひじ", TO_BYTES(encoding.stringConstant(IEncoding::kElbow)));
    ASSERT_STREQ("腕", TO_BYTES(encoding.stringConstant(IEncoding::kArm)));
    ASSERT_STREQ("手首", TO_BYTES(encoding.stringConstant(IEncoding::kWrist)));
    ASSERT_STREQ("センター", TO_BYTES(encoding.stringConstant(IEncoding::kCenter)));
    ASSERT_STREQ("", TO_BYTES(encoding.stringConstant(static_cast<IEncoding::ConstantType>(-1))));
#endif
}

TEST(EncodingTest, ConvertNullToString)
{
    Encoding encoding(0);
    QScopedPointer<IString> result(encoding.toString(0, IString::kUTF8, 0));
    IString *s = result.data();
    ASSERT_TRUE(s);
    ASSERT_EQ(0, s->size());
    ASSERT_EQ(0, s->toByteArray()[0]);
}

TEST(EncodingTest, ConvertNullToByteArray)
{
    Encoding encoding(0);
    uint8_t *result = encoding.toByteArray(0, IString::kUTF8);
    ASSERT_TRUE(result);
    ASSERT_EQ(0, strlen(reinterpret_cast<const char *>(result)));
    ASSERT_EQ(0, result[0]);
    encoding.disposeByteArray(result);
}

// skip IString::kUTF16
INSTANTIATE_TEST_CASE_P(EncodingInstance, ConvertTest, Values(IString::kShiftJIS, IString::kUTF8));

#include "Common.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/extensions/icu4c/Encoding.h"

#define TO_STR_C(s) reinterpret_cast<const char *>(s)
#define TO_CSTRING(s) static_cast<const String *>(s.data())
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
    const QString source("いろはにほへとちりぬるをわかよたれそつねならむうゐのおくやまけふこえてあさきゆめみしゑひもせすん");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(source.toUtf8().constData(), String::toStdString(TO_CSTRING(result)->value()).c_str());
}

TEST_P(ConvertTest, ToStringWithHeadSpaces)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const QString source("   spaces"), expected("spaces");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(expected.toUtf8().constData(), String::toStdString(TO_CSTRING(result)->value()).c_str());
}

TEST_P(ConvertTest, ToStringWithTrailSpaces)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const QString source("spaces   "), expected("spaces");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(expected.toUtf8().constData(), String::toStdString(TO_CSTRING(result)->value()).c_str());
}

TEST_P(ConvertTest, ToStringWithHeadAndTrailSpaces)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const QString source("   spaces    "), expected("spaces");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(expected.toUtf8().constData(), String::toStdString(TO_CSTRING(result)->value()).c_str());
}

TEST_P(ConvertTest, ToStringWith0x1a)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const QString expected("spaces");
    QString source;
    source.append(QString("spaces"));
    source.append(QChar(0x1a));
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    const uint8_t *stringInBytes = reinterpret_cast<const uint8_t *>(bytes.constData());
    QScopedPointer<IString> result(encoding.toString(stringInBytes, codecEnum, bytes.length()));
    ASSERT_STREQ(expected.toUtf8().constData(), String::toStdString(TO_CSTRING(result)->value()).c_str());
}

TEST(ConvertTest, ToStringWithNull)
{
    Encoding encoding(0);
    QScopedPointer<IString> s(encoding.toString(0, IString::kMaxCodecType, 0));
    ASSERT_TRUE(s);
    ASSERT_EQ(0, s->size());
}

TEST(ConvertTest, ToStringWithInvalidCodec)
{
    Encoding encoding(0);
    const uint8_t source[] = "This is a test";
    ASSERT_EQ(0, encoding.toString(source, IString::kMaxCodecType, sizeof(source)));
    ASSERT_EQ(0, encoding.toString(source, sizeof(source), IString::kMaxCodecType));
}

TEST_P(ConvertTest, ToByteArray)
{
    Encoding encoding(0);
    const IString::Codec codecEnum = GetParam();
    const String source("いろはにほへとちりぬるをわかよたれそつねならむうゐのおくやまけふこえてあさきゆめみしゑひもせすん");
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    uint8_t *stringInBytes = encoding.toByteArray(&source, codecEnum);
    const QString &s = QString::fromStdString(String::toStdString(source.value()));
    EXPECT_STREQ(codec->fromUnicode(s).constData(), TO_STR_C(stringInBytes));
    encoding.disposeByteArray(stringInBytes);
}

TEST_P(ConvertTest, DetectCodec)
{
    Encoding encoding(0);
    const QString source("いろはにほへとちりぬるをわかよたれそつねならむうゐのおくやまけふこえてあさきゆめみしゑひもせすん");
    const IString::Codec codecEnum = GetParam();
    const QTextCodec *codec = QTextCodec::codecForName(GetCodecString(codecEnum));
    const QByteArray &bytes = codec->fromUnicode(source);
    ASSERT_EQ(codecEnum, encoding.detectCodec(bytes.constData(), bytes.length()));
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

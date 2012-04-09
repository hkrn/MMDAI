#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl2/Common.h>
#include <vpvl2/internal/util.h>

using namespace vpvl2;

namespace {

class String : public IString
{
public:
    explicit String(const QString &s) : m_bytes(s.toUtf8()), m_value(s) {
    }
    ~String() {
    }

    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(m_bytes.constData());
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const QString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_bytes.constData());
    }
    size_t length() const {
        return m_value.length();
    }

private:
    QByteArray m_bytes;
    QString m_value;
};

}

class TestInternal : public QObject
{
    Q_OBJECT

public:
    TestInternal();

private Q_SLOTS:
    void lerp();
    void size8();
    void size16();
    void size32();
    void stringEquals();
    void stringToFloat();
    void stringToInt();
    void zerofill();
    void clearAll();
    void version();
    void rad2deg();
    void deg2rad();
    // libvpvl2
    void sizeText();
    void readWriteSignedIndex8();
    void readWriteSignedIndex16();
    void readWriteSignedIndex32();
    void readWriteUnsignedIndex8();
    void readWriteUnsignedIndex16();
    void readWriteUnsignedIndex32();
    void setAndGetPosition();
    void setAndGetRotation();
    void writeNullString();
    void writeNotNullString();
    void estimateSize();
    void setString();
    void toggleFlag();
};

TestInternal::TestInternal()
{
}

void TestInternal::lerp()
{
    QCOMPARE(internal::lerp(4, 2, 0), 4.0f);
    QCOMPARE(internal::lerp(4, 2, 1), 2.0f);
    QCOMPARE(internal::lerp(4, 2, 0.5), 3.0f);
}

void TestInternal::size8()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    quint8 expected = 255;
    buffer.open(QBuffer::WriteOnly);
    stream << expected;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t rest = 0, actual = 0;
    // rest is not enough to read (0 < 1)
    QVERIFY(!internal::size8(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(0));
    rest = sizeof(quint8);
    // rest is now enough to read (1 = 1)
    QVERIFY(internal::size8(ptr, rest, actual));
    QCOMPARE(actual, size_t(expected));
    QCOMPARE(rest, size_t(0));
}

void TestInternal::size16()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    quint16 expected = 65535;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t rest = 1, actual = 0;
    // rest is not enough to read (1 < 2)
    QVERIFY(!internal::size16(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(1));
    rest = sizeof(quint16);
    // rest is now enough to read (2 = 2)
    QVERIFY(internal::size16(ptr, rest, actual));
    QCOMPARE(actual, size_t(expected));
    QCOMPARE(rest, size_t(0));
}

void TestInternal::size32()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    quint32 expected = INT_MAX;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t rest = 2, actual = 0;
    // rest is not enough to read (2 < 4)
    QVERIFY(!internal::size32(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(2));
    rest = sizeof(quint32);
    // rest is now enough to read (4 = 4)
    QVERIFY(internal::size32(ptr, rest, actual));
    QCOMPARE(actual, size_t(expected));
    QCOMPARE(rest, size_t(0));
}

void TestInternal::stringEquals()
{
    const char *foo = "foo", *bar = "bar";
    const uint8_t *baz = reinterpret_cast<const uint8_t *>(foo),
            *qux = reinterpret_cast<const uint8_t *>(bar);
    QVERIFY(internal::stringEquals(foo, foo, 3));
    QVERIFY(!internal::stringEquals(foo, bar, 3));
    QVERIFY(internal::stringEquals(baz, baz, 3));
    QVERIFY(!internal::stringEquals(baz, qux, 3));
}

void TestInternal::stringToInt()
{
    QCOMPARE(internal::stringToInt("42"), 42);
    QCOMPARE(internal::stringToInt("test"), 0);
}

void TestInternal::stringToFloat()
{
    QCOMPARE(internal::stringToFloat("4.2"), 4.2f);
    QCOMPARE(internal::stringToFloat("4.2f"), 4.2f);
    QCOMPARE(internal::stringToFloat("test"), 0.0f);
}

void TestInternal::zerofill()
{
    float src[] = { 1, 2, 3, 4, 5 };
    float dst[] = { 0, 0, 0, 0, 0 };
    internal::zerofill(src, sizeof(float) * 5);
    for (int i = 0; i < 5; i++) {
        QCOMPARE(src[i], dst[i]);
    }
}

void TestInternal::clearAll()
{
    Array<int *> array;
    array.add(new int(1));
    array.add(new int(2));
    array.add(new int(3));
    array.releaseAll();
    QCOMPARE(array.count(), 0);
    Hash<HashString, int*> hash;
    hash.insert(HashString("foo"), new int(1));
    hash.insert(HashString("bar"), new int(2));
    hash.insert(HashString("baz"), new int(3));
    hash.releaseAll();
    QCOMPARE(hash.count(), 0);
}

void TestInternal::version()
{
    QVERIFY(isLibraryVersionCorrect(VPVL2_VERSION));
    QVERIFY(!isLibraryVersionCorrect(
                VPVL2_MAKE_VERSION(VPVL2_VERSION_MAJOR - 1,
                                   VPVL2_VERSION_COMPAT,
                                   VPVL2_VERSION_MINOR)));
    QCOMPARE(libraryVersionString(), VPVL2_VERSION_STRING);
}

void TestInternal::rad2deg()
{
    QCOMPARE(180.0f, degree(radian(180.0f)));
}

void TestInternal::deg2rad()
{
    QCOMPARE(kPI, radian(degree(kPI)));
}

void TestInternal::sizeText()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    int expected = 4;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    stream << "test";
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data()), *text = 0;
    size_t rest = 4, size = 0;
    internal::sizeText(ptr, rest, text, size);
    QCOMPARE(size_t(0), rest);
    QCOMPARE(size_t(expected), size);
    QVERIFY(qstrncmp("test", reinterpret_cast<const char *>(text), expected));
}

void TestInternal::readWriteSignedIndex8()
{
    int8_t expected = INT8_MIN;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(INT8_MIN, internal::readSignedIndex(ptr, sizeof(expected)));
}

void TestInternal::readWriteSignedIndex16()
{
    int16_t expected = INT16_MIN;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(INT16_MIN, internal::readSignedIndex(ptr, sizeof(expected)));
}

void TestInternal::readWriteSignedIndex32()
{
    int expected = INT_MIN;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(INT_MIN, internal::readSignedIndex(ptr, sizeof(expected)));
}

void TestInternal::readWriteUnsignedIndex8()
{
    uint8_t expected = UINT8_MAX;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(UINT8_MAX, internal::readUnsignedIndex(ptr, sizeof(expected)));
}

void TestInternal::readWriteUnsignedIndex16()
{
    uint16_t expected = UINT16_MAX;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(UINT16_MAX, internal::readUnsignedIndex(ptr, sizeof(expected)));
}

void TestInternal::readWriteUnsignedIndex32()
{
    int expected = INT_MAX;
    QByteArray bytes;
    bytes.resize(sizeof(expected));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeSignedIndex(expected, sizeof(expected), data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(INT_MAX, internal::readUnsignedIndex(ptr, sizeof(expected)));
}

void TestInternal::setAndGetPosition()
{
    Vector3 v(0.1, 0.2, 0.3);
    float f[3];
    internal::getPosition(v, f);
    internal::setPosition(f, v);
    QCOMPARE(0.1f, v.x());
    QCOMPARE(0.2f, v.y());
    QCOMPARE(0.3f, btFabs(v.z()));
}

void TestInternal::setAndGetRotation()
{
    Quaternion q(0.1, 0.2, 0.3, 0.4);
    float f[4];
    internal::getRotation(q, f);
    internal::setRotation(f, q);
    QCOMPARE(0.1f, q.x());
    QCOMPARE(0.2f, btFabs(q.y()));
    QCOMPARE(0.3f, btFabs(q.z()));
    QCOMPARE(0.4f, q.w());
}

void TestInternal::writeNullString()
{
    QByteArray bytes;
    size_t size = internal::estimateSize(0);
    bytes.resize(size);
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeString(0, data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    QCOMPARE(0, internal::readSignedIndex(ptr, sizeof(int)));
}

void TestInternal::writeNotNullString()
{
    QByteArray bytes;
    String str("Hello World");
    bytes.resize(internal::estimateSize(&str));
    uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    internal::writeString(&str, data);
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t length = str.length();
    QCOMPARE(length, size_t(internal::readSignedIndex(ptr, sizeof(int))));
    QVERIFY(qstrncmp(reinterpret_cast<const char *>(str.toByteArray()), reinterpret_cast<const char *>(ptr), length) == 0);
}

void TestInternal::estimateSize()
{
    String str("Hello World");
    QCOMPARE(size_t(4), internal::estimateSize(0));
    QCOMPARE(size_t(4) + str.length(), internal::estimateSize(&str));
}

void TestInternal::setString()
{
    IString *value = 0;
    internal::setString(0, value);
    QCOMPARE(static_cast<IString*>(0), value);
    String str("Hello World");
    internal::setString(&str, value);
    QVERIFY(value != &str);
    QVERIFY(value->equals(&str));
}

void TestInternal::toggleFlag()
{
    uint16_t flag = 0;
    internal::toggleFlag(0x0002, true, flag);
    internal::toggleFlag(0x0010, true, flag);
    internal::toggleFlag(0x0400, true, flag);
    QCOMPARE(0x0412, int(flag));
    internal::toggleFlag(0x0002, false, flag);
    QCOMPARE(0x0410, int(flag));
    internal::toggleFlag(0x0010, false, flag);
    QCOMPARE(0x0400, int(flag));
    internal::toggleFlag(0x0400, false, flag);
    QCOMPARE(0x0000, int(flag));
}

QTEST_APPLESS_MAIN(TestInternal)

#include "TestInternal.moc"


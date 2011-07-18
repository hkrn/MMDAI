#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/internal/util.h>

class TestInternal : public QObject
{
    Q_OBJECT

public:
    TestInternal();

private Q_SLOTS:
    void lerp();
    void vector3();
    void vector4();
    void size8();
    void size16();
    void size32();
    void stringEquals();
    void stringToFloat();
    void stringToInt();
    void zerofill();
    void clearAll();
};

TestInternal::TestInternal()
{
}

void TestInternal::lerp()
{
    QCOMPARE(vpvl::internal::lerp(4, 2, 0), 4.0f);
    QCOMPARE(vpvl::internal::lerp(4, 2, 1), 2.0f);
    QCOMPARE(vpvl::internal::lerp(4, 2, 0.5), 3.0f);
}

void TestInternal::vector3()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << 1.0f;
    stream << 2.0f;
    stream << 3.0f;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    float actual[3];
    vpvl::internal::vector3(ptr, actual);
    QCOMPARE(actual[0], 1.0f);
    QCOMPARE(actual[1], 2.0f);
    QCOMPARE(actual[2], 3.0f);
}

void TestInternal::vector4()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << 4.0f;
    stream << 5.0f;
    stream << 6.0f;
    stream << 7.0f;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    float actual[3];
    vpvl::internal::vector4(ptr, actual);
    QCOMPARE(actual[0], 4.0f);
    QCOMPARE(actual[1], 5.0f);
    QCOMPARE(actual[2], 6.0f);
    QCOMPARE(actual[3], 7.0f);
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
    QVERIFY(!vpvl::internal::size8(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(0));
    rest = sizeof(quint8);
    // rest is now enough to read (1 = 1)
    QVERIFY(vpvl::internal::size8(ptr, rest, actual));
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
    QVERIFY(!vpvl::internal::size16(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(1));
    rest = sizeof(quint16);
    // rest is now enough to read (2 = 2)
    QVERIFY(vpvl::internal::size16(ptr, rest, actual));
    QCOMPARE(actual, size_t(expected));
    QCOMPARE(rest, size_t(0));
}

void TestInternal::size32()
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    QDataStream stream(&buffer);
    quint32 expected = 4294967295;
    buffer.open(QBuffer::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << expected;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(bytes.data());
    size_t rest = 2, actual = 0;
    // rest is not enough to read (2 < 4)
    QVERIFY(!vpvl::internal::size32(ptr, rest, actual));
    QCOMPARE(actual, size_t(0));
    QCOMPARE(rest, size_t(2));
    rest = sizeof(quint32);
    // rest is now enough to read (4 = 4)
    QVERIFY(vpvl::internal::size32(ptr, rest, actual));
    QCOMPARE(actual, size_t(expected));
    QCOMPARE(rest, size_t(0));
}

void TestInternal::stringEquals()
{
    const char *foo = "foo", *bar = "bar";
    const uint8_t *baz = reinterpret_cast<const uint8_t *>(foo),
            *qux = reinterpret_cast<const uint8_t *>(bar);
    QVERIFY(vpvl::internal::stringEquals(foo, foo, 3));
    QVERIFY(!vpvl::internal::stringEquals(foo, bar, 3));
    QVERIFY(vpvl::internal::stringEquals(baz, baz, 3));
    QVERIFY(!vpvl::internal::stringEquals(baz, qux, 3));
}

void TestInternal::stringToInt()
{
    QCOMPARE(vpvl::internal::stringToInt("42"), 42);
    QCOMPARE(vpvl::internal::stringToInt("test"), 0);
}

void TestInternal::stringToFloat()
{
    QCOMPARE(vpvl::internal::stringToFloat("4.2"), 4.2f);
    QCOMPARE(vpvl::internal::stringToFloat("4.2f"), 4.2f);
    QCOMPARE(vpvl::internal::stringToFloat("test"), 0.0f);
}

void TestInternal::zerofill()
{
    float src[] = { 1, 2, 3, 4, 5 };
    float dst[] = { 0, 0, 0, 0, 0 };
    vpvl::internal::zerofill(src, sizeof(float) * 5);
    for (int i = 0; i < 5; i++) {
        QCOMPARE(src[i], dst[i]);
    }
}

void TestInternal::clearAll()
{
    btAlignedObjectArray<int *> array;
    array.push_back(new int(1));
    array.push_back(new int(2));
    array.push_back(new int(3));
    vpvl::internal::clearAll(array);
    QCOMPARE(array.size(), 0);
    btHashMap<btHashString, int*> hash;
    hash.insert(btHashString("foo"), new int(1));
    hash.insert(btHashString("bar"), new int(2));
    hash.insert(btHashString("baz"), new int(3));
    vpvl::internal::clearAll(hash);
    QCOMPARE(hash.size(), 0);
}

QTEST_APPLESS_MAIN(TestInternal)

#include "TestInternal.moc"


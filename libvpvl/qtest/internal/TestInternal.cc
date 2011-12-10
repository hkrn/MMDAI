#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/Common.h>
#include <vpvl/internal/util.h>

using namespace vpvl;

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
    QVERIFY(isLibraryVersionCorrect(VPVL_VERSION));
    QVERIFY(!isLibraryVersionCorrect(
                VPVL_MAKE_VERSION(VPVL_VERSION_MAJOR - 1,
                                  VPVL_VERSION_COMPAT,
                                  VPVL_VERSION_MINOR)));
    QCOMPARE(libraryVersionString(), VPVL_VERSION_STRING);
}

void TestInternal::rad2deg()
{
    QCOMPARE(180.0f, degree(radian(180.0f)));
}

void TestInternal::deg2rad()
{
    QCOMPARE(kPI, radian(degree(kPI)));
}

QTEST_APPLESS_MAIN(TestInternal)

#include "TestInternal.moc"


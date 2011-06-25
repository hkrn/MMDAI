#include "gtest/gtest.h"
#include "vpvl/internal/util.h"

TEST(InternalTest, Lerp) {
    EXPECT_EQ(4, vpvl::internal::lerp(4, 2, 0));
    EXPECT_EQ(2, vpvl::internal::lerp(4, 2, 1));
    EXPECT_EQ(3, vpvl::internal::lerp(4, 2, 0.5));
}

TEST(InternalTest, VectorN) {
    float src[] = { 1, 2, 3, 4 };
    float dst1[] = { 0, 0, 0 };
    float dst2[] = { 0, 0, 0, 0 };
    uint8_t *ptr = reinterpret_cast<uint8_t *>(src), *start = ptr;
    vpvl::internal::vector3(ptr, dst1);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(src[i], dst1[i]);
    }
    EXPECT_EQ(12, ptr - start);
    ptr = start;
    vpvl::internal::vector4(ptr, dst2);
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(src[i], dst2[i]);
    }
    EXPECT_EQ(16, ptr - start);
}

TEST(InternalTest, StringEquals) {
    const char *foo = "foo", *bar = "bar";
    const uint8_t *baz = reinterpret_cast<const uint8_t *>(foo),
            *qux = reinterpret_cast<const uint8_t *>(bar);
    EXPECT_TRUE(vpvl::internal::stringEquals(foo, foo, 3));
    EXPECT_FALSE(vpvl::internal::stringEquals(foo, bar, 3));
    EXPECT_TRUE(vpvl::internal::stringEquals(baz, baz, 3));
    EXPECT_FALSE(vpvl::internal::stringEquals(baz, qux, 3));
}

TEST(InternalTest, StringToInt) {
    EXPECT_EQ(42, vpvl::internal::stringToInt("42"));
    EXPECT_EQ(0, vpvl::internal::stringToInt("test"));
}

TEST(InternalTest, StringToFloat) {
    EXPECT_EQ(4.2f, vpvl::internal::stringToFloat("4.2"));
    EXPECT_EQ(4.2f, vpvl::internal::stringToFloat("4.2f"));
    EXPECT_EQ(0, vpvl::internal::stringToFloat("test"));
}

TEST(InternalTest, ZeroFill) {
    float src[] = { 1, 2, 3, 4, 5 };
    float dst[] = { 0, 0, 0, 0, 0 };
    vpvl::internal::zerofill(src, sizeof(float) * 5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(dst[i], src[i]);
    }
}

TEST(InternalTest, ClearAll) {
    btAlignedObjectArray<int *> array;
    array.push_back(new int(1));
    array.push_back(new int(2));
    array.push_back(new int(3));
    vpvl::internal::clearAll(array);
    EXPECT_EQ(0, array.size());
    btHashMap<btHashString, int*> hash;
    hash.insert(btHashString("foo"), new int(1));
    hash.insert(btHashString("bar"), new int(2));
    hash.insert(btHashString("baz"), new int(3));
    vpvl::internal::clearAll(hash);
    EXPECT_EQ(0, hash.size());
}

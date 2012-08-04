#include "Common.h"

TEST(String, StartsWith)
{
    CString c("This is a test.");
    QScopedPointer<IString> s(new CString("This"));
    ASSERT_TRUE(c.startsWith(s.data()));
    s.reset(new CString(" This"));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new CString(" is "));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new CString("test."));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new CString("test. "));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new CString("foo"));
    ASSERT_FALSE(c.startsWith(s.data()));
}

TEST(String, Contains)
{
    CString c("This is a test.");
    QScopedPointer<IString> s(new CString("This"));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new CString(" This"));
    ASSERT_FALSE(c.contains(s.data()));
    s.reset(new CString(" is "));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new CString("test."));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new CString("test. "));
    ASSERT_FALSE(c.contains(s.data()));
    s.reset(new CString("foo"));
    ASSERT_FALSE(c.contains(s.data()));
}

TEST(String, EndsWith)
{
    CString c("This is a test.");
    QScopedPointer<IString> s(new CString("This"));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new CString(" This"));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new CString(" is "));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new CString("test."));
    ASSERT_TRUE(c.endsWith(s.data()));
    s.reset(new CString("test. "));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new CString("foo"));
    ASSERT_FALSE(c.endsWith(s.data()));
}

TEST(String, Clone)
{
    CString c("This is a test.");
    QScopedPointer<IString> s(new CString("This is a test.")), c2(c.clone());
    ASSERT_TRUE(c2->equals(s.data()));
    ASSERT_TRUE(c2.data() != s.data());
}

TEST(String, ToHashString)
{
    Hash<HashString, int> hash;
    const char key[] = "key";
    int value = 42;
    hash.insert(key, value);
    CString c("key");
    const int *ptr = hash.find(c.toHashString());
    ASSERT_TRUE(ptr);
    ASSERT_EQ(42, *ptr);
}

TEST(String, ToByteArray)
{
    const char str[] = "This is a test.";
    CString c(str);
    ASSERT_STREQ(str, reinterpret_cast<const char *>(c.toByteArray()));
}

TEST(String, Length)
{
    const char str[] = "This is a test.";
    CString c(str);
    ASSERT_EQ(sizeof(str) - 1, c.length());
}

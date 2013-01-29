#include "Common.h"
#include "vpvl2/extensions/icu4c/String.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

TEST(String, StartsWith)
{
    String c("This is a test.");
    QScopedPointer<IString> s(new String("This"));
    ASSERT_TRUE(c.startsWith(s.data()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new String(" is "));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new String("test."));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.startsWith(s.data()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.startsWith(s.data()));
}

TEST(String, Contains)
{
    String c("This is a test.");
    QScopedPointer<IString> s(new String("This"));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.contains(s.data()));
    s.reset(new String(" is "));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new String("test."));
    ASSERT_TRUE(c.contains(s.data()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.contains(s.data()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.contains(s.data()));
}

TEST(String, EndsWith)
{
    String c("This is a test.");
    QScopedPointer<IString> s(new String("This"));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new String(" is "));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new String("test."));
    ASSERT_TRUE(c.endsWith(s.data()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.endsWith(s.data()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.endsWith(s.data()));
}

TEST(String, Clone)
{
    String c("This is a test.");
    QScopedPointer<IString> s(new String("This is a test.")), c2(c.clone());
    ASSERT_TRUE(c2->equals(s.data()));
    ASSERT_TRUE(c2.data() != s.data());
}

TEST(String, ToHashString)
{
    Hash<HashString, int> hash;
    const char key[] = "key";
    int value = 42;
    hash.insert(key, value);
    String c("key");
    const int *ptr = hash.find(c.toHashString());
    ASSERT_TRUE(ptr);
    ASSERT_EQ(42, *ptr);
}

TEST(String, ToByteArray)
{
    const char str[] = "This is a test.";
    String c(str);
    ASSERT_STREQ(str, reinterpret_cast<const char *>(c.toByteArray()));
}

TEST(String, Length)
{
    const char str[] = "This is a test.";
    String c(str);
    ASSERT_EQ(sizeof(str) - 1, c.size());
}

TEST(String, Split)
{
    String s("This*Is*A*Test"), sep("*");
    Array<IString *> tokens;
    s.split(&sep, -1, tokens);
    ASSERT_EQ(4, tokens.count());
    ASSERT_STREQ(reinterpret_cast<const char *>(String("This").toByteArray()),
                 reinterpret_cast<const char *>(tokens[0]->toByteArray()));
    ASSERT_STREQ(reinterpret_cast<const char *>(String("Is").toByteArray()),
                 reinterpret_cast<const char *>(tokens[1]->toByteArray()));
    ASSERT_STREQ(reinterpret_cast<const char *>(String("A").toByteArray()),
                 reinterpret_cast<const char *>(tokens[2]->toByteArray()));
    ASSERT_STREQ(reinterpret_cast<const char *>(String("Test").toByteArray()),
                 reinterpret_cast<const char *>(tokens[3]->toByteArray()));
    s.split(&sep, 0, tokens);
    ASSERT_EQ(1, tokens.count());
    ASSERT_STREQ(reinterpret_cast<const char *>(s.toByteArray()),
                 reinterpret_cast<const char *>(tokens[0]->toByteArray()));
    tokens.releaseAll();
    s.split(&sep, 3, tokens);
    ASSERT_EQ(3, tokens.count());
    ASSERT_STREQ(reinterpret_cast<const char *>(String("This").toByteArray()),
                 reinterpret_cast<const char *>(tokens[0]->toByteArray()));
    ASSERT_STREQ(reinterpret_cast<const char *>(String("Is").toByteArray()),
                 reinterpret_cast<const char *>(tokens[1]->toByteArray()));
    ASSERT_STREQ(reinterpret_cast<const char *>(String("A*Test").toByteArray()),
                 reinterpret_cast<const char *>(tokens[2]->toByteArray()));
    tokens.releaseAll();
}

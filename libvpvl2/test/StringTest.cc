#include "Common.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/extensions/icu4c/String.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;

TEST(String, StartsWith)
{
    String c("This is a test.");
    std::unique_ptr<IString> s(new String("This"));
    ASSERT_TRUE(c.startsWith(s.get()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.startsWith(s.get()));
    s.reset(new String(" is "));
    ASSERT_FALSE(c.startsWith(s.get()));
    s.reset(new String("test."));
    ASSERT_FALSE(c.startsWith(s.get()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.startsWith(s.get()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.startsWith(s.get()));
}

TEST(String, Contains)
{
    String c("This is a test.");
    std::unique_ptr<IString> s(new String("This"));
    ASSERT_TRUE(c.contains(s.get()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.contains(s.get()));
    s.reset(new String(" is "));
    ASSERT_TRUE(c.contains(s.get()));
    s.reset(new String("test."));
    ASSERT_TRUE(c.contains(s.get()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.contains(s.get()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.contains(s.get()));
}

TEST(String, EndsWith)
{
    String c("This is a test.");
    std::unique_ptr<IString> s(new String("This"));
    ASSERT_FALSE(c.endsWith(s.get()));
    s.reset(new String(" This"));
    ASSERT_FALSE(c.endsWith(s.get()));
    s.reset(new String(" is "));
    ASSERT_FALSE(c.endsWith(s.get()));
    s.reset(new String("test."));
    ASSERT_TRUE(c.endsWith(s.get()));
    s.reset(new String("test. "));
    ASSERT_FALSE(c.endsWith(s.get()));
    s.reset(new String("foo"));
    ASSERT_FALSE(c.endsWith(s.get()));
}

TEST(String, Clone)
{
    String c("This is a test.");
    std::unique_ptr<IString> s(new String("This is a test.")), c2(c.clone());
    ASSERT_TRUE(c2->equals(s.get()));
    ASSERT_TRUE(c2.get() != s.get());
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

TEST(String, Join)
{
    Array<IString *> strings;
    String sep("*"), foo("foo"), bar("bar"), baz("baz");
    std::unique_ptr<IString> empty(sep.join(strings));
    ASSERT_STREQ(reinterpret_cast<const char *>(""), reinterpret_cast<const char *>(empty->toByteArray()));
    strings.append(&foo);
    std::unique_ptr<IString> s1(sep.join(strings));
    ASSERT_STREQ(reinterpret_cast<const char *>("foo"), reinterpret_cast<const char *>(s1->toByteArray()));
    strings.append(&bar);
    std::unique_ptr<IString> s2(sep.join(strings));
    ASSERT_STREQ(reinterpret_cast<const char *>("foo*bar"), reinterpret_cast<const char *>(s2->toByteArray()));
    strings.append(&baz);
    std::unique_ptr<IString> s3(sep.join(strings));
    ASSERT_STREQ(reinterpret_cast<const char *>("foo*bar*baz"), reinterpret_cast<const char *>(s3->toByteArray()));
}

TEST(String, ToStdString)
{
    Encoding e(0);
    std::string t("This is a test."), t2;
    IString *name = e.toString(reinterpret_cast<const uint8 *>(t.data()), t.size(), IString::kShiftJIS);
    ASSERT_TRUE(name);
    t2.assign(String::toStdString(static_cast<const String *>(name)->value()));
    ASSERT_EQ(0, t.compare(t2));
}

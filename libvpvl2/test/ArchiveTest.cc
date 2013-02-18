#include "Common.h"
#include "vpvl2/extensions/Archive.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/qt/Util.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::qt;

namespace {

QStringList UIToStringList(const Archive::EntryNames &value)
{
    QStringList ret;
    Archive::EntryNames::const_iterator it = value.begin();
    while (it != value.end()) {
        ret << Util::toQString(*it);
        ++it;
    }
    return ret;
}

Archive::EntrySet UIToSet(const QStringList &value)
{
    Archive::EntrySet ret;
    foreach (const QString &s, value) {
        ret.insert(s.toStdString());
    }
    return ret;
}

Archive::EntrySet UIToSet(const Archive::EntryNames &value)
{
    Archive::EntrySet ret;
    Archive::EntryNames::const_iterator it = value.begin();
    while (it != value.end()) {
        ret.insert(String::toStdString(*it));
        ++it;
    }
    return ret;
}

bool UICompareEntries(const QStringList &entries, const Archive &archive)
{
    const Archive::EntryNames &e = archive.entryNames();
    const QSet<QString> &set = entries.toSet();
    Archive::EntryNames::const_iterator it = e.begin();
    while (it != e.end()) {
        if (!set.contains(Util::toQString(*it))) {
            return false;
        }
        ++it;
    }
    return true;
}

static void UncompressArchive(Archive &archive, Archive::EntryNames &entries)
{
    QFile file(":misc/test.zip");
    QScopedPointer<QTemporaryFile> temp(QTemporaryFile::createLocalFile(file));
    ASSERT_TRUE(temp);
    temp->setAutoRemove(true);
    String path(Util::fromQString(temp->fileName()));
    ASSERT_TRUE(archive.open(&path, entries));
}

static const QStringList AllEntries()
{
    QStringList entries;
    entries << QString("bar.txt");
    entries << QString("baz.txt");
    entries << QString("foo.txt");
    entries << QString("path/");
    entries << QString("path/to/");
    entries << QString("path/to/entry.txt");
    entries.sort();
    return entries;
}

}

TEST(ArchiveTest, GetEntries)
{
    Encoding encoding(0);
    Archive archive(&encoding);
    std::vector<UnicodeString> entries;
    UncompressArchive(archive, entries);
    QStringList actual = UIToStringList(entries);
    actual.sort();
    ASSERT_TRUE(actual == AllEntries());
}

TEST(ArchiveTest, UncompressAllEntries)
{
    Encoding encoding(0);
    Archive archive(&encoding);
    Archive::EntryNames entries;
    UncompressArchive(archive, entries);
    ASSERT_TRUE(archive.uncompress(UIToSet(entries)));
    //entries.sort();
    QStringList actualEntries = UIToStringList(archive.entryNames());
    actualEntries.sort();
    ASSERT_TRUE(actualEntries == AllEntries());
    ASSERT_STREQ("foo\n", archive.data("foo.txt")->c_str());
    ASSERT_STREQ("bar\n", archive.data("bar.txt")->c_str());
    ASSERT_STREQ("baz\n", archive.data("baz.txt")->c_str());
    ASSERT_STREQ("entry.txt\n", archive.data("path/to/entry.txt")->c_str());
}

TEST(ArchiveTest, UncompressEntriesPartially)
{
    Encoding encoding(0);
    Archive archive(&encoding);
    Archive::EntryNames entries;
    UncompressArchive(archive, entries);
    QStringList extractEntries; extractEntries << "foo.txt";
    ASSERT_TRUE(archive.uncompress(UIToSet(extractEntries)));
    ASSERT_TRUE(UICompareEntries(extractEntries, archive));
    const std::string *dataRef = archive.data("foo.txt");
    ASSERT_TRUE(dataRef);
    ASSERT_STREQ("foo\n", dataRef->c_str());
    ASSERT_FALSE(archive.data("bar.txt"));
    ASSERT_FALSE(archive.data("baz.txt"));
    ASSERT_FALSE(archive.data("path/to/entry.txt"));
}

TEST(ArchiveTest, UncompressWithReplaceIfMatch)
{
    Encoding encoding(0);
    Archive archive(&encoding);
    Archive::EntryNames entries;
    QStringList extractEntries;
    UncompressArchive(archive, entries);
    extractEntries << "path/to/entry.txt";
    ASSERT_TRUE(archive.uncompress(UIToSet(extractEntries)));
    archive.replaceFilePath("path/to", "/PATH/TO/");
    extractEntries.clear(); extractEntries << "/PATH/TO/entry.txt";
    ASSERT_TRUE(UICompareEntries(extractEntries, archive));
    const std::string *dataRef = archive.data("/PATH/TO/entry.txt");
    ASSERT_TRUE(dataRef);
    ASSERT_STREQ("entry.txt\n", dataRef->c_str());
}

TEST(ArchiveTest, UncompressWithReplaceIfNotMatch)
{
    Encoding encoding(0);
    Archive archive(&encoding);
    Archive::EntryNames entries;
    QStringList extractEntries;
    UncompressArchive(archive, entries);
    extractEntries << "foo.txt" << "bar.txt";
    ASSERT_TRUE(archive.uncompress(UIToSet(extractEntries)));
    archive.replaceFilePath("test", "/path/to/");
    extractEntries.clear();
    extractEntries << "/path/to/foo.txt" << "/path/to/bar.txt";
    extractEntries.sort();
    ASSERT_TRUE(UICompareEntries(extractEntries, archive));
    const std::string *dataRef1 = archive.data("/path/to/foo.txt");
    ASSERT_TRUE(dataRef1);
    ASSERT_STREQ("foo\n", dataRef1->c_str());
    const std::string *dataRef2 = archive.data("/path/to/bar.txt");
    ASSERT_TRUE(dataRef2);
    ASSERT_STREQ("bar\n", dataRef2->c_str());
}

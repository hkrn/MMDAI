#include "Common.h"
#include "vpvl2/qt/Archive.h"

namespace {

static void UncompressArchive(Archive &archive, QStringList &entries)
{
    QFile file(":misc/test.zip");
    QTemporaryFile *temp = QTemporaryFile::createLocalFile(file);
    ASSERT_TRUE(temp);
    temp->setAutoRemove(true);
    ASSERT_TRUE(archive.open(temp->fileName(), entries));
    delete temp;
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
    Archive archive;
    QStringList entries;
    UncompressArchive(archive, entries);
    entries.sort();
    ASSERT_TRUE(entries == AllEntries());
}

TEST(ArchiveTest, UncompressAllEntries)
{
    Archive archive;
    QStringList entries;
    UncompressArchive(archive, entries);
    ASSERT_TRUE(archive.uncompress(entries));
    entries.sort();
    QStringList actualEntries = archive.entryNames();
    actualEntries.sort();
    ASSERT_TRUE(actualEntries == AllEntries());
    ASSERT_STREQ("foo\n", archive.data("foo.txt").constData());
    ASSERT_STREQ("bar\n", archive.data("bar.txt").constData());
    ASSERT_STREQ("baz\n", archive.data("baz.txt").constData());
    ASSERT_STREQ("entry.txt\n", archive.data("path/to/entry.txt").constData());
}

TEST(ArchiveTest, UncompressEntriesPartially)
{
    Archive archive;
    QStringList entries, extractEntries;
    UncompressArchive(archive, entries);
    extractEntries << "foo.txt";
    ASSERT_TRUE(archive.uncompress(extractEntries));
    ASSERT_TRUE(archive.entryNames() == extractEntries);
    ASSERT_STREQ("foo\n", archive.data("foo.txt").constData());
    ASSERT_TRUE(archive.data("bar.txt").isEmpty());
    ASSERT_TRUE(archive.data("baz.txt").isEmpty());
    ASSERT_TRUE(archive.data("path/to/entry.txt").isEmpty());
}

TEST(ArchiveTest, UncompressWithReplaceIfMatch)
{
    Archive archive;
    QStringList entries, extractEntries;
    UncompressArchive(archive, entries);
    extractEntries << "path/to/entry.txt";
    ASSERT_TRUE(archive.uncompress(extractEntries));
    archive.replaceFilePath("path/to", "/PATH/TO/");
    extractEntries.clear(); extractEntries << "/PATH/TO/entry.txt";
    ASSERT_TRUE(archive.entryNames() == extractEntries);
    ASSERT_STREQ("entry.txt\n", archive.data("/PATH/TO/entry.txt").constData());
}

TEST(ArchiveTest, UncompressWithReplaceIfNotMatch)
{
    Archive archive;
    QStringList entries, extractEntries;
    UncompressArchive(archive, entries);
    extractEntries << "foo.txt" << "bar.txt";
    ASSERT_TRUE(archive.uncompress(extractEntries));
    archive.replaceFilePath("test", "/path/to/");
    extractEntries.clear();
    extractEntries << "/path/to/foo.txt" << "/path/to/bar.txt";
    extractEntries.sort();
    ASSERT_TRUE(archive.entryNames() == extractEntries);
    ASSERT_STREQ("foo\n", archive.data("/path/to/foo.txt").constData());
    ASSERT_STREQ("bar\n", archive.data("/path/to/bar.txt").constData());
}

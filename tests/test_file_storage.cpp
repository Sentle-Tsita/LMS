#include <gtest/gtest.h>

#include "TestWorld.hpp"
#include "services/Errors.hpp"
#include "storage/FileStorage.hpp"

namespace {

class FileStorageTest : public ::testing::Test {
protected:
    TestWorld world;
    std::string file(const std::string& name, std::size_t bytes = 100) { return world.makeFile(name, bytes); }
};

TEST_F(FileStorageTest, AcceptsAllowedTypes) {
    for (const char* name : {"a.docx", "a.pdf", "a.cpp", "a.py", "a.java"}) {
        EXPECT_NO_THROW(FileStorage::validate(file(name))) << name;
    }
}

TEST_F(FileStorageTest, ExtensionCheckIgnoresCase) {
    EXPECT_NO_THROW(FileStorage::validate(file("REPORT.PDF")));
}

TEST_F(FileStorageTest, RejectsOtherTypes) {
    for (const char* name : {"virus.exe", "photo.png", "archive.zip", "noextension"}) {
        EXPECT_THROW(FileStorage::validate(file(name)), ValidationError) << name;
    }
}

TEST_F(FileStorageTest, RejectsAMissingFile) {
    EXPECT_THROW(FileStorage::validate(world.dir / "nothing.pdf"), ValidationError);
}

TEST_F(FileStorageTest, SizeLimitIsExactlyFiveMegabytes) {
    EXPECT_NO_THROW(FileStorage::validate(file("exact.pdf", 5 * 1024 * 1024)));                    // 5 MB: accepted
    EXPECT_THROW(FileStorage::validate(file("over.pdf", 5 * 1024 * 1024 + 1)), ValidationError);   // 5 MB + 1 byte
    EXPECT_THROW(FileStorage::validate(file("six.pdf", 6 * 1024 * 1024)), ValidationError);
}

TEST_F(FileStorageTest, StoreCopiesTheFileUnderASafeName) {
    const std::string original = file("my essay (final).pdf", 50);
    const std::string stored = world.storage.store(original, 7, 3, DateTime(std::chrono::seconds(1234)));
    EXPECT_TRUE(std::filesystem::exists(stored));
    EXPECT_EQ(std::filesystem::file_size(stored), 50u);
    EXPECT_NE(stored.find("a7_s3_1234.pdf"), std::string::npos);
    EXPECT_EQ(stored.find("essay"), std::string::npos);  // the user's file name is not used
}

TEST_F(FileStorageTest, StoreRefusesABadFile) {
    EXPECT_THROW(world.storage.store(file("bad.exe"), 1, 1, DateTime(std::chrono::seconds(1))), ValidationError);
}

}  // namespace

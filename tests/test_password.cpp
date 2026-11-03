#include <gtest/gtest.h>

#include "security/PasswordHasher.hpp"

TEST(PasswordHasher, SaltIsRandomAndTheRightLength) {
    PasswordHasher hasher(1000);
    const std::string a = hasher.newSalt();
    const std::string b = hasher.newSalt();
    EXPECT_EQ(a.size(), 32u);
    EXPECT_NE(a, b);
}

TEST(PasswordHasher, HashIsDeterministicForTheSameSalt) {
    PasswordHasher hasher(1000);
    const std::string salt = hasher.newSalt();
    EXPECT_EQ(hasher.hash("Password123", salt), hasher.hash("Password123", salt));
    EXPECT_EQ(hasher.hash("Password123", salt).size(), 64u);
}

TEST(PasswordHasher, DifferentSaltGivesDifferentHash) {
    PasswordHasher hasher(1000);
    EXPECT_NE(hasher.hash("Password123", hasher.newSalt()), hasher.hash("Password123", hasher.newSalt()));
}

TEST(PasswordHasher, HashNeverContainsThePassword) {
    PasswordHasher hasher(1000);
    EXPECT_EQ(hasher.hash("Password123", hasher.newSalt()).find("Password123"), std::string::npos);
}

TEST(PasswordHasher, VerifyAcceptsOnlyTheRightPassword) {
    PasswordHasher hasher(1000);
    const std::string salt = hasher.newSalt();
    const std::string stored = hasher.hash("Password123", salt);
    EXPECT_TRUE(hasher.verify("Password123", salt, stored));
    EXPECT_FALSE(hasher.verify("password123", salt, stored));
    EXPECT_FALSE(hasher.verify("", salt, stored));
    EXPECT_FALSE(hasher.verify("Password123", salt, "short"));
}

TEST(PasswordHasher, IterationCountChangesTheHash) {
    const std::string salt = "0123456789abcdef0123456789abcdef";
    EXPECT_NE(PasswordHasher(1000).hash("Password123", salt), PasswordHasher(2000).hash("Password123", salt));
}

TEST(PasswordHasher, MatchesThePublishedPbkdf2Sha256Vector) {
    // Published test vector (RFC 7914): PBKDF2-HMAC-SHA-256, P="passwd", S="salt", c=1, first 32 bytes.
    EXPECT_EQ(PasswordHasher(1).hash("passwd", "salt"), "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc");
}

// Evidence for the measurable non-functional requirements of the SRS.
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>

#include "TestWorld.hpp"

namespace {

using Clock = std::chrono::steady_clock;

double secondsSince(Clock::time_point start) { return std::chrono::duration<double>(Clock::now() - start).count(); }

// NFR-01: login completes within 2 seconds with 800 students stored (real password-hashing settings).
TEST(NonFunctional, LoginIsFastWith800StudentsStored) {
    Database db(":memory:");
    db.initialiseSchema(LMS_SCHEMA_PATH);
    UserMapper mapper;
    SqliteRepository<std::unique_ptr<User>, int> users(db, mapper);
    PasswordHasher realHasher;  // default: 600,000 PBKDF2 iterations
    const std::string salt = realHasher.newSalt();
    const std::string hash = realHasher.hash("Password123", salt);
    for (int i = 0; i < 800; ++i) {
        users.save(makeUser("STUDENT", 0, "student" + std::to_string(i), hash, salt, true));
    }
    ASSERT_EQ(users.findAll().size(), 800u);

    DepartmentMapper departmentMapper;
    CourseMapper courseMapper;
    SqliteRepository<Department, int> departments(db, departmentMapper);
    CourseRepository courses(db, courseMapper);
    AccountService accounts(users, departments, courses, realHasher);

    double slowest = 0.0;
    for (int i = 0; i < 10; ++i) {  // 10 logins, as in the SRS verification step
        const auto start = Clock::now();
        const auto user = accounts.login("student" + std::to_string(i * 70), "Password123");
        slowest = std::max(slowest, secondsSince(start));
        ASSERT_NE(user, nullptr);
    }
    std::cout << "[ NFR-01 ] slowest of 10 logins with 800 students stored: " << slowest << " s (limit 2 s)\n";
    EXPECT_LT(slowest, 2.0);
}

// NFR-02: submitting a file of up to 5 MB completes within 10 seconds.
TEST(NonFunctional, SubmittingA5MegabyteFileIsFast) {
    TestWorld world;
    const Seed seed = world.seedStandard();
    const std::string big = world.makeFile("big.pdf", 5 * 1024 * 1024);
    double slowest = 0.0;
    for (int i = 0; i < 5; ++i) {  // 5 submissions, as in the SRS verification step
        const int student = i == 0 ? seed.student1 : seed.student2;
        const auto start = Clock::now();
        const DateTime when = seed.deadline - std::chrono::hours(10 - i);
        if (world.submissionService.findFor(student, seed.assignment)) {
            world.submissionService.resubmit(student, seed.assignment, big, when);
        } else {
            world.submissionService.submit(student, seed.assignment, big, when);
        }
        slowest = std::max(slowest, secondsSince(start));
    }
    std::cout << "[ NFR-02 ] slowest of 5 submissions of a 5 MB file: " << slowest << " s (limit 10 s)\n";
    EXPECT_LT(slowest, 10.0);
}

// NFR-08: saved data remains after the system is closed and reopened.
TEST(NonFunctional, DataSurvivesClosingAndReopeningTheDatabase) {
    const std::filesystem::path dir = makeTempDir();
    const std::string file = (dir / "lms.db").string();
    {
        Database db(file);
        db.initialiseSchema(LMS_SCHEMA_PATH);
        UserMapper mapper;
        SqliteRepository<std::unique_ptr<User>, int> users(db, mapper);
        users.save(makeUser("STUDENT", 0, "persistent", "hash", "salt", true));
    }  // database closed here
    {
        Database db(file);
        EXPECT_FALSE(db.initialiseSchema(LMS_SCHEMA_PATH));  // already set up: nothing is wiped
        UserMapper mapper;
        SqliteRepository<std::unique_ptr<User>, int> users(db, mapper);
        const auto found = users.findWhere({{"username", "persistent"}});
        ASSERT_EQ(found.size(), 1u);
        EXPECT_EQ(found[0]->roleName(), "STUDENT");
    }
    std::error_code ignored;
    std::filesystem::remove_all(dir, ignored);
}

}  // namespace

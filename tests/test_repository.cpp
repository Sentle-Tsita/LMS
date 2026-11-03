#include <gtest/gtest.h>

#include "TestWorld.hpp"

namespace {

using std::chrono::hours;

class RepositoryTest : public ::testing::Test {
protected:
    TestWorld world;
};

TEST_F(RepositoryTest, DepartmentWithoutAHeadReadsBackAsEmpty) {
    const int id = world.departments.save(Department(0, "Science", std::nullopt));
    EXPECT_GT(id, 0);
    const auto loaded = world.departments.findById(id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getName(), "Science");
    EXPECT_FALSE(loaded->getHodUserId().has_value());
}

TEST_F(RepositoryTest, DepartmentWithAHeadReadsBackWithTheHead) {
    const int hod = world.accounts.createUser("HOD", "hod1", "Password123");
    const int id = world.departments.save(Department(0, "Science", hod));
    EXPECT_EQ(world.departments.findById(id)->getHodUserId(), std::optional<int>(hod));
}

TEST_F(RepositoryTest, SavingAnExistingDepartmentUpdatesIt) {
    const int id = world.departments.save(Department(0, "Science", std::nullopt));
    EXPECT_EQ(world.departments.save(Department(id, "Natural Science", std::nullopt)), id);
    EXPECT_EQ(world.departments.findById(id)->getName(), "Natural Science");
    EXPECT_EQ(world.departments.findAll().size(), 1u);
}

TEST_F(RepositoryTest, MissingIdGivesAnEmptyResult) {
    EXPECT_FALSE(world.departments.findById(999).has_value());
}

TEST_F(RepositoryTest, UpdatingARowThatDoesNotExistThrows) {
    EXPECT_THROW(world.departments.save(Department(999, "Ghost", std::nullopt)), DatabaseError);
}

TEST_F(RepositoryTest, ForeignKeyViolationThrows) {
    EXPECT_THROW(world.departments.save(Department(0, "Science", 12345)), DatabaseError);
}

TEST_F(RepositoryTest, UsersComeBackAsTheirRoleSubclass) {
    for (const char* role : {"STUDENT", "LECTURER", "HOD", "ADMINISTRATOR"}) {
        const int id = world.accounts.createUser(role, std::string("u_") + role, "Password123");
        const auto loaded = world.users.findById(id);
        ASSERT_TRUE(loaded.has_value()) << role;
        EXPECT_EQ((*loaded)->roleName(), role);
        EXPECT_TRUE((*loaded)->isActive());
    }
}

TEST_F(RepositoryTest, FindWhereMatchesOnAColumn) {
    world.accounts.createUser("STUDENT", "alice", "Password123");
    EXPECT_EQ(world.users.findWhere({{"username", "alice"}}).size(), 1u);
    EXPECT_TRUE(world.users.findWhere({{"username", "bob"}}).empty());
}

TEST_F(RepositoryTest, ValuesAreBoundSoInjectionTextIsJustText) {
    world.accounts.createUser("STUDENT", "alice", "Password123");
    EXPECT_TRUE(world.users.findWhere({{"username", "x' OR '1'='1"}}).empty());
    EXPECT_TRUE(world.users.findWhere({{"username", "x'; DROP TABLE user;--"}}).empty());
    EXPECT_EQ(world.users.findAll().size(), 1u);  // the table is still there
}

TEST_F(RepositoryTest, UnknownColumnNamesAreRejected) {
    EXPECT_THROW(world.users.findWhere({{"1=1; DROP TABLE user;--", 1}}), DatabaseError);
}

TEST_F(RepositoryTest, CourseSaveInsertsThenUpdates) {
    const int dept = world.departments.save(Department(0, "Science", std::nullopt));
    world.courses.save(Course("SCI1", "Physics", dept, std::nullopt));
    EXPECT_FALSE(world.courses.findById("SCI1")->getLecturerId().has_value());
    world.courses.save(Course("SCI1", "Physics I", dept, std::nullopt));  // same key: update
    EXPECT_EQ(world.courses.findAll().size(), 1u);
    EXPECT_EQ(world.courses.findById("SCI1")->getCourseName(), "Physics I");
}

TEST_F(RepositoryTest, EnrolmentQueries) {
    const Seed seed = world.seedStandard();
    EXPECT_TRUE(world.courses.isEnrolled(seed.student1, "CS101"));
    EXPECT_FALSE(world.courses.isEnrolled(seed.student3, "CS101"));
    ASSERT_EQ(world.courses.findByStudent(seed.student1).size(), 1u);
    EXPECT_EQ(world.courses.findByStudent(seed.student1)[0].getCourseCode(), "CS101");
    EXPECT_TRUE(world.courses.findByStudent(seed.student3).empty());
    EXPECT_THROW(world.courses.enrol(seed.student1, "CS101"), DatabaseError);  // already enrolled
}

TEST_F(RepositoryTest, AssignmentAndExtensionKeepTheirDates) {
    const Seed seed = world.seedStandard();
    const auto assignment = world.assignmentRepo.findById(seed.assignment);
    ASSERT_TRUE(assignment.has_value());
    EXPECT_EQ(assignment->getDeadline(), seed.deadline);
    EXPECT_DOUBLE_EQ(assignment->getMaxMark(), 20.0);

    const int id = world.extensions.save(Extension(0, seed.assignment, seed.student1, seed.deadline + hours(48)));
    EXPECT_EQ(world.extensions.findById(id)->getNewDeadline(), seed.deadline + hours(48));
}

TEST_F(RepositoryTest, SubmissionKeepsStatusMarkAndFeedback) {
    const Seed seed = world.seedStandard();
    Submission submission(seed.student1, seed.assignment, "f.pdf", seed.deadline);
    const int id = world.submissions.save(submission);

    auto loaded = world.submissions.findById(id);
    EXPECT_EQ(loaded->getStatus(), SubmissionStatus::Submitted);
    EXPECT_FALSE(loaded->getMark().has_value());

    loaded->setMark(12.5);
    loaded->setFeedback("Well done");
    loaded->setStatus(SubmissionStatus::Moderated);
    world.submissions.save(*loaded);
    const auto again = world.submissions.findById(id);
    EXPECT_EQ(again->getStatus(), SubmissionStatus::Moderated);
    EXPECT_EQ(again->getMark(), std::optional<double>(12.5));
    EXPECT_EQ(again->getFeedback(), "Well done");
}

TEST_F(RepositoryTest, JoinQueriesFindTheRightPeople) {
    const Seed seed = world.seedStandard();
    const int id = world.submissions.save(Submission(seed.student1, seed.assignment, "f.pdf", seed.deadline));
    EXPECT_EQ(world.submissions.lecturerOf(id), std::optional<int>(seed.lectCs));
    EXPECT_EQ(world.submissions.headOf(id), std::optional<int>(seed.hodCs));
    EXPECT_FALSE(world.submissions.headOf(9999).has_value());
    EXPECT_EQ(world.submissions.findForLecturer(seed.lectCs).size(), 1u);
    EXPECT_TRUE(world.submissions.findForLecturer(seed.lectMath).empty());
    EXPECT_EQ(world.submissions.findForHod(seed.hodCs).size(), 1u);
    EXPECT_TRUE(world.submissions.findForHod(seed.hodMath).empty());
}

TEST_F(RepositoryTest, AuditEntryWithoutAnOldMarkReadsBackAsEmpty) {
    const Seed seed = world.seedStandard();
    const int sub = world.submissions.save(Submission(seed.student1, seed.assignment, "f.pdf", seed.deadline));
    const int id = world.auditLog.save(AuditLogEntry(0, sub, seed.lectCs, std::nullopt, 15.0, seed.deadline, "Initial marking"));
    const auto entry = world.auditLog.findById(id);
    EXPECT_FALSE(entry->getOldMark().has_value());
    EXPECT_DOUBLE_EQ(entry->getNewMark(), 15.0);
    EXPECT_EQ(entry->getReason(), "Initial marking");
}

}  // namespace

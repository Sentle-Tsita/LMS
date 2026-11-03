// Unit and integration tests of the four services, with the real repositories on an in-memory database.
#include <gtest/gtest.h>

#include "TestWorld.hpp"

namespace {

using std::chrono::hours;
using std::chrono::seconds;

class ServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        seed = world.seedStandard();
        pdf = world.makeFile("work.pdf");
    }

    int submissionId(int student) { return world.submissionService.findFor(student, seed.assignment)->getSubmissionId(); }

    // Submits student1's work (or the given student's) at deadline + offset and returns the status.
    SubmissionStatus submitAt(seconds offset, int student = 0) {
        return world.submissionService.submit(student == 0 ? seed.student1 : student, seed.assignment, pdf, seed.deadline + offset);
    }

    TestWorld world;
    Seed seed;
    std::string pdf;
};

// ---------------------------------------------------------------- accounts (FR-01 to FR-06)

TEST_F(ServiceTest, PasswordsAreStoredOnlyAsSaltedHashes) {
    const auto stored = world.users.findById(seed.student1);
    EXPECT_NE((*stored)->getPasswordHash(), "Password123");
    EXPECT_EQ((*stored)->getPasswordHash().size(), 64u);
    EXPECT_EQ((*stored)->getSalt().size(), 32u);
    const auto other = world.users.findById(seed.student2);
    EXPECT_NE((*stored)->getSalt(), (*other)->getSalt());
    EXPECT_NE((*stored)->getPasswordHash(), (*other)->getPasswordHash());  // same password, different salt
}

TEST_F(ServiceTest, LoginSucceedsWithTheRightPassword) {
    const auto user = world.accounts.login("lect_cs", "Password123");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->roleName(), "LECTURER");
}

TEST_F(ServiceTest, LoginFailsWithAWrongPasswordOrUnknownUser) {
    EXPECT_EQ(world.accounts.login("lect_cs", "wrong-password"), nullptr);
    EXPECT_EQ(world.accounts.login("nobody", "Password123"), nullptr);
    EXPECT_EQ(world.accounts.login("lect_cs", ""), nullptr);
}

TEST_F(ServiceTest, DeactivatedUserCannotLogIn) {
    world.accounts.deactivateUser(seed.student1);
    EXPECT_EQ(world.accounts.login("student1", "Password123"), nullptr);
    EXPECT_TRUE(world.users.findById(seed.student1).has_value());  // deactivated, not deleted
}

TEST_F(ServiceTest, InvalidAccountDetailsAreRejected) {
    EXPECT_THROW(world.accounts.createUser("STUDENT", "student1", "Password123"), ValidationError);  // duplicate
    EXPECT_THROW(world.accounts.createUser("STUDENT", "ab", "Password123"), ValidationError);        // too short
    EXPECT_THROW(world.accounts.createUser("STUDENT", "has space", "Password123"), ValidationError);
    EXPECT_THROW(world.accounts.createUser("STUDENT", "newuser", "short"), ValidationError);
    EXPECT_THROW(world.accounts.createUser("JANITOR", "newuser", "Password123"), ValidationError);
    EXPECT_THROW(world.accounts.deactivateUser(99999), ValidationError);
}

TEST_F(ServiceTest, SetupRulesForDepartmentsCoursesAndEnrolment) {
    EXPECT_THROW(world.accounts.createDepartment("  ", std::nullopt), ValidationError);
    EXPECT_THROW(world.accounts.createDepartment("Art", seed.student1), ValidationError);        // not a head of department
    EXPECT_THROW(world.accounts.assignLecturer("CS101", seed.student1), ValidationError);        // not a lecturer
    EXPECT_THROW(world.accounts.assignLecturer("NOPE", seed.lectCs), ValidationError);
    EXPECT_THROW(world.accounts.enrolStudent(seed.lectCs, "CS101"), ValidationError);            // not a student
    EXPECT_THROW(world.accounts.enrolStudent(seed.student1, "CS101"), ValidationError);          // already enrolled
    EXPECT_THROW(world.accounts.enrolStudent(seed.student3, "NOPE"), ValidationError);
    EXPECT_THROW(world.accounts.saveCourse(Course("X1", "Name", 9999, std::nullopt)), ValidationError);
    EXPECT_THROW(world.accounts.saveCourse(Course("", "Name", seed.deptCs, std::nullopt)), ValidationError);
}

TEST_F(ServiceTest, AdministratorCanAssignALecturerAndEnrolAStudent) {
    world.accounts.assignLecturer("MA101", seed.lectCs);
    EXPECT_EQ(world.courses.findById("MA101")->getLecturerId(), std::optional<int>(seed.lectCs));
    world.accounts.enrolStudent(seed.student3, "CS101");
    EXPECT_TRUE(world.courses.isEnrolled(seed.student3, "CS101"));
}

// ------------------------------------------------------- assignments and extensions (FR-07, FR-08)

TEST_F(ServiceTest, LecturerCreatesAssignmentsOnlyForOwnCourses) {
    EXPECT_GT(world.assignments.createAssignment(seed.lectCs, "CS101", "Quiz", seed.deadline, 10.0), 0);
    EXPECT_THROW(world.assignments.createAssignment(seed.lectMath, "CS101", "Quiz", seed.deadline, 10.0), AccessDeniedError);
    EXPECT_THROW(world.assignments.createAssignment(seed.lectCs, "NOPE", "Quiz", seed.deadline, 10.0), ValidationError);
    EXPECT_THROW(world.assignments.createAssignment(seed.lectCs, "CS101", " ", seed.deadline, 10.0), ValidationError);
    EXPECT_THROW(world.assignments.createAssignment(seed.lectCs, "CS101", "Quiz", seed.deadline, 0.0), ValidationError);
}

TEST_F(ServiceTest, ExtensionChangesOnlyThatStudentsDeadline) {
    const DateTime extended = seed.deadline + hours(48);
    world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, extended);
    EXPECT_EQ(world.assignments.effectiveDeadline(seed.assignment, seed.student1), extended);
    EXPECT_EQ(world.assignments.effectiveDeadline(seed.assignment, seed.student2), seed.deadline);
}

TEST_F(ServiceTest, GrantingAgainReplacesTheEarlierExtension) {
    world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, seed.deadline + hours(24));
    world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, seed.deadline + hours(72));
    EXPECT_EQ(world.assignments.effectiveDeadline(seed.assignment, seed.student1), seed.deadline + hours(72));
    EXPECT_EQ(world.extensions.findAll().size(), 1u);
}

TEST_F(ServiceTest, ExtensionRules) {
    EXPECT_THROW(world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, seed.deadline), ValidationError);  // not later
    EXPECT_THROW(world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, seed.deadline - hours(1)), ValidationError);
    EXPECT_THROW(world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student3, seed.deadline + hours(1)), ValidationError);  // not enrolled
    EXPECT_THROW(world.assignments.grantExtension(seed.lectMath, seed.assignment, seed.student1, seed.deadline + hours(1)), AccessDeniedError);
    EXPECT_THROW(world.assignments.effectiveDeadline(9999, seed.student1), ValidationError);
}

TEST_F(ServiceTest, StudentsSeeOnlyTheirOwnCoursesAndAssignments) {
    EXPECT_EQ(world.assignments.assignmentsForStudent(seed.student1).size(), 1u);
    EXPECT_TRUE(world.assignments.assignmentsForStudent(seed.student3).empty());
    EXPECT_EQ(world.assignments.assignmentsForLecturer(seed.lectCs).size(), 1u);
    EXPECT_TRUE(world.assignments.assignmentsForLecturer(seed.lectMath).empty());
}

// ------------------------------------------------------------------- submitting (FR-09 to FR-11)

TEST_F(ServiceTest, OnTimeWorkIsSubmitted) {
    EXPECT_EQ(submitAt(-hours(5)), SubmissionStatus::Submitted);
}

TEST_F(ServiceTest, WorkExactlyAtTheDeadlineIsOnTimeAndOneSecondLaterIsLate) {
    EXPECT_EQ(submitAt(seconds(0), seed.student1), SubmissionStatus::Submitted);
    EXPECT_EQ(submitAt(seconds(1), seed.student2), SubmissionStatus::Late);
}

TEST_F(ServiceTest, LateWorkIsAcceptedAndSavedAsLate) {
    EXPECT_EQ(submitAt(hours(100)), SubmissionStatus::Late);  // even 100 hours late is accepted, not rejected
    EXPECT_EQ(world.submissionService.findFor(seed.student1, seed.assignment)->getStatus(), SubmissionStatus::Late);
}

TEST_F(ServiceTest, AnExtensionMovesTheLateBoundary) {
    world.assignments.grantExtension(seed.lectCs, seed.assignment, seed.student1, seed.deadline + hours(48));
    EXPECT_EQ(submitAt(hours(30), seed.student1), SubmissionStatus::Submitted);
    EXPECT_EQ(submitAt(hours(30), seed.student2), SubmissionStatus::Late);
}

TEST_F(ServiceTest, SubmissionRules) {
    EXPECT_THROW(world.submissionService.submit(seed.student3, seed.assignment, pdf, seed.deadline), AccessDeniedError);  // not enrolled
    EXPECT_THROW(world.submissionService.submit(seed.student1, seed.assignment, world.makeFile("bad.exe"), seed.deadline), ValidationError);
    EXPECT_THROW(world.submissionService.submit(seed.student1, seed.assignment, world.makeFile("big.pdf", 5 * 1024 * 1024 + 1), seed.deadline), ValidationError);
    EXPECT_THROW(world.submissionService.submit(seed.student1, 9999, pdf, seed.deadline), ValidationError);
    submitAt(-hours(1));
    EXPECT_THROW(submitAt(-hours(1)), ValidationError);  // a second submit must be a resubmit
}

TEST_F(ServiceTest, ARejectedFileLeavesNoSubmissionBehind) {
    EXPECT_THROW(world.submissionService.submit(seed.student1, seed.assignment, world.makeFile("bad.exe"), seed.deadline), ValidationError);
    EXPECT_FALSE(world.submissionService.findFor(seed.student1, seed.assignment).has_value());
}

// --------------------------------------------------------------------------- resubmitting (FR-10)

TEST_F(ServiceTest, ResubmittingBeforeTheDeadlineReplacesTheEarlierSubmission) {
    submitAt(-hours(10));
    const int first = submissionId(seed.student1);
    const std::string newFile = world.makeFile("better.pdf");
    EXPECT_EQ(world.submissionService.resubmit(seed.student1, seed.assignment, newFile, seed.deadline - hours(2)), SubmissionStatus::Submitted);
    EXPECT_EQ(submissionId(seed.student1), first);  // same row
    EXPECT_EQ(world.submissions.findAll().size(), 1u);
    EXPECT_EQ(world.submissionService.findFor(seed.student1, seed.assignment)->getSubmittedAt(), seed.deadline - hours(2));
}

TEST_F(ServiceTest, ResubmittingAfterTheDeadlineIsRejected) {
    submitAt(-hours(10));
    EXPECT_THROW(world.submissionService.resubmit(seed.student1, seed.assignment, pdf, seed.deadline + seconds(1)), ValidationError);
}

TEST_F(ServiceTest, LateWorkCannotBeResubmitted) {
    submitAt(hours(5));
    EXPECT_THROW(world.submissionService.resubmit(seed.student1, seed.assignment, pdf, seed.deadline + hours(6)), ValidationError);
}

TEST_F(ServiceTest, MarkedWorkCannotBeResubmitted) {
    submitAt(-hours(10));
    world.submissionService.markSubmission(seed.lectCs, submissionId(seed.student1), 15.0, "ok", seed.deadline);
    EXPECT_THROW(world.submissionService.resubmit(seed.student1, seed.assignment, pdf, seed.deadline - hours(1)), ValidationError);
}

TEST_F(ServiceTest, ResubmittingNeedsAnEarlierSubmission) {
    EXPECT_THROW(world.submissionService.resubmit(seed.student1, seed.assignment, pdf, seed.deadline - hours(1)), ValidationError);
}

// --------------------------------------------------------------- marking and the late penalty (FR-12)

struct PenaltyCase {
    long long lateSeconds;
    double expectedFinalMark;  // raw mark 15, maximum 20
};

class MarkingPenalty : public ServiceTest, public ::testing::WithParamInterface<PenaltyCase> {};

TEST_P(MarkingPenalty, FinalMarkAfterThePenalty) {
    submitAt(seconds(GetParam().lateSeconds));
    const double finalMark = world.submissionService.markSubmission(seed.lectCs, submissionId(seed.student1), 15.0, "fb", seed.deadline);
    EXPECT_DOUBLE_EQ(finalMark, GetParam().expectedFinalMark) << GetParam().lateSeconds << " s late";
}

INSTANTIATE_TEST_SUITE_P(Boundaries, MarkingPenalty,
                         ::testing::Values(PenaltyCase{-3600, 15.0},   // on time
                                           PenaltyCase{0, 15.0},       // exactly on time
                                           PenaltyCase{1, 13.0},       // 1 s late: -10%
                                           PenaltyCase{86400, 13.0},   // 24 h
                                           PenaltyCase{86401, 11.0},   // 24 h 1 s
                                           PenaltyCase{172800, 11.0},  // 48 h
                                           PenaltyCase{172801, 9.0},   // 48 h 1 s
                                           PenaltyCase{259200, 9.0},   // 72 h: -30%
                                           PenaltyCase{259201, 0.0}),  // 72 h 1 s: zero
                         [](const ::testing::TestParamInfo<PenaltyCase>& info) {
                             const long long s = info.param.lateSeconds;
                             return (s < 0 ? std::string("early_") : std::string("late_")) + std::to_string(s < 0 ? -s : s) + "s";
                         });

TEST_F(ServiceTest, TheFinalMarkNeverGoesBelowZero) {
    submitAt(hours(60));  // 60 h late: -6 from a raw mark of 3
    EXPECT_DOUBLE_EQ(world.submissionService.markSubmission(seed.lectCs, submissionId(seed.student1), 3.0, "fb", seed.deadline), 0.0);
}

TEST_F(ServiceTest, MarkingSavesMarkFeedbackStatusAndAnAuditEntry) {
    submitAt(-hours(1));
    const int id = submissionId(seed.student1);
    world.submissionService.markSubmission(seed.lectCs, id, 15.0, "Good work", seed.deadline);
    const Submission saved = world.submissionService.getSubmission(id);
    EXPECT_EQ(saved.getStatus(), SubmissionStatus::Marked);
    EXPECT_EQ(saved.getMark(), std::optional<double>(15.0));
    EXPECT_EQ(saved.getFeedback(), "Good work");

    const auto log = world.auditLog.findAll();
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0].getSubmissionId(), id);
    EXPECT_EQ(log[0].getChangedBy(), seed.lectCs);
    EXPECT_FALSE(log[0].getOldMark().has_value());
    EXPECT_DOUBLE_EQ(log[0].getNewMark(), 15.0);
}

TEST_F(ServiceTest, MarkingRules) {
    submitAt(-hours(1));
    const int id = submissionId(seed.student1);
    EXPECT_THROW(world.submissionService.markSubmission(seed.lectCs, id, 21.0, "x", seed.deadline), ValidationError);   // above the maximum
    EXPECT_THROW(world.submissionService.markSubmission(seed.lectCs, id, -1.0, "x", seed.deadline), ValidationError);
    EXPECT_THROW(world.submissionService.markSubmission(seed.lectMath, id, 10.0, "x", seed.deadline), AccessDeniedError);  // not the lecturer
    EXPECT_THROW(world.submissionService.markSubmission(seed.lectCs, 9999, 10.0, "x", seed.deadline), ValidationError);
    EXPECT_NO_THROW(world.submissionService.markSubmission(seed.lectCs, id, 20.0, "x", seed.deadline));   // the maximum itself is allowed
    EXPECT_THROW(world.submissionService.markSubmission(seed.lectCs, id, 10.0, "x", seed.deadline), ValidationError);  // already marked
}

TEST_F(ServiceTest, LecturerSeesOnlyWorkWaitingForMarking) {
    submitAt(-hours(1), seed.student1);
    submitAt(-hours(1), seed.student2);
    EXPECT_EQ(world.submissionService.awaitingMarking(seed.lectCs).size(), 2u);
    world.submissionService.markSubmission(seed.lectCs, submissionId(seed.student1), 10.0, "x", seed.deadline);
    EXPECT_EQ(world.submissionService.awaitingMarking(seed.lectCs).size(), 1u);
    EXPECT_TRUE(world.submissionService.awaitingMarking(seed.lectMath).empty());
}

// ------------------------------------------------------------ moderation and release (FR-13 to FR-15, FR-17)

class ModerationTest : public ServiceTest {
protected:
    // student1's on-time work, marked 15/20 by the lecturer.
    int markedSubmission() {
        submitAt(-hours(1));
        const int id = submissionId(seed.student1);
        world.submissionService.markSubmission(seed.lectCs, id, 15.0, "Good work", seed.deadline);
        return id;
    }
};

TEST_F(ModerationTest, HodSeesOnlyMarkedWorkInTheirOwnDepartment) {
    const int id = markedSubmission();
    const auto mine = world.moderation.getMarkedForDepartment(seed.hodCs);
    ASSERT_EQ(mine.size(), 1u);
    EXPECT_EQ(mine[0].getSubmissionId(), id);
    EXPECT_TRUE(world.moderation.getMarkedForDepartment(seed.hodMath).empty());
}

TEST_F(ModerationTest, ChangingAMarkNeedsAReason) {
    const int id = markedSubmission();
    EXPECT_THROW(world.moderation.changeMark(seed.hodCs, id, 12.0, "", seed.deadline), ValidationError);
    EXPECT_THROW(world.moderation.changeMark(seed.hodCs, id, 12.0, "   ", seed.deadline), ValidationError);
    EXPECT_DOUBLE_EQ(*world.submissionService.getSubmission(id).getMark(), 15.0);  // unchanged
}

TEST_F(ModerationTest, ChangingAMarkIsLoggedWithWhoWhenOldNewAndWhy) {
    const int id = markedSubmission();
    const DateTime when = seed.deadline + hours(30);
    world.moderation.changeMark(seed.hodCs, id, 17.0, "Rubric point missed", when);
    EXPECT_DOUBLE_EQ(*world.submissionService.getSubmission(id).getMark(), 17.0);

    const auto log = world.auditLog.findAll();
    ASSERT_EQ(log.size(), 2u);  // the lecturer's first mark, then this change
    EXPECT_EQ(log[1].getChangedBy(), seed.hodCs);
    EXPECT_EQ(log[1].getChangedAt(), when);
    EXPECT_EQ(log[1].getOldMark(), std::optional<double>(15.0));
    EXPECT_DOUBLE_EQ(log[1].getNewMark(), 17.0);
    EXPECT_EQ(log[1].getReason(), "Rubric point missed");
}

TEST_F(ModerationTest, ModerationRules) {
    const int id = markedSubmission();
    EXPECT_THROW(world.moderation.changeMark(seed.hodCs, id, 25.0, "too high", seed.deadline), ValidationError);
    EXPECT_THROW(world.moderation.changeMark(seed.hodCs, id, -1.0, "negative", seed.deadline), ValidationError);
    EXPECT_THROW(world.moderation.changeMark(seed.hodMath, id, 10.0, "wrong dept", seed.deadline), AccessDeniedError);
    EXPECT_THROW(world.moderation.approveMark(seed.hodMath, id), AccessDeniedError);
    EXPECT_THROW(world.moderation.releaseMarks(seed.hodMath, {id}), AccessDeniedError);
    EXPECT_THROW(world.moderation.approveMark(seed.hodCs, 9999), ValidationError);
}

TEST_F(ModerationTest, MarksMustBeApprovedBeforeTheyCanBeReleased) {
    const int id = markedSubmission();
    EXPECT_THROW(world.moderation.releaseMarks(seed.hodCs, {id}), ValidationError);
    world.moderation.approveMark(seed.hodCs, id);
    EXPECT_EQ(world.submissionService.getSubmission(id).getStatus(), SubmissionStatus::Moderated);
    EXPECT_EQ(world.moderation.getModeratedForDepartment(seed.hodCs).size(), 1u);
    EXPECT_EQ(world.moderation.releaseMarks(seed.hodCs, {id}), 1);
    EXPECT_EQ(world.submissionService.getSubmission(id).getStatus(), SubmissionStatus::Released);
}

TEST_F(ModerationTest, ApprovedWorkCannotBeChangedOrApprovedAgain) {
    const int id = markedSubmission();
    world.moderation.approveMark(seed.hodCs, id);
    EXPECT_THROW(world.moderation.changeMark(seed.hodCs, id, 10.0, "late change", seed.deadline), ValidationError);
    EXPECT_THROW(world.moderation.approveMark(seed.hodCs, id), ValidationError);
}

TEST_F(ModerationTest, OneBadIdReleasesNothing) {
    const int id = markedSubmission();
    world.moderation.approveMark(seed.hodCs, id);
    submitAt(-hours(1), seed.student2);  // student2's work is not marked or approved
    const int other = submissionId(seed.student2);
    EXPECT_THROW(world.moderation.releaseMarks(seed.hodCs, {id, other}), ValidationError);
    EXPECT_EQ(world.submissionService.getSubmission(id).getStatus(), SubmissionStatus::Moderated);  // still not released
}

TEST_F(ModerationTest, StudentsSeeNothingUntilTheHodReleases) {
    const int id = markedSubmission();
    EXPECT_TRUE(world.submissionService.releasedFor(seed.student1).empty());  // marked
    world.moderation.approveMark(seed.hodCs, id);
    EXPECT_TRUE(world.submissionService.releasedFor(seed.student1).empty());  // approved, not yet released
    world.moderation.releaseMarks(seed.hodCs, {id});
    const auto released = world.submissionService.releasedFor(seed.student1);
    ASSERT_EQ(released.size(), 1u);
    EXPECT_DOUBLE_EQ(*released[0].getMark(), 15.0);
    EXPECT_EQ(released[0].getFeedback(), "Good work");
    EXPECT_TRUE(world.submissionService.releasedFor(seed.student2).empty());  // students see only their own
}

}  // namespace

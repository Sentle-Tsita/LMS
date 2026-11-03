// System tests: the real console menus are driven by scripted input, like a user at the keyboard.
// They follow the use cases and include exception flows (wrong password, bad file, bad input, closed input).
#include <gtest/gtest.h>

#include <sstream>

#include "TestWorld.hpp"
#include "ui/ConsoleApp.hpp"

namespace {

class SystemTest : public ::testing::Test {
protected:
    void SetUp() override { seed = world.seedStandard(); }

    // Runs the application with the given keystrokes and returns everything it printed.
    std::string run(const std::string& script) {
        std::istringstream in(script);
        std::ostringstream out;
        ConsoleApp app(world.accounts, world.assignments, world.submissionService, world.moderation, in, out);
        app.run();
        return out.str();
    }

    static bool has(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

    TestWorld world;
    Seed seed;
};

// ST-01 (FR-01): login
TEST_F(SystemTest, WrongPasswordIsRefusedAndTheRightOneWorks) {
    const std::string out = run("student1\nwrong\nstudent1\nPassword123\n0\n\n");
    EXPECT_TRUE(has(out, "Wrong username or password"));
    EXPECT_TRUE(has(out, "Welcome, student1."));
    EXPECT_TRUE(has(out, "Student menu"));
}

// ST-02 (FR-02): each role gets its own menu, and cannot reach another role's functions
TEST_F(SystemTest, EachRoleSeesOnlyItsOwnMenu) {
    EXPECT_TRUE(has(run("student1\nPassword123\n0\n\n"), "Submit assignment"));
    EXPECT_TRUE(has(run("lect_cs\nPassword123\n0\n\n"), "Grant extension"));
    EXPECT_TRUE(has(run("hod_cs\nPassword123\n0\n\n"), "Release approved marks"));
    const std::string student = run("student1\nPassword123\n0\n\n");
    EXPECT_FALSE(has(student, "Grant extension"));
    EXPECT_FALSE(has(student, "Release approved marks"));
}

TEST_F(SystemTest, AStudentCannotUseAnotherRolesOptions) {
    const std::string out = run("student1\nPassword123\n4\n0\n\n");  // option 4 does not exist for a student
    EXPECT_TRUE(has(out, "Unknown option."));
}

// ST-03 (FR-03): a deactivated user cannot log in
TEST_F(SystemTest, DeactivatedUserCannotLogIn) {
    world.accounts.deactivateUser(seed.student1);
    EXPECT_TRUE(has(run("student1\nPassword123\n\n"), "deactivated"));
}

// ST-04 (FR-03 to FR-06): the administrator sets up the school through the menus
TEST_F(SystemTest, AdministratorSetsUpAccountsCoursesAndEnrolment) {
    world.accounts.createUser("ADMINISTRATOR", "admin1", "Password123");
    const std::string script =
        "admin1\nPassword123\n"
        "1\n1\nSTUDENT\nnewstudent\nPassword123\n"  // create account
        "2\nPhysics\n\n"                              // create department, no head yet
        "4\n2\n" + std::to_string(seed.student3) + "\nCS101\n"  // enrol student3 in CS101
        "0\n\n";
    const std::string out = run(script);
    EXPECT_TRUE(has(out, "Account created with id"));
    EXPECT_TRUE(has(out, "Department created with id"));
    EXPECT_TRUE(has(out, "Student enrolled."));
    EXPECT_TRUE(world.courses.isEnrolled(seed.student3, "CS101"));
    EXPECT_NE(world.accounts.login("newstudent", "Password123"), nullptr);
}

TEST_F(SystemTest, FirstRunCreatesTheFirstAdministrator) {
    TestWorld empty;  // no users at all
    std::istringstream in("firstadmin\nPassword123\nfirstadmin\nPassword123\n0\n\n");
    std::ostringstream out;
    ConsoleApp app(empty.accounts, empty.assignments, empty.submissionService, empty.moderation, in, out);
    app.run();
    EXPECT_TRUE(has(out.str(), "Administrator account created."));
    EXPECT_TRUE(has(out.str(), "Administrator menu"));
}

// ST-05 (UC Submit assignment, main flow): lecturer creates, student sees and submits
TEST_F(SystemTest, LecturerCreatesAnAssignmentAndStudentSeesAndSubmitsIt) {
    std::string out = run("lect_cs\nPassword123\n1\nCS101\nQuiz 1\n2099-01-01 00:00\n10\n0\n\n");
    EXPECT_TRUE(has(out, "created."));

    const std::string file = world.makeFile("answer.pdf");
    out = run("student1\nPassword123\n1\n0\n\n");
    EXPECT_TRUE(has(out, "CS101 - Introduction to Programming"));
    EXPECT_TRUE(has(out, "Quiz 1"));

    const int quizId = world.assignmentRepo.findAll().back().getAssignmentId();
    out = run("student1\nPassword123\n2\n" + std::to_string(quizId) + "\n" + file + "\n0\n\n");
    EXPECT_TRUE(has(out, "Submitted on time."));
}

// ST-06 (UC Submit assignment, alternative flow): late work is accepted, not rejected
TEST_F(SystemTest, LateWorkIsAcceptedAndFlaggedLate) {
    const int oldAssignment = world.assignments.createAssignment(seed.lectCs, "CS101", "Old task", fromIsoString("2020-01-01 00:00:00"), 20.0);
    const std::string file = world.makeFile("late.pdf");
    const std::string out = run("student1\nPassword123\n2\n" + std::to_string(oldAssignment) + "\n" + file + "\n0\n\n");
    EXPECT_TRUE(has(out, "Submitted LATE"));
    EXPECT_EQ(world.submissionService.findFor(seed.student1, oldAssignment)->getStatus(), SubmissionStatus::Late);
}

// ST-07 (UC Submit assignment, alternative flow): resubmission replaces the earlier file
TEST_F(SystemTest, ResubmittingBeforeTheDeadlineReplacesTheFile) {
    const int future = world.assignments.createAssignment(seed.lectCs, "CS101", "Future task", fromIsoString("2099-01-01 00:00:00"), 20.0);
    const std::string file = world.makeFile("v1.pdf");
    const std::string second = world.makeFile("v2.pdf");
    run("student1\nPassword123\n2\n" + std::to_string(future) + "\n" + file + "\n0\n\n");
    const std::string out = run("student1\nPassword123\n2\n" + std::to_string(future) + "\n" + second + "\n0\n\n");
    EXPECT_TRUE(has(out, "Resubmitted."));
    EXPECT_EQ(world.submissions.findWhere({{"assignment_id", future}}).size(), 1u);
}

// ST-08 (UC Submit assignment, exception flows): bad file type, missing file, not a number
TEST_F(SystemTest, BadInputShowsAMessageAndTheMenuCarriesOn) {
    const int future = world.assignments.createAssignment(seed.lectCs, "CS101", "Future task", fromIsoString("2099-01-01 00:00:00"), 20.0);
    const std::string exe = world.makeFile("program.exe");
    std::string out = run("student1\nPassword123\n2\n" + std::to_string(future) + "\n" + exe + "\n0\n\n");
    EXPECT_TRUE(has(out, "is not allowed"));
    EXPECT_TRUE(has(out, "Logged out."));  // the program did not crash

    out = run("student1\nPassword123\n2\n" + std::to_string(future) + "\nC:/no/such/file.pdf\n0\n\n");
    EXPECT_TRUE(has(out, "File not found"));

    out = run("student1\nPassword123\nabc\n0\n\n");  // letters instead of a menu number
    EXPECT_TRUE(has(out, "Please enter a whole number."));

    out = run("student1\nPassword123\n2\n9999\nfile.pdf\n0\n\n");  // no such assignment
    EXPECT_TRUE(has(out, "No assignment with id 9999"));
}

TEST_F(SystemTest, TheProgramEndsCleanlyWhenInputCloses) {
    EXPECT_NO_THROW(run("student1\nPassword123\n"));  // input ends while the menu is waiting
    EXPECT_NO_THROW(run(""));
}

// ST-09 (UC Grant extension): only that student's deadline changes
TEST_F(SystemTest, LecturerGrantsAnExtensionToOneStudent) {
    const std::string out = run("lect_cs\nPassword123\n2\n" + std::to_string(seed.assignment) + "\n" +
                                std::to_string(seed.student1) + "\n2026-11-20 12:00\n0\n\n");
    EXPECT_TRUE(has(out, "Extension granted"));
    EXPECT_EQ(world.assignments.effectiveDeadline(seed.assignment, seed.student1), fromIsoString("2026-11-20 12:00:00"));
    EXPECT_EQ(world.assignments.effectiveDeadline(seed.assignment, seed.student2), seed.deadline);
}

TEST_F(SystemTest, InvalidDatesAreAskedForAgain) {
    const std::string out = run("lect_cs\nPassword123\n2\n" + std::to_string(seed.assignment) + "\n" +
                                std::to_string(seed.student1) + "\ntomorrow\n2026-11-20 12:00\n0\n\n");
    EXPECT_TRUE(has(out, "Invalid date and time"));
    EXPECT_TRUE(has(out, "Extension granted"));
}

// ST-10 (UC Moderate and release marks): the complete process from submission to released mark
TEST_F(SystemTest, FullProcessFromSubmissionToReleasedMark) {
    const std::string file = world.makeFile("essay.pdf");
    world.submissionService.submit(seed.student1, seed.assignment, file, seed.deadline - std::chrono::hours(2));
    const std::string id = std::to_string(world.submissionService.findFor(seed.student1, seed.assignment)->getSubmissionId());

    // The student sees no mark yet.
    std::string out = run("student1\nPassword123\n3\n0\n\n");
    EXPECT_TRUE(has(out, "mark not released yet"));

    // The lecturer marks it.
    out = run("lect_cs\nPassword123\n3\n" + id + "\n15\nGood work\n0\n\n");
    EXPECT_TRUE(has(out, "Final mark: 15"));

    // Still no mark for the student: the HoD has not moderated it.
    out = run("student1\nPassword123\n3\n0\n\n");
    EXPECT_TRUE(has(out, "mark not released yet"));
    EXPECT_FALSE(has(out, "Good work"));

    // The HoD tries to change the mark with no reason (refused), then gives one, approves, and releases.
    out = run("hod_cs\nPassword123\n1\n" + id + "\n2\n17\n\n2\n17\nRubric point missed\n1\n2\ny\n0\n\n");
    EXPECT_TRUE(has(out, "Error: A reason is required"));
    EXPECT_TRUE(has(out, "Mark changed and logged."));
    EXPECT_TRUE(has(out, "Approved."));
    EXPECT_TRUE(has(out, "1 mark(s) released."));

    // Now the student sees the moderated mark and the written feedback.
    out = run("student1\nPassword123\n3\n0\n\n");
    EXPECT_TRUE(has(out, "17 / 20"));
    EXPECT_TRUE(has(out, "Good work"));

    // Both mark changes are in the audit log.
    EXPECT_EQ(world.auditLog.findAll().size(), 2u);
}

// ST-11: a HoD cannot touch another department's work
TEST_F(SystemTest, HodCannotModerateAnotherDepartmentsWork) {
    world.submissionService.submit(seed.student1, seed.assignment, world.makeFile("e.pdf"), seed.deadline - std::chrono::hours(2));
    const int id = world.submissionService.findFor(seed.student1, seed.assignment)->getSubmissionId();
    world.submissionService.markSubmission(seed.lectCs, id, 15.0, "ok", seed.deadline);
    const std::string out = run("hod_math\nPassword123\n1\n0\n\n");
    EXPECT_TRUE(has(out, "No marks are waiting for review."));
}

// ST-12: late penalty seen through the menus (72 h 1 s late gives zero)
TEST_F(SystemTest, VeryLateWorkIsMarkedZeroThroughTheMenus) {
    const std::string file = world.makeFile("late.pdf");
    world.submissionService.submit(seed.student1, seed.assignment, file, seed.deadline + std::chrono::seconds(259201));
    const std::string id = std::to_string(world.submissionService.findFor(seed.student1, seed.assignment)->getSubmissionId());
    const std::string out = run("lect_cs\nPassword123\n3\n" + id + "\n18\nGood but late\n0\n\n");
    EXPECT_TRUE(has(out, "Final mark: 0."));
    EXPECT_TRUE(has(out, "LATE"));
}

}  // namespace

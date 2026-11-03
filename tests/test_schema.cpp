// Tests of the SQL schema from the design: tables, keys and constraints.
#include <gtest/gtest.h>

#include "TestWorld.hpp"

namespace {

class SchemaTest : public ::testing::Test {
protected:
    SchemaTest() : db(":memory:") { db.initialiseSchema(LMS_SCHEMA_PATH); }

    // user ids: 1 student, 2 lecturer, 3 head of department
    void seedBasics() {
        db.execute(
            "INSERT INTO user (username, password_hash, salt, role) VALUES "
            "('stu1','h','s','STUDENT'), ('lec1','h','s','LECTURER'), ('hod1','h','s','HOD');"
            "INSERT INTO department (name, hod_user_id) VALUES ('Maths', 3);"
            "INSERT INTO course (course_code, course_name, department_id, lecturer_id) VALUES ('MTH101', 'Algebra', 1, 2);"
            "INSERT INTO assignment (course_code, title, deadline, max_mark) VALUES ('MTH101', 'A1', '2026-11-01 12:00:00', 100);"
            "INSERT INTO submission (assignment_id, student_id, file_path, submitted_at) VALUES (1, 1, 'f.pdf', '2026-10-30 10:00:00');");
    }

    Database db;
};

TEST_F(SchemaTest, AllEightTablesExist) {
    for (const char* table : {"user", "department", "course", "enrol", "assignment", "extension", "submission", "audit_log"}) {
        EXPECT_TRUE(db.tableExists(table)) << table;
    }
}

TEST_F(SchemaTest, SchemaIsCreatedOnlyOnce) {
    EXPECT_FALSE(db.initialiseSchema(LMS_SCHEMA_PATH));
    Database fresh(":memory:");
    EXPECT_TRUE(fresh.initialiseSchema(LMS_SCHEMA_PATH));
}

TEST_F(SchemaTest, MissingSchemaFileThrows) {
    Database fresh(":memory:");
    EXPECT_THROW(fresh.initialiseSchema("no_such_file.sql"), DatabaseError);
}

TEST_F(SchemaTest, TableNameLookupIsNotInjectable) {
    EXPECT_FALSE(db.tableExists("x'; DROP TABLE user;--"));
    EXPECT_TRUE(db.tableExists("user"));
}

TEST_F(SchemaTest, RejectsInvalidSql) { EXPECT_THROW(db.execute("NOT SQL"), DatabaseError); }

TEST_F(SchemaTest, RejectsUnknownRole) {
    EXPECT_THROW(db.execute("INSERT INTO user (username, password_hash, salt, role) VALUES ('x','h','s','JANITOR');"), DatabaseError);
}

TEST_F(SchemaTest, UsernamesAreUnique) {
    db.execute("INSERT INTO user (username, password_hash, salt, role) VALUES ('x','h','s','STUDENT');");
    EXPECT_THROW(db.execute("INSERT INTO user (username, password_hash, salt, role) VALUES ('x','h2','s2','LECTURER');"), DatabaseError);
}

TEST_F(SchemaTest, ForeignKeysAreEnforced) {
    EXPECT_THROW(db.execute("INSERT INTO course (course_code, course_name, department_id) VALUES ('MTH101', 'Algebra', 999);"), DatabaseError);
}

TEST_F(SchemaTest, RejectsUnknownSubmissionStatus) {
    seedBasics();
    EXPECT_THROW(db.execute("UPDATE submission SET status = 'DONE' WHERE submission_id = 1;"), DatabaseError);
}

TEST_F(SchemaTest, AcceptsEverySubmissionStatusInTheStateMachine) {
    seedBasics();
    for (const char* status : {"SUBMITTED", "LATE", "MARKED", "MODERATED", "RELEASED", "UNDER_REVIEW"}) {
        EXPECT_NO_THROW(db.execute(std::string("UPDATE submission SET status = '") + status + "' WHERE submission_id = 1;")) << status;
    }
}

TEST_F(SchemaTest, MarkCannotBeNegative) {
    seedBasics();
    EXPECT_THROW(db.execute("UPDATE submission SET mark = -1 WHERE submission_id = 1;"), DatabaseError);
}

TEST_F(SchemaTest, OneSubmissionPerStudentPerAssignment) {
    seedBasics();
    EXPECT_THROW(db.execute("INSERT INTO submission (assignment_id, student_id, file_path, submitted_at) "
                            "VALUES (1, 1, 'g.pdf', '2026-10-31 10:00:00');"),
                 DatabaseError);
}

TEST_F(SchemaTest, OneExtensionPerStudentPerAssignment) {
    seedBasics();
    db.execute("INSERT INTO extension (assignment_id, student_id, new_deadline) VALUES (1, 1, '2026-11-03 12:00:00');");
    EXPECT_THROW(db.execute("INSERT INTO extension (assignment_id, student_id, new_deadline) VALUES (1, 1, '2026-11-05 12:00:00');"), DatabaseError);
}

TEST_F(SchemaTest, EnrolmentCannotBeRepeated) {
    seedBasics();
    db.execute("INSERT INTO enrol (student_id, course_code) VALUES (1, 'MTH101');");
    EXPECT_THROW(db.execute("INSERT INTO enrol (student_id, course_code) VALUES (1, 'MTH101');"), DatabaseError);
}

TEST_F(SchemaTest, MaximumMarkMustBePositive) {
    seedBasics();
    EXPECT_THROW(db.execute("INSERT INTO assignment (course_code, title, deadline, max_mark) VALUES ('MTH101', 'Bad', '2026-11-01 12:00:00', 0);"), DatabaseError);
}

TEST_F(SchemaTest, AuditLogRequiresAReason) {
    seedBasics();
    EXPECT_THROW(db.execute("INSERT INTO audit_log (submission_id, changed_by, changed_at, old_mark, new_mark, reason) "
                            "VALUES (1, 3, '2026-11-02 09:00:00', 50, 60, '   ');"),
                 DatabaseError);
}

TEST_F(SchemaTest, AuditLogAcceptsACompleteEntry) {
    seedBasics();
    EXPECT_NO_THROW(db.execute("INSERT INTO audit_log (submission_id, changed_by, changed_at, old_mark, new_mark, reason) "
                               "VALUES (1, 3, '2026-11-02 09:00:00', 50, 60, 'Marking error');"));
}

TEST(DatabaseConnection, OpeningAnImpossiblePathThrows) {
    EXPECT_THROW(Database("no_such_folder_xyz/test.db"), DatabaseError);
}

}  // namespace

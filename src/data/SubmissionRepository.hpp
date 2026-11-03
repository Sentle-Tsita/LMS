#pragma once
#include <optional>
#include <vector>
#include "data/SqliteRepository.hpp"
#include "domain/Submission.hpp"

// Submission repository plus the queries that need joins across assignment, course and department.
class SubmissionRepository : public SqliteRepository<Submission, int> {
public:
    SubmissionRepository(Database& database, const EntityMapper<Submission, int>& mapper)
        : SqliteRepository<Submission, int>(database, mapper) {}

    // Submissions for courses taught by this lecturer.
    std::vector<Submission> findForLecturer(int lecturerId) {
        Statement statement(db_, "SELECT " + selectList("s.") + " FROM submission s " + JOIN_COURSE +
                                 " WHERE c.lecturer_id = ? ORDER BY s.submission_id;");
        statement.bindInt(1, lecturerId);
        return readAll(statement);
    }

    // Submissions for courses in the department headed by this user.
    std::vector<Submission> findForHod(int hodUserId) {
        Statement statement(db_, "SELECT " + selectList("s.") + " FROM submission s " + JOIN_COURSE +
                                 " JOIN department d ON d.department_id = c.department_id "
                                 "WHERE d.hod_user_id = ? ORDER BY s.submission_id;");
        statement.bindInt(1, hodUserId);
        return readAll(statement);
    }

    std::optional<int> lecturerOf(int submissionId) {
        Statement statement(db_, "SELECT c.lecturer_id FROM submission s " + JOIN_COURSE + " WHERE s.submission_id = ?;");
        statement.bindInt(1, submissionId);
        if (!statement.step() || statement.isNull(0)) {
            return std::nullopt;
        }
        return statement.getInt(0);
    }

    std::optional<int> headOf(int submissionId) {
        Statement statement(db_, "SELECT d.hod_user_id FROM submission s " + JOIN_COURSE +
                                 " JOIN department d ON d.department_id = c.department_id WHERE s.submission_id = ?;");
        statement.bindInt(1, submissionId);
        if (!statement.step() || statement.isNull(0)) {
            return std::nullopt;
        }
        return statement.getInt(0);
    }

private:
    static inline const std::string JOIN_COURSE =
        "JOIN assignment a ON a.assignment_id = s.assignment_id JOIN course c ON c.course_code = a.course_code";
};

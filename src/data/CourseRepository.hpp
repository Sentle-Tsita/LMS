#pragma once
#include <string>
#include <vector>
#include "data/SqliteRepository.hpp"
#include "domain/Course.hpp"

// Course repository plus the enrolment link table (there is no Enrolment class, see the design notes).
class CourseRepository : public SqliteRepository<Course, std::string> {
public:
    CourseRepository(Database& database, const EntityMapper<Course, std::string>& mapper)
        : SqliteRepository<Course, std::string>(database, mapper) {}

    void enrol(int studentId, const std::string& courseCode) {
        Statement statement(db_, "INSERT INTO enrol (student_id, course_code) VALUES (?, ?);");
        statement.bindInt(1, studentId);
        statement.bindText(2, courseCode);
        statement.step();
    }

    bool isEnrolled(int studentId, const std::string& courseCode) {
        Statement statement(db_, "SELECT 1 FROM enrol WHERE student_id = ? AND course_code = ?;");
        statement.bindInt(1, studentId);
        statement.bindText(2, courseCode);
        return statement.step();
    }

    std::vector<Course> findByStudent(int studentId) {
        Statement statement(db_, "SELECT " + selectList("c.") + " FROM course c JOIN enrol e ON e.course_code = c.course_code "
                                 "WHERE e.student_id = ? ORDER BY c.course_code;");
        statement.bindInt(1, studentId);
        return readAll(statement);
    }
};

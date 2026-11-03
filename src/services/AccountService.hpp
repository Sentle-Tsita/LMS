#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "data/CourseRepository.hpp"
#include "data/RepositoryTypes.hpp"
#include "domain/Course.hpp"
#include "security/PasswordHasher.hpp"

// Accounts, departments, courses, lecturer assignment and enrolment (FR-01 to FR-06).
class AccountService {
public:
    AccountService(UserRepository& users, DepartmentRepository& departments, CourseRepository& courses,
                   const PasswordHasher& hasher)
        : users_(users), departments_(departments), courses_(courses), hasher_(hasher) {}

    // Returns the user, or nullptr if the username or password is wrong or the account is deactivated.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);

    // role: STUDENT, LECTURER, HOD or ADMINISTRATOR. Returns the new user id.
    int createUser(const std::string& role, const std::string& username, const std::string& password);
    void deactivateUser(int userId);  // accounts are deactivated, never deleted (SRS assumption)

    int createDepartment(const std::string& name, std::optional<int> hodUserId);
    void saveCourse(const Course& course);
    void assignLecturer(const std::string& courseCode, int lecturerId);
    void enrolStudent(int studentId, const std::string& courseCode);

    bool hasUsers();
    std::vector<std::unique_ptr<User>> listUsers();
    std::vector<Department> listDepartments();
    std::vector<Course> listCourses();

private:
    void requireRole(int userId, const std::string& role, const std::string& description);

    UserRepository& users_;
    DepartmentRepository& departments_;
    CourseRepository& courses_;
    const PasswordHasher& hasher_;
};

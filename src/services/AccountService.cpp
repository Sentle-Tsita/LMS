#include "services/AccountService.hpp"

#include "domain/UserFactory.hpp"
#include "services/Errors.hpp"
#include "util/StringUtil.hpp"

std::unique_ptr<User> AccountService::login(const std::string& username, const std::string& password) {
    std::vector<std::unique_ptr<User>> found = users_.findWhere({{"username", username}});
    if (found.empty()) {
        return nullptr;
    }
    std::unique_ptr<User> user = std::move(found.front());
    if (!user->isActive() || !hasher_.verify(password, user->getSalt(), user->getPasswordHash())) {
        return nullptr;
    }
    return user;
}

int AccountService::createUser(const std::string& role, const std::string& username, const std::string& password) {
    const std::string name = trim(username);
    if (name.size() < 3 || name.size() > 30 || name.find(' ') != std::string::npos) {
        throw ValidationError("Username must be 3 to 30 characters with no spaces.");
    }
    if (password.size() < 8) {
        throw ValidationError("Password must be at least 8 characters.");
    }
    if (!users_.findWhere({{"username", name}}).empty()) {
        throw ValidationError("Username '" + name + "' is already taken.");
    }
    const std::string salt = hasher_.newSalt();
    std::unique_ptr<User> user;
    try {
        user = makeUser(role, 0, name, hasher_.hash(password, salt), salt, true);
    } catch (const std::invalid_argument&) {
        throw ValidationError("Unknown role '" + role + "'. Use STUDENT, LECTURER, HOD or ADMINISTRATOR.");
    }
    return users_.save(user);
}

void AccountService::deactivateUser(int userId) {
    std::optional<std::unique_ptr<User>> user = users_.findById(userId);
    if (!user) {
        throw ValidationError("No user with id " + std::to_string(userId) + ".");
    }
    (*user)->setActive(false);
    users_.save(*user);
}

void AccountService::requireRole(int userId, const std::string& role, const std::string& description) {
    std::optional<std::unique_ptr<User>> user = users_.findById(userId);
    if (!user || (*user)->roleName() != role) {
        throw ValidationError("User " + std::to_string(userId) + " is not a " + description + ".");
    }
}

int AccountService::createDepartment(const std::string& name, std::optional<int> hodUserId) {
    const std::string cleaned = trim(name);
    if (cleaned.empty()) {
        throw ValidationError("Department name cannot be empty.");
    }
    if (hodUserId) {
        requireRole(*hodUserId, "HOD", "head of department");
    }
    return departments_.save(Department(0, cleaned, hodUserId));
}

void AccountService::saveCourse(const Course& course) {
    if (trim(course.getCourseCode()).empty() || trim(course.getCourseName()).empty()) {
        throw ValidationError("Course code and name cannot be empty.");
    }
    if (!departments_.findById(course.getDepartmentId())) {
        throw ValidationError("No department with id " + std::to_string(course.getDepartmentId()) + ".");
    }
    if (course.getLecturerId()) {
        requireRole(*course.getLecturerId(), "LECTURER", "lecturer");
    }
    courses_.save(course);
}

void AccountService::assignLecturer(const std::string& courseCode, int lecturerId) {
    std::optional<Course> course = courses_.findById(courseCode);
    if (!course) {
        throw ValidationError("No course with code '" + courseCode + "'.");
    }
    requireRole(lecturerId, "LECTURER", "lecturer");
    course->setLecturerId(lecturerId);
    courses_.save(*course);
}

void AccountService::enrolStudent(int studentId, const std::string& courseCode) {
    requireRole(studentId, "STUDENT", "student");
    if (!courses_.findById(courseCode)) {
        throw ValidationError("No course with code '" + courseCode + "'.");
    }
    if (courses_.isEnrolled(studentId, courseCode)) {
        throw ValidationError("The student is already enrolled in " + courseCode + ".");
    }
    courses_.enrol(studentId, courseCode);
}

bool AccountService::hasUsers() { return !users_.findAll().empty(); }
std::vector<std::unique_ptr<User>> AccountService::listUsers() { return users_.findAll(); }
std::vector<Department> AccountService::listDepartments() { return departments_.findAll(); }
std::vector<Course> AccountService::listCourses() { return courses_.findAll(); }

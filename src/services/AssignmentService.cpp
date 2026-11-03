#include "services/AssignmentService.hpp"

#include "services/Errors.hpp"
#include "util/StringUtil.hpp"

void AssignmentService::requireTeaches(int lecturerId, const std::string& courseCode) {
    std::optional<Course> course = courses_.findById(courseCode);
    if (!course) {
        throw ValidationError("No course with code '" + courseCode + "'.");
    }
    if (!course->getLecturerId() || *course->getLecturerId() != lecturerId) {
        throw AccessDeniedError("You are not the lecturer of " + courseCode + ".");
    }
}

int AssignmentService::createAssignment(int lecturerId, const std::string& courseCode, const std::string& title,
                                        DateTime deadline, double maxMark) {
    requireTeaches(lecturerId, courseCode);
    if (trim(title).empty()) {
        throw ValidationError("Assignment title cannot be empty.");
    }
    if (!(maxMark > 0.0)) {
        throw ValidationError("Maximum mark must be greater than zero.");
    }
    return assignments_.save(Assignment(0, courseCode, trim(title), deadline, maxMark));
}

Assignment AssignmentService::getAssignment(int assignmentId) {
    std::optional<Assignment> assignment = assignments_.findById(assignmentId);
    if (!assignment) {
        throw ValidationError("No assignment with id " + std::to_string(assignmentId) + ".");
    }
    return *assignment;
}

int AssignmentService::grantExtension(int lecturerId, int assignmentId, int studentId, DateTime newDeadline) {
    const Assignment assignment = getAssignment(assignmentId);
    requireTeaches(lecturerId, assignment.getCourseCode());
    if (!courses_.isEnrolled(studentId, assignment.getCourseCode())) {
        throw ValidationError("That student is not enrolled in " + assignment.getCourseCode() + ".");
    }
    if (newDeadline <= assignment.getDeadline()) {
        throw ValidationError("The extended deadline must be later than the original deadline.");
    }
    int existingId = 0;
    const std::vector<Extension> existing = extensions_.findWhere({{"assignment_id", assignmentId}, {"student_id", studentId}});
    if (!existing.empty()) {
        existingId = existing.front().getExtensionId();
    }
    return extensions_.save(Extension(existingId, assignmentId, studentId, newDeadline));
}

DateTime AssignmentService::effectiveDeadline(int assignmentId, int studentId) {
    const Assignment assignment = getAssignment(assignmentId);
    const std::vector<Extension> extension = extensions_.findWhere({{"assignment_id", assignmentId}, {"student_id", studentId}});
    if (!extension.empty()) {
        return extension.front().getNewDeadline();
    }
    return assignment.getDeadline();
}

std::vector<Course> AssignmentService::coursesForLecturer(int lecturerId) {
    return courses_.findWhere({{"lecturer_id", lecturerId}});
}

std::vector<Course> AssignmentService::coursesForStudent(int studentId) { return courses_.findByStudent(studentId); }

std::vector<Assignment> AssignmentService::assignmentsForStudent(int studentId) {
    std::vector<Assignment> result;
    for (const Course& course : courses_.findByStudent(studentId)) {
        const std::vector<Assignment> found = assignments_.findWhere({{"course_code", course.getCourseCode()}});
        result.insert(result.end(), found.begin(), found.end());
    }
    return result;
}

std::vector<Assignment> AssignmentService::assignmentsForLecturer(int lecturerId) {
    std::vector<Assignment> result;
    for (const Course& course : courses_.findWhere({{"lecturer_id", lecturerId}})) {
        const std::vector<Assignment> found = assignments_.findWhere({{"course_code", course.getCourseCode()}});
        result.insert(result.end(), found.begin(), found.end());
    }
    return result;
}

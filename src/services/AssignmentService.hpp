#pragma once
#include <string>
#include <vector>

#include "data/CourseRepository.hpp"
#include "data/RepositoryTypes.hpp"
#include "domain/DateTime.hpp"

// Assignments, extensions and the effective deadline (FR-07, FR-08).
class AssignmentService {
public:
    AssignmentService(AssignmentRepository& assignments, ExtensionRepository& extensions, CourseRepository& courses)
        : assignments_(assignments), extensions_(extensions), courses_(courses) {}

    int createAssignment(int lecturerId, const std::string& courseCode, const std::string& title, DateTime deadline,
                         double maxMark);
    // Gives one student more time. Granting again replaces the earlier extension.
    int grantExtension(int lecturerId, int assignmentId, int studentId, DateTime newDeadline);
    // The extended deadline if the lecturer granted one, otherwise the original deadline.
    DateTime effectiveDeadline(int assignmentId, int studentId);

    Assignment getAssignment(int assignmentId);
    std::vector<Course> coursesForLecturer(int lecturerId);
    std::vector<Course> coursesForStudent(int studentId);
    std::vector<Assignment> assignmentsForStudent(int studentId);
    std::vector<Assignment> assignmentsForLecturer(int lecturerId);

private:
    void requireTeaches(int lecturerId, const std::string& courseCode);

    AssignmentRepository& assignments_;
    ExtensionRepository& extensions_;
    CourseRepository& courses_;
};

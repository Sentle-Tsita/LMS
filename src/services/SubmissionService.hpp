#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include "data/CourseRepository.hpp"
#include "data/RepositoryTypes.hpp"
#include "data/SubmissionRepository.hpp"
#include "domain/LatePenaltyStrategy.hpp"
#include "domain/Submission.hpp"
#include "services/AssignmentService.hpp"
#include "storage/FileStorage.hpp"

// Submitting, resubmitting and marking (FR-09 to FR-12, FR-17 for the first mark).
// The late penalty comes from a LatePenaltyStrategy, so the rule can change without changing this class.
class SubmissionService {
public:
    SubmissionService(SubmissionRepository& submissions, AssignmentService& assignments, CourseRepository& courses,
                      AuditLogRepository& auditLog, const LatePenaltyStrategy& penalty, FileStorage& storage)
        : submissions_(submissions), assignments_(assignments), courses_(courses), auditLog_(auditLog),
          penalty_(penalty), storage_(storage) {}

    // First submission. Late work is accepted and saved with status LATE (FR-09, FR-11).
    SubmissionStatus submit(int studentId, int assignmentId, const std::filesystem::path& file,
                            DateTime now = std::chrono::system_clock::now());
    // Replaces the earlier submission. Only before the effective deadline and before marking (FR-10).
    SubmissionStatus resubmit(int studentId, int assignmentId, const std::filesystem::path& file,
                              DateTime now = std::chrono::system_clock::now());
    // Saves the mark after applying the late penalty, never below zero. Returns the final mark (FR-12).
    double markSubmission(int lecturerId, int submissionId, double rawMark, const std::string& feedback,
                          DateTime now = std::chrono::system_clock::now());

    Submission getSubmission(int submissionId);
    std::optional<Submission> findFor(int studentId, int assignmentId);
    std::vector<Submission> submissionsForStudent(int studentId);
    // Marks and feedback the student is allowed to see: only RELEASED submissions (FR-15, FR-18).
    std::vector<Submission> releasedFor(int studentId);
    std::vector<Submission> awaitingMarking(int lecturerId);

private:
    void requireEnrolled(int studentId, const Assignment& assignment);

    SubmissionRepository& submissions_;
    AssignmentService& assignments_;
    CourseRepository& courses_;
    AuditLogRepository& auditLog_;
    const LatePenaltyStrategy& penalty_;
    FileStorage& storage_;
};

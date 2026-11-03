#include "services/SubmissionService.hpp"

#include <algorithm>
#include <iterator>

#include "services/Errors.hpp"
#include "util/DateTimeUtil.hpp"

void SubmissionService::requireEnrolled(int studentId, const Assignment& assignment) {
    if (!courses_.isEnrolled(studentId, assignment.getCourseCode())) {
        throw AccessDeniedError("You are not enrolled in " + assignment.getCourseCode() + ".");
    }
}

std::optional<Submission> SubmissionService::findFor(int studentId, int assignmentId) {
    std::vector<Submission> found = submissions_.findWhere({{"assignment_id", assignmentId}, {"student_id", studentId}});
    if (found.empty()) {
        return std::nullopt;
    }
    return found.front();
}

SubmissionStatus SubmissionService::submit(int studentId, int assignmentId, const std::filesystem::path& file, DateTime now) {
    now = floorToSeconds(now);
    const Assignment assignment = assignments_.getAssignment(assignmentId);
    requireEnrolled(studentId, assignment);
    if (findFor(studentId, assignmentId)) {
        throw ValidationError("You have already submitted this assignment. Use resubmit to replace it.");
    }
    const std::string stored = storage_.store(file, assignmentId, studentId, now);
    const bool onTime = now <= assignments_.effectiveDeadline(assignmentId, studentId);
    Submission submission(studentId, assignmentId, stored, now);
    submission.setStatus(onTime ? SubmissionStatus::Submitted : SubmissionStatus::Late);
    submissions_.save(submission);
    return submission.getStatus();
}

SubmissionStatus SubmissionService::resubmit(int studentId, int assignmentId, const std::filesystem::path& file, DateTime now) {
    now = floorToSeconds(now);
    const Assignment assignment = assignments_.getAssignment(assignmentId);
    requireEnrolled(studentId, assignment);
    std::optional<Submission> existing = findFor(studentId, assignmentId);
    if (!existing) {
        throw ValidationError("You have not submitted this assignment yet.");
    }
    if (existing->getStatus() != SubmissionStatus::Submitted) {
        throw ValidationError("This work can no longer be resubmitted (it is late or already marked).");
    }
    if (now > assignments_.effectiveDeadline(assignmentId, studentId)) {
        throw ValidationError("The deadline has passed, so the work cannot be resubmitted.");
    }
    const std::string stored = storage_.store(file, assignmentId, studentId, now);
    Submission replacement(studentId, assignmentId, stored, now);
    replacement.setSubmissionId(existing->getSubmissionId());  // same row: the new file replaces the old one
    replacement.setStatus(SubmissionStatus::Submitted);
    submissions_.save(replacement);
    return SubmissionStatus::Submitted;
}

double SubmissionService::markSubmission(int lecturerId, int submissionId, double rawMark, const std::string& feedback,
                                         DateTime now) {
    now = floorToSeconds(now);
    Submission submission = getSubmission(submissionId);
    const std::optional<int> teacher = submissions_.lecturerOf(submissionId);
    if (!teacher || *teacher != lecturerId) {
        throw AccessDeniedError("You are not the lecturer of this course.");
    }
    if (submission.getStatus() != SubmissionStatus::Submitted && submission.getStatus() != SubmissionStatus::Late) {
        throw ValidationError("This submission has already been marked.");
    }
    const Assignment assignment = assignments_.getAssignment(submission.getAssignmentId());
    if (rawMark < 0.0 || rawMark > assignment.getMaxMark()) {
        throw ValidationError("The mark must be between 0 and " + std::to_string(assignment.getMaxMark()) + ".");
    }

    double finalMark = rawMark;
    std::string reason = "Initial marking";
    if (submission.getStatus() == SubmissionStatus::Late) {
        const DateTime deadline = assignments_.effectiveDeadline(submission.getAssignmentId(), submission.getStudentId());
        const double hoursLate = hoursBetween(submission.getSubmittedAt(), deadline);
        const double deduction = penalty_.calculatePenalty(hoursLate, assignment.getMaxMark());
        finalMark = std::max(0.0, rawMark - deduction);
        reason += " (late penalty applied)";
    }

    submission.setMark(finalMark);
    submission.setFeedback(feedback);
    submission.setStatus(SubmissionStatus::Marked);
    submissions_.save(submission);
    auditLog_.save(AuditLogEntry(0, submissionId, lecturerId, std::nullopt, finalMark, now, reason));
    return finalMark;
}

Submission SubmissionService::getSubmission(int submissionId) {
    std::optional<Submission> submission = submissions_.findById(submissionId);
    if (!submission) {
        throw ValidationError("No submission with id " + std::to_string(submissionId) + ".");
    }
    return *submission;
}

std::vector<Submission> SubmissionService::submissionsForStudent(int studentId) {
    return submissions_.findWhere({{"student_id", studentId}});
}

std::vector<Submission> SubmissionService::releasedFor(int studentId) {
    const std::vector<Submission> all = submissionsForStudent(studentId);
    std::vector<Submission> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
                 [](const Submission& submission) { return submission.getStatus() == SubmissionStatus::Released; });
    return result;
}

std::vector<Submission> SubmissionService::awaitingMarking(int lecturerId) {
    const std::vector<Submission> all = submissions_.findForLecturer(lecturerId);
    std::vector<Submission> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result), [](const Submission& submission) {
        return submission.getStatus() == SubmissionStatus::Submitted || submission.getStatus() == SubmissionStatus::Late;
    });
    return result;
}

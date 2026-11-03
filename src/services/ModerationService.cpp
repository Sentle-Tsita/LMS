#include "services/ModerationService.hpp"

#include <algorithm>
#include <iterator>

#include "services/Errors.hpp"
#include "util/DateTimeUtil.hpp"
#include "util/StringUtil.hpp"

namespace {

std::vector<Submission> withStatus(const std::vector<Submission>& all, SubmissionStatus status) {
    std::vector<Submission> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
                 [status](const Submission& submission) { return submission.getStatus() == status; });
    return result;
}

}  // namespace

Submission ModerationService::loadForHod(int hodUserId, int submissionId) {
    std::optional<Submission> submission = submissions_.findById(submissionId);
    if (!submission) {
        throw ValidationError("No submission with id " + std::to_string(submissionId) + ".");
    }
    const std::optional<int> head = submissions_.headOf(submissionId);
    if (!head || *head != hodUserId) {
        throw AccessDeniedError("This submission is not in your department.");
    }
    return *submission;
}

std::vector<Submission> ModerationService::getMarkedForDepartment(int hodUserId) {
    return withStatus(submissions_.findForHod(hodUserId), SubmissionStatus::Marked);
}

std::vector<Submission> ModerationService::getModeratedForDepartment(int hodUserId) {
    return withStatus(submissions_.findForHod(hodUserId), SubmissionStatus::Moderated);
}

void ModerationService::changeMark(int hodUserId, int submissionId, double newMark, const std::string& reason, DateTime now) {
    Submission submission = loadForHod(hodUserId, submissionId);
    const std::string cleanReason = trim(reason);
    if (cleanReason.empty()) {
        throw ValidationError("A reason is required to change a mark.");
    }
    if (submission.getStatus() != SubmissionStatus::Marked) {
        throw ValidationError("Only marks waiting for review can be changed.");
    }
    const std::optional<Assignment> assignment = assignments_.findById(submission.getAssignmentId());
    if (!assignment || newMark < 0.0 || newMark > assignment->getMaxMark()) {
        throw ValidationError("The mark must be between 0 and the assignment's maximum mark.");
    }
    const std::optional<double> oldMark = submission.getMark();
    submission.setMark(newMark);
    submissions_.save(submission);
    auditLog_.save(AuditLogEntry(0, submissionId, hodUserId, oldMark, newMark, floorToSeconds(now), cleanReason));
}

void ModerationService::approveMark(int hodUserId, int submissionId) {
    Submission submission = loadForHod(hodUserId, submissionId);
    if (submission.getStatus() != SubmissionStatus::Marked) {
        throw ValidationError("Only marks waiting for review can be approved.");
    }
    submission.setStatus(SubmissionStatus::Moderated);
    submissions_.save(submission);
}

int ModerationService::releaseMarks(int hodUserId, const std::vector<int>& submissionIds) {
    std::vector<Submission> toRelease;
    for (int id : submissionIds) {  // check everything first, so a bad id releases nothing
        Submission submission = loadForHod(hodUserId, id);
        if (submission.getStatus() != SubmissionStatus::Moderated) {
            throw ValidationError("Submission " + std::to_string(id) + " has not been approved yet.");
        }
        toRelease.push_back(submission);
    }
    for (Submission& submission : toRelease) {
        submission.setStatus(SubmissionStatus::Released);
        submissions_.save(submission);
    }
    return static_cast<int>(toRelease.size());
}

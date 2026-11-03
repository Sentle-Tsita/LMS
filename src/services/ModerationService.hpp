#pragma once
#include <chrono>
#include <string>
#include <vector>

#include "data/RepositoryTypes.hpp"
#include "data/SubmissionRepository.hpp"
#include "domain/Submission.hpp"

// The Head of Department reviews, changes, approves and releases marks (FR-13 to FR-15, FR-17).
// Students see nothing until the HoD releases (the status moves MARKED -> MODERATED -> RELEASED).
class ModerationService {
public:
    ModerationService(SubmissionRepository& submissions, AssignmentRepository& assignments, AuditLogRepository& auditLog)
        : submissions_(submissions), assignments_(assignments), auditLog_(auditLog) {}

    // Marked submissions in this HoD's department, waiting for review (FR-13).
    std::vector<Submission> getMarkedForDepartment(int hodUserId);
    // Approved submissions waiting to be released.
    std::vector<Submission> getModeratedForDepartment(int hodUserId);

    // Changes a mark. A reason is required, and the change is logged (FR-14, FR-17).
    void changeMark(int hodUserId, int submissionId, double newMark, const std::string& reason,
                    DateTime now = std::chrono::system_clock::now());
    void approveMark(int hodUserId, int submissionId);
    // Releases approved marks and feedback to students (FR-15). Returns how many were released.
    int releaseMarks(int hodUserId, const std::vector<int>& submissionIds);

private:
    Submission loadForHod(int hodUserId, int submissionId);

    SubmissionRepository& submissions_;
    AssignmentRepository& assignments_;
    AuditLogRepository& auditLog_;
};

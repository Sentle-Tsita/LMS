#pragma once
#include <string>
#include <utility>
#include "DateTime.hpp"
#include <optional>

class AuditLogEntry{
    private:
        int entryId;
        int submissionId;
        int changedBy;
        std::optional<double> oldMark;
        double newMark;
        DateTime changedAt;
        std::string reason;
    public:
        AuditLogEntry(int eid, int sid, int cb, std::optional<double> om, double nm, DateTime ca, std::string r)
            : entryId(eid),
            submissionId(sid),
            changedBy(cb),
            oldMark(om),
            newMark(nm),
            changedAt(ca),
            reason(std::move(r)) {}

        int getEntryId() const { return entryId; }
        int getSubmissionId() const { return submissionId; }
        int getChangedBy() const { return changedBy; }
        std::optional<double> getOldMark() const { return oldMark; }
        double getNewMark() const { return newMark; }
        DateTime getChangedAt() const { return changedAt; }
        std::string getReason() const { return reason; }
};
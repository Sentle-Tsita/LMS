#pragma once
#include <optional>
#include <string>
#include <utility>
#include "DateTime.hpp"
#include "SubmissionStatus.hpp"

class Submission {
    private:
        int submissionId = 0;
        int studentId;
        int assignmentId;
        std::string filePath;
        DateTime submittedAt;
        SubmissionStatus status = SubmissionStatus::Submitted;
        std::optional<double> mark;
        std::string feedback;

    public:
        Submission(int sid, int aid, std::string fp, DateTime sa)
            : studentId(sid),
            assignmentId(aid),
            filePath(move(fp)),
            submittedAt(sa) {}

        int getSubmissionId() const { return submissionId; }
        void setSubmissionId(int id) { submissionId = id; }

        int getStudentId() const { return studentId; }
        int getAssignmentId() const { return assignmentId; }
        const std::string& getFilePath() const { return filePath; }
        DateTime getSubmittedAt() const { return submittedAt; }

        SubmissionStatus getStatus() const { return status; }
        void setStatus(SubmissionStatus st) { status = st; }

        std::optional<double> getMark() const { return mark; }
        void setMark(double m) { mark = m; }

        const std::string& getFeedback() const { return feedback; }
        void setFeedback(std::string fb) { feedback = move(fb); }
};
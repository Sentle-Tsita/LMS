#include <iostream>
#include <optional>
#include <string>
#include <chrono>

using namespace std;

// 1. Domain: users

// 2. Domain: academic data

using DateTime = chrono::system_clock::time_point;

enum class SubmissionStatus {
    Submitted,
    Late,
    Marked,
    Moderated,
    Released,
    UnderReview
};

class Submission {
    private:
        int submissionId = 0;
        int studentId;
        int assignmentId;
        string filePath;
        DateTime submittedAt;
        SubmissionStatus status = SubmissionStatus::Submitted;
        optional<double> mark;
        string feedback;

    public:
        Submission(int sid, int aid, string fp, DateTime sa)
            : studentId(sid),
            assignmentId(aid),
            filePath(move(fp)),
            submittedAt(sa) {}

        int getSubmissionId() const { return submissionId; }
        void setSubmissionId(int id) { submissionId = id; }

        int getStudentId() const { return studentId; }
        int getAssignmentId() const { return assignmentId; }
        const string& getFilePath() const { return filePath; }
        DateTime getSubmittedAt() const { return submittedAt; }

        SubmissionStatus getStatus() const { return status; }
        void setStatus(SubmissionStatus st) { status = st; }

        optional<double> getMark() const { return mark; }
        void setMark(double m) { mark = m; }

        const string& getFeedback() const { return feedback; }
        void setFeedback(string fb) { feedback = move(fb); }
};

// 3. Late penalty (Strategy)

const double HoursPerDay = 24.0;
const double MaxLateHours = 72.0;
const double PenaltyPerDay = 0.1;

class LatePenaltyStrategy{
    public:
        virtual ~LatePenaltyStrategy() = default;
        virtual double calculatePenalty(double hoursLate, double maxMark) const = 0;
};

class StandardLatePenalty: public LatePenaltyStrategy{
    public:
        double calculatePenalty(double hoursLate, double maxMark) const override{
            if(hoursLate <= 0.0){
                return 0.0;
            }
            if (hoursLate > MaxLateHours){
                return maxMark;
            }
            int count = 0;
            double hoursLeft = hoursLate; 
            while(hoursLeft > 0.0){
                count ++;
                hoursLeft -= HoursPerDay;
            }
            return maxMark*count*PenaltyPerDay;
        }
};
// 4. Data access (Repository)

// 5. Services

// 6. Console interface

#ifndef LMS_TESTING
int main() {
    cout << "LMS v0.1\n";

    StandardLatePenalty penalty;
    double hours[] = {0, 1, 24, 24.01, 25, 72, 73};

    for (double h : hours) {
        cout << h << " hours late, max mark 20: deduct "
             << penalty.calculatePenalty(h, 20) << "\n";
    }

    Submission submission(1, 101, "assignment.pdf",
                     chrono::system_clock::now());

    cout << "Mark is empty: " << submission.getMark().has_value() << "\n";

    submission.setMark(15.5);
    cout << "Mark: " << *submission.getMark() << "\n";

    cout << "Status: " 
        << static_cast<int>(submission.getStatus()) << "\n";
    return 0;
}
#endif
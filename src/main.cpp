#include <iostream>
#include <optional>
#include <string>
#include <chrono>
#include <utility>
#include <vector>
#include <memory>
#include "domain/User.hpp"
#include "domain/Student.hpp"
#include "domain/Lecturer.hpp"
#include "domain/HeadOfDepartment.hpp"
#include "domain/Administrator.hpp"

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

class Department {
    private:
        int departmentId;
        string name;
        string hodUserId;
    public:
        Department(int did, string n, string hid)
            : departmentId(did),
            name(move(n)),
            hodUserId(move(hid)) {}

        int getDepartmentId() const { return departmentId; }
        string getName() const { return name; }
        string getHodUserId() const { return hodUserId; }
        void setName(string newName) { name = newName; }
        void setHodUserId(string newHodUserId) { hodUserId = newHodUserId; }

};

class Course{
    private:
        string courseCode;
        string courseName;
        int departmentId;
        int lecturerId;
    public:
        Course(string cc, string cn, int did, int lid)
            : courseCode(move(cc)),
            courseName(move(cn)),
            departmentId(did),
            lecturerId(lid) {}

        string getCourseCode() const { return courseCode; }
        string getCourseName() const { return courseName; }
        int getDepartmentId() const { return departmentId;}
        int getLecturerId() const { return lecturerId; }
        void setCourseName(string newCourseName) { courseName = newCourseName; }
        void setDepartmentId(int newDepartmentId) { departmentId = newDepartmentId; }
        void setLecturerId(int newLecturerId) { lecturerId = newLecturerId; }
};

class Assignment{
    private:
        int assignmentId;
        string courseCode;
        string title;
        DateTime deadline;
        double maxMark;
    public:
        Assignment(int aid, string cc, string t, DateTime d, double mm)
            : assignmentId(aid),
            courseCode(move(cc)),
            title(move(t)),
            deadline(d),
            maxMark(mm) {}

        int getAssignmentId() const { return assignmentId;}
        string getCourseCode() const { return courseCode;}
        string getTitle() const { return title; }
        DateTime getDeadline() const { return deadline;}
        double getMaxMark() const {return maxMark;}
};

class Extension{
    private:
        int extensionId;
        int assignmentId;
        int studentId;
        DateTime newDeadline;
    public:
        Extension(int eid, int aid, int sid, DateTime nd)
            : extensionId(eid),
            assignmentId(aid),
            studentId(sid),
            newDeadline(nd) {}

        int getExtensionId() { return extensionId; }
        int getAssignmentId() { return assignmentId;}
        int getStudentId() { return studentId; }

        DateTime getNewDeadline() { return newDeadline; }
};

class AuditLogEntry{
    private:
        int entryId;
        int submissionId;
        int changedBy;
        double oldMark;
        double newMark;
        DateTime changedAt;
        string reason;
    public:
        AuditLogEntry(int eid, int sid, int cb, double om, double nm, DateTime ca, string r)
            : entryId(eid),
            submissionId(sid),
            changedBy(cb),
            oldMark(om),
            newMark(nm),
            changedAt(ca),
            reason(move(r)) {}

        int getEntryId() const { return entryId; }
        int getSubmissionId() const { return submissionId; }
        int getChangedBy() const { return changedBy; }
        double getOldMark() const { return oldMark; }
        double getNewMark() const { return newMark; }
        DateTime getChangedAt() { return changedAt; }
        string getReason() { return reason; }
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

    cout << "Has mark: " << submission.getMark().has_value() << "\n";

    submission.setMark(15.5);
    cout << "Mark: " << *submission.getMark() << "\n";

    cout << "Status: " 
        << static_cast<int>(submission.getStatus()) << "\n";

    vector<unique_ptr<User>> users;
    users.push_back(make_unique<Student>(1, "s202322646", "hash", "salt"));
    users.push_back(make_unique<Lecturer>(1, "l202323232", "hash", "salt"));
    users.push_back(make_unique<HeadOfDepartment>(1, "h202322323", "hash", "salt"));
    users.push_back(make_unique<Administrator>(1, "a202322222", "hash", "salt"));

    for (const auto& u : users) {
        u->showMenu();
    }

    Assignment assignment(1, "CS4433", "Essay", chrono::system_clock::now() - chrono::hours(25), 20);
    double hoursLate = chrono::duration<double, ratio<3600>>(
        submission.getSubmittedAt() - assignment.getDeadline()).count();
    cout << "Hours late: " << hoursLate << "\n";
    cout << "Deduct: " << penalty.calculatePenalty(hoursLate, assignment.getMaxMark()) << "\n";

    return 0;
}
#endif
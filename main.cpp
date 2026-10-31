#include <iostream>
#include <optional>
#include <string>
#include <chrono>
#include <utility>
#include <vector>
#include <memory>

using namespace std;

// 1. Domain: users
class User {
    private:
        int userId;
        string username;
        string passwordHash;
        string salt;
        bool active = true;
    public:
        User(int id, string un, string ph, string sl, bool ac = true)
            : userId(id),
            username(move(un)),
            passwordHash(move(ph)),
            salt(move(sl)),
            active(ac) {}

        virtual ~User() = default;

        virtual void showMenu() const = 0;

        int getUserId() const { return userId; }
        const string& getUsername() const { return username; }
        const string& getPasswordHash() const { return passwordHash; }
        const string& getSalt() const { return salt; }
        bool isActive() const { return active; }
        void setActive(bool ac) { active = ac; }
};

class Student : public User {
public: using User::User;
    void showMenu() const override{
        cout << "Student menu\n"
         << "1. View my courses and assignments\n"
         << "2. Submit assignment\n"
         << "3. View released marks and feedback\n"
         << "0. Log out\n";
    }
};

class Lecturer : public User {
public: using User::User;
    void showMenu() const override{
        cout << "Lecturer menu\n"
         << "1. Create assignment\n"
         << "2. Grant extension\n"
         << "3. Mark submissions and write feedback\n"
         << "0. Log out\n";
    }
};

class HeadOfDepartment : public User {
public: using User::User;
    void showMenu() const override{
        cout << "HOD menu\n"
         << "1. Review and moderate marks\n"
         << "2. Release approved marks\n"
         << "0. Log out\n";
    }
};

class Administrator : public User {
public: using User::User;
    void showMenu() const override{
        cout << "Administrator menu\n"
         << "1. Manage Account(s)\n"
         << "2. Create Department\n"
         << "3. Create Course\n"
         << "4. Manage Course(s)\n"
         << "0. Log out\n";
    }
};


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

    return 0;
}
#endif
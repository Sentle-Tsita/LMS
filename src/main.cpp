#include <iostream>
#include <vector>
#include <memory>
#include "domain/User.hpp"
#include "domain/Student.hpp"
#include "domain/Lecturer.hpp"
#include "domain/HeadOfDepartment.hpp"
#include "domain/Administrator.hpp"
#include "domain/Submission.hpp"
#include "domain/Department.hpp"
#include "domain/Course.hpp"
#include "domain/Assignment.hpp"
#include "domain/Extension.hpp"
#include "domain/AuditLogEntry.hpp"

using namespace std;

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
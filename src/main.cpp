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
#include "domain/StandardLatePenalty.hpp"

using namespace std;

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
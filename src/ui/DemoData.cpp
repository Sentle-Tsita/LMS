#include "ui/DemoData.hpp"

#include <chrono>

void seedDemoData(AccountService& accounts, AssignmentService& assignments) {
    const std::string password = "Password123";
    accounts.createUser("ADMINISTRATOR", "admin", password);
    const int hod = accounts.createUser("HOD", "hod.cs", password);
    const int lecturer = accounts.createUser("LECTURER", "lecturer1", password);
    const int department = accounts.createDepartment("Computer Science", hod);
    accounts.saveCourse(Course("CS4433", "Software Engineering", department, lecturer));
    for (const char* name : {"student1", "student2", "student3"}) {
        accounts.enrolStudent(accounts.createUser("STUDENT", name, password), "CS4433");
    }
    assignments.createAssignment(lecturer, "CS4433", "Essay on design patterns",
                                 std::chrono::system_clock::now() + std::chrono::hours(24 * 7), 20.0);
}

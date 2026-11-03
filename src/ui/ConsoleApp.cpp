#include "ui/ConsoleApp.hpp"

#include "util/DateTimeUtil.hpp"
#include "util/StringUtil.hpp"

std::string ConsoleApp::readLine(const std::string& prompt) {
    out_ << prompt << std::flush;
    std::string line;
    if (!std::getline(in_, line)) {
        throw InputEnded{};
    }
    return trim(line);
}

int ConsoleApp::readInt(const std::string& prompt) {
    for (;;) {
        const std::string text = readLine(prompt);
        try {
            std::size_t used = 0;
            const int value = std::stoi(text, &used);
            if (used == text.size()) {
                return value;
            }
        } catch (const std::exception&) {
        }
        out_ << "Please enter a whole number.\n";
    }
}

double ConsoleApp::readDouble(const std::string& prompt) {
    for (;;) {
        const std::string text = readLine(prompt);
        try {
            std::size_t used = 0;
            const double value = std::stod(text, &used);
            if (used == text.size()) {
                return value;
            }
        } catch (const std::exception&) {
        }
        out_ << "Please enter a number.\n";
    }
}

DateTime ConsoleApp::readDate(const std::string& prompt) {
    for (;;) {
        try {
            return fromIsoString(readLine(prompt + " (UTC, YYYY-MM-DD HH:MM): "));
        } catch (const std::invalid_argument& error) {
            out_ << error.what() << "\n";
        }
    }
}

std::optional<int> ConsoleApp::readOptionalInt(const std::string& prompt) {
    for (;;) {
        const std::string text = readLine(prompt);
        if (text.empty()) {
            return std::nullopt;
        }
        try {
            std::size_t used = 0;
            const int value = std::stoi(text, &used);
            if (used == text.size()) {
                return value;
            }
        } catch (const std::exception&) {
        }
        out_ << "Please enter a whole number, or leave it blank.\n";
    }
}

void ConsoleApp::run() {
    try {
        out_ << "Moderated Learning Management System\n";
        if (!accounts_.hasUsers()) {
            firstRunSetup();
        }
        for (;;) {
            out_ << "\nLog in (leave the username blank to quit)\n";
            const std::string username = readLine("Username: ");
            if (username.empty()) {
                return;
            }
            const std::string password = readLine("Password: ");
            std::unique_ptr<User> user;
            try {
                user = accounts_.login(username, password);
            } catch (const std::exception& error) {
                out_ << "Error: " << error.what() << "\n";
                continue;
            }
            if (!user) {
                out_ << "Wrong username or password, or the account is deactivated.\n";
                continue;
            }
            out_ << "Welcome, " << user->getUsername() << ".\n";
            session(*user);
        }
    } catch (const InputEnded&) {
        out_ << "\n";
    }
}

void ConsoleApp::firstRunSetup() {
    out_ << "\nNo accounts exist yet. Create the first administrator account.\n";
    for (;;) {
        const std::string username = readLine("Username: ");
        const std::string password = readLine("Password (at least 8 characters): ");
        try {
            accounts_.createUser("ADMINISTRATOR", username, password);
            out_ << "Administrator account created.\n";
            return;
        } catch (const std::exception& error) {
            out_ << "Error: " << error.what() << "\n";
        }
    }
}

void ConsoleApp::session(const User& user) {
    for (;;) {
        user.showMenu(out_);
        const int choice = readInt("Choose an option: ");
        if (choice == 0) {
            out_ << "Logged out.\n";
            return;
        }
        try {
            dispatch(user, choice);
        } catch (const InputEnded&) {
            throw;
        } catch (const std::exception& error) {  // any failure is shown, and the menu carries on (NFR-06)
            out_ << "Error: " << error.what() << "\n";
        }
    }
}

void ConsoleApp::dispatch(const User& user, int choice) {
    const std::string role = user.roleName();
    if (role == "STUDENT") {
        studentAction(user, choice);
    } else if (role == "LECTURER") {
        lecturerAction(user, choice);
    } else if (role == "HOD") {
        hodAction(user, choice);
    } else {
        adminAction(choice);
    }
}

void ConsoleApp::showSubmissionLine(const Submission& submission) {
    out_ << "  Submission " << submission.getSubmissionId() << ": student " << submission.getStudentId()
         << ", assignment " << submission.getAssignmentId() << ", " << toString(submission.getStatus())
         << ", submitted " << toIsoString(submission.getSubmittedAt());
    if (submission.getMark()) {
        out_ << ", mark " << *submission.getMark();
    }
    out_ << "\n";
}

// ---------------------------------------------------------------- student
void ConsoleApp::studentAction(const User& user, int choice) {
    const int id = user.getUserId();
    if (choice == 1) {
        const std::vector<Course> courses = assignments_.coursesForStudent(id);
        if (courses.empty()) {
            out_ << "You are not enrolled in any course.\n";
        }
        for (const Course& course : courses) {
            out_ << course.getCourseCode() << " - " << course.getCourseName() << "\n";
        }
        for (const Assignment& assignment : assignments_.assignmentsForStudent(id)) {
            out_ << "  Assignment " << assignment.getAssignmentId() << " (" << assignment.getCourseCode() << "): "
                 << assignment.getTitle() << ", max mark " << assignment.getMaxMark() << ", deadline "
                 << toIsoString(assignments_.effectiveDeadline(assignment.getAssignmentId(), id)) << " UTC";
            if (const auto mine = submissions_.findFor(id, assignment.getAssignmentId())) {
                out_ << " [" << toString(mine->getStatus()) << "]";
            }
            out_ << "\n";
        }
    } else if (choice == 2) {
        const int assignmentId = readInt("Assignment id: ");
        const std::string file = readLine("Path to your file (.docx, .pdf or source code, up to 5 MB): ");
        const bool again = submissions_.findFor(id, assignmentId).has_value();
        const SubmissionStatus status = again ? submissions_.resubmit(id, assignmentId, file)
                                              : submissions_.submit(id, assignmentId, file);
        if (status == SubmissionStatus::Late) {
            out_ << "Submitted LATE. The work is accepted, and a late penalty will be applied when it is marked.\n";
        } else {
            out_ << (again ? "Resubmitted. The new file replaced your earlier one.\n" : "Submitted on time.\n");
        }
    } else if (choice == 3) {
        bool any = false;
        for (const Submission& submission : submissions_.submissionsForStudent(id)) {
            const Assignment assignment = assignments_.getAssignment(submission.getAssignmentId());
            out_ << assignment.getCourseCode() << " - " << assignment.getTitle() << ": ";
            if (submission.getStatus() == SubmissionStatus::Released) {
                out_ << *submission.getMark() << " / " << assignment.getMaxMark() << "\n    Feedback: "
                     << (submission.getFeedback().empty() ? "(none)" : submission.getFeedback()) << "\n";
            } else if (submission.getStatus() == SubmissionStatus::UnderReview) {
                out_ << "Mark under review\n";
            } else {
                out_ << "mark not released yet\n";
            }
            any = true;
        }
        if (!any) {
            out_ << "You have not submitted any work.\n";
        }
    } else {
        out_ << "Unknown option.\n";
    }
}

// --------------------------------------------------------------- lecturer
void ConsoleApp::lecturerAction(const User& user, int choice) {
    const int id = user.getUserId();
    if (choice == 1) {
        for (const Course& course : assignments_.coursesForLecturer(id)) {
            out_ << "  " << course.getCourseCode() << " - " << course.getCourseName() << "\n";
        }
        const std::string code = readLine("Course code: ");
        const std::string title = readLine("Title: ");
        const DateTime deadline = readDate("Deadline");
        const double maxMark = readDouble("Maximum mark: ");
        const int created = assignments_.createAssignment(id, code, title, deadline, maxMark);
        out_ << "Assignment " << created << " created.\n";
    } else if (choice == 2) {
        for (const Assignment& assignment : assignments_.assignmentsForLecturer(id)) {
            out_ << "  Assignment " << assignment.getAssignmentId() << ": " << assignment.getTitle() << " ("
                 << assignment.getCourseCode() << "), deadline " << toIsoString(assignment.getDeadline()) << " UTC\n";
        }
        const int assignmentId = readInt("Assignment id: ");
        const int studentId = readInt("Student id: ");
        const DateTime newDeadline = readDate("New deadline for this student");
        assignments_.grantExtension(id, assignmentId, studentId, newDeadline);
        out_ << "Extension granted to student " << studentId << " only.\n";
    } else if (choice == 3) {
        const std::vector<Submission> waiting = submissions_.awaitingMarking(id);
        if (waiting.empty()) {
            out_ << "No submissions are waiting to be marked.\n";
            return;
        }
        for (const Submission& submission : waiting) {
            showSubmissionLine(submission);
        }
        const int submissionId = readInt("Submission id to mark (0 to go back): ");
        if (submissionId == 0) {
            return;
        }
        const double rawMark = readDouble("Mark before any late penalty: ");
        const std::string feedback = readLine("Written feedback: ");
        const double finalMark = submissions_.markSubmission(id, submissionId, rawMark, feedback);
        out_ << "Saved. Final mark: " << finalMark << ". It goes to the Head of Department for review.\n";
    } else {
        out_ << "Unknown option.\n";
    }
}

// ------------------------------------------------------------------- HoD
void ConsoleApp::hodAction(const User& user, int choice) {
    const int id = user.getUserId();
    if (choice == 1) {
        const std::vector<Submission> marked = moderation_.getMarkedForDepartment(id);
        if (marked.empty()) {
            out_ << "No marks are waiting for review.\n";
            return;
        }
        for (const Submission& submission : marked) {
            showSubmissionLine(submission);
        }
        const int submissionId = readInt("Submission id to review (0 to go back): ");
        if (submissionId == 0) {
            return;
        }
        for (;;) {
            const int action = readInt("1. Approve  2. Change mark  0. Back: ");
            if (action == 0) {
                return;
            }
            try {  // an error (for example a missing reason) lets the HoD try again
                if (action == 1) {
                    moderation_.approveMark(id, submissionId);
                    out_ << "Approved. It can now be released.\n";
                    return;
                }
                if (action == 2) {
                    const double newMark = readDouble("New mark: ");
                    const std::string reason = readLine("Reason for the change: ");
                    moderation_.changeMark(id, submissionId, newMark, reason);
                    out_ << "Mark changed and logged.\n";
                } else {
                    out_ << "Unknown option.\n";
                }
            } catch (const InputEnded&) {
                throw;
            } catch (const std::exception& error) {
                out_ << "Error: " << error.what() << "\n";
            }
        }
    } else if (choice == 2) {
        const std::vector<Submission> approved = moderation_.getModeratedForDepartment(id);
        if (approved.empty()) {
            out_ << "No approved marks are waiting for release.\n";
            return;
        }
        std::vector<int> ids;
        for (const Submission& submission : approved) {
            showSubmissionLine(submission);
            ids.push_back(submission.getSubmissionId());
        }
        if (readLine("Release all of these to students? (y/n): ") == "y") {
            out_ << moderation_.releaseMarks(id, ids) << " mark(s) released.\n";
        }
    } else {
        out_ << "Unknown option.\n";
    }
}

// ----------------------------------------------------------------- admin
void ConsoleApp::adminAction(int choice) {
    if (choice == 1) {
        const int action = readInt("1. Create account  2. Deactivate account  3. List accounts: ");
        if (action == 1) {
            const std::string role = readLine("Role (STUDENT, LECTURER, HOD, ADMINISTRATOR): ");
            const std::string username = readLine("Username: ");
            const std::string password = readLine("Password (at least 8 characters): ");
            out_ << "Account created with id " << accounts_.createUser(role, username, password) << ".\n";
        } else if (action == 2) {
            accounts_.deactivateUser(readInt("User id: "));
            out_ << "Account deactivated.\n";
        } else if (action == 3) {
            for (const auto& user : accounts_.listUsers()) {
                out_ << "  " << user->getUserId() << "  " << user->getUsername() << "  " << user->roleName()
                     << (user->isActive() ? "" : "  (deactivated)") << "\n";
            }
        }
    } else if (choice == 2) {
        const std::string name = readLine("Department name: ");
        const std::optional<int> hod = readOptionalInt("Head of department user id (blank for none yet): ");
        out_ << "Department created with id " << accounts_.createDepartment(name, hod) << ".\n";
    } else if (choice == 3) {
        for (const Department& department : accounts_.listDepartments()) {
            out_ << "  Department " << department.getDepartmentId() << ": " << department.getName() << "\n";
        }
        const std::string code = readLine("Course code: ");
        const std::string name = readLine("Course name: ");
        const int department = readInt("Department id: ");
        const std::optional<int> lecturer = readOptionalInt("Lecturer user id (blank for none yet): ");
        accounts_.saveCourse(Course(code, name, department, lecturer));
        out_ << "Course saved.\n";
    } else if (choice == 4) {
        const int action = readInt("1. Assign lecturer  2. Enrol student: ");
        if (action == 1) {
            const std::string code = readLine("Course code: ");
            accounts_.assignLecturer(code, readInt("Lecturer user id: "));
            out_ << "Lecturer assigned.\n";
        } else if (action == 2) {
            const int student = readInt("Student user id: ");
            accounts_.enrolStudent(student, readLine("Course code: "));
            out_ << "Student enrolled.\n";
        }
    } else {
        out_ << "Unknown option.\n";
    }
}

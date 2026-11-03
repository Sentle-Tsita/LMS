#pragma once
#include <iostream>
#include <optional>
#include <string>

#include "domain/User.hpp"
#include "services/AccountService.hpp"
#include "services/AssignmentService.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"

// Console menus for the four roles. Input and output streams are injected, so the whole
// interface can be driven by a script in system tests.
class ConsoleApp {
public:
    ConsoleApp(AccountService& accounts, AssignmentService& assignments, SubmissionService& submissions,
               ModerationService& moderation, std::istream& in, std::ostream& out)
        : accounts_(accounts), assignments_(assignments), submissions_(submissions), moderation_(moderation),
          in_(in), out_(out) {}

    void run();

private:
    struct InputEnded {};  // thrown when the input stream closes

    // input helpers
    std::string readLine(const std::string& prompt);
    int readInt(const std::string& prompt);
    double readDouble(const std::string& prompt);
    DateTime readDate(const std::string& prompt);
    std::optional<int> readOptionalInt(const std::string& prompt);

    void firstRunSetup();
    void session(const User& user);
    void dispatch(const User& user, int choice);

    void studentAction(const User& user, int choice);
    void lecturerAction(const User& user, int choice);
    void hodAction(const User& user, int choice);
    void adminAction(int choice);

    void showSubmissionLine(const Submission& submission);

    AccountService& accounts_;
    AssignmentService& assignments_;
    SubmissionService& submissions_;
    ModerationService& moderation_;
    std::istream& in_;
    std::ostream& out_;
};

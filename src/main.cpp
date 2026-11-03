#include <filesystem>
#include <iostream>
#include <string>

#include "data/CourseRepository.hpp"
#include "data/Database.hpp"
#include "data/Mappers.hpp"
#include "data/SubmissionRepository.hpp"
#include "domain/StandardLatePenalty.hpp"
#include "security/PasswordHasher.hpp"
#include "services/AccountService.hpp"
#include "services/AssignmentService.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"
#include "storage/FileStorage.hpp"
#include "ui/ConsoleApp.hpp"
#include "ui/DemoData.hpp"

// Wires the layers together: database -> repositories -> services -> console menus.
int main(int argc, char** argv) {
    try {
        const bool demo = argc > 1 && std::string(argv[1]) == "--demo";

        Database db("lms.db");
        // Use sql/schema.sql next to the program if it is there, otherwise the build-time location.
        const std::string schema = std::filesystem::exists("sql/schema.sql") ? "sql/schema.sql" : LMS_SCHEMA_PATH;
        db.initialiseSchema(schema);

        UserMapper userMapper;
        DepartmentMapper departmentMapper;
        CourseMapper courseMapper;
        AssignmentMapper assignmentMapper;
        ExtensionMapper extensionMapper;
        SubmissionMapper submissionMapper;
        AuditLogMapper auditMapper;

        SqliteRepository<std::unique_ptr<User>, int> users(db, userMapper);
        SqliteRepository<Department, int> departments(db, departmentMapper);
        CourseRepository courses(db, courseMapper);
        SqliteRepository<Assignment, int> assignmentRepo(db, assignmentMapper);
        SqliteRepository<Extension, int> extensions(db, extensionMapper);
        SubmissionRepository submissions(db, submissionMapper);
        SqliteRepository<AuditLogEntry, int> auditLog(db, auditMapper);

        PasswordHasher hasher;
        StandardLatePenalty penalty;
        FileStorage storage("submissions");

        AccountService accounts(users, departments, courses, hasher);
        AssignmentService assignments(assignmentRepo, extensions, courses);
        SubmissionService submissionService(submissions, assignments, courses, auditLog, penalty, storage);
        ModerationService moderation(submissions, assignmentRepo, auditLog);

        if (demo && !accounts.hasUsers()) {
            seedDemoData(accounts, assignments);
            std::cout << "Demo data created. All demo accounts use the password Password123.\n";
        }

        ConsoleApp app(accounts, assignments, submissionService, moderation, std::cin, std::cout);
        app.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}

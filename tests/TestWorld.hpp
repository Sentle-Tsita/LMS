#pragma once
// A complete in-memory system (database, repositories, services) for tests.
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "data/CourseRepository.hpp"
#include "data/Database.hpp"
#include "data/Mappers.hpp"
#include "data/SubmissionRepository.hpp"
#include "domain/StandardLatePenalty.hpp"
#include "security/PasswordHasher.hpp"
#include "services/AccountService.hpp"
#include "services/AssignmentService.hpp"
#include "services/Errors.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"
#include "storage/FileStorage.hpp"
#include "util/DateTimeUtil.hpp"

inline std::filesystem::path makeTempDir() {
    static std::atomic<int> counter{0};
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("lms_test_" + std::to_string(ticks) + "_" + std::to_string(counter++));
    std::filesystem::create_directories(dir);
    return dir;
}

// Ids created by seedStandard().
struct Seed {
    int hodCs = 0, hodMath = 0, lectCs = 0, lectMath = 0, student1 = 0, student2 = 0, student3 = 0;
    int deptCs = 0, deptMath = 0;
    int assignment = 0;  // CS101, taught by lectCs, headed by hodCs
    DateTime deadline;
};

struct TestWorld {
    Database db;
    UserMapper userMapper;
    DepartmentMapper departmentMapper;
    CourseMapper courseMapper;
    AssignmentMapper assignmentMapper;
    ExtensionMapper extensionMapper;
    SubmissionMapper submissionMapper;
    AuditLogMapper auditMapper;
    SqliteRepository<std::unique_ptr<User>, int> users;
    SqliteRepository<Department, int> departments;
    CourseRepository courses;
    SqliteRepository<Assignment, int> assignmentRepo;
    SqliteRepository<Extension, int> extensions;
    SubmissionRepository submissions;
    SqliteRepository<AuditLogEntry, int> auditLog;
    PasswordHasher hasher;
    StandardLatePenalty penalty;
    std::filesystem::path dir;
    FileStorage storage;
    AccountService accounts;
    AssignmentService assignments;
    SubmissionService submissionService;
    ModerationService moderation;

    TestWorld()
        : db(":memory:"), users(db, userMapper), departments(db, departmentMapper), courses(db, courseMapper),
          assignmentRepo(db, assignmentMapper), extensions(db, extensionMapper), submissions(db, submissionMapper),
          auditLog(db, auditMapper), hasher(1000), dir(makeTempDir()), storage(dir / "store"),
          accounts(users, departments, courses, hasher), assignments(assignmentRepo, extensions, courses),
          submissionService(submissions, assignments, courses, auditLog, penalty, storage),
          moderation(submissions, assignmentRepo, auditLog) {
        db.initialiseSchema(LMS_SCHEMA_PATH);
    }

    ~TestWorld() {
        std::error_code ignored;
        std::filesystem::remove_all(dir, ignored);
    }

    // Two departments, each with a head and a lecturer. CS101 has student1 and student2 enrolled.
    // student3 is enrolled in nothing. One assignment (max mark 20) is due 2026-11-10 12:00:00.
    Seed seedStandard() {
        Seed s;
        s.hodCs = accounts.createUser("HOD", "hod_cs", "Password123");
        s.hodMath = accounts.createUser("HOD", "hod_math", "Password123");
        s.lectCs = accounts.createUser("LECTURER", "lect_cs", "Password123");
        s.lectMath = accounts.createUser("LECTURER", "lect_math", "Password123");
        s.student1 = accounts.createUser("STUDENT", "student1", "Password123");
        s.student2 = accounts.createUser("STUDENT", "student2", "Password123");
        s.student3 = accounts.createUser("STUDENT", "student3", "Password123");
        s.deptCs = accounts.createDepartment("Computer Science", s.hodCs);
        s.deptMath = accounts.createDepartment("Mathematics", s.hodMath);
        accounts.saveCourse(Course("CS101", "Introduction to Programming", s.deptCs, s.lectCs));
        accounts.saveCourse(Course("MA101", "Calculus", s.deptMath, s.lectMath));
        accounts.enrolStudent(s.student1, "CS101");
        accounts.enrolStudent(s.student2, "CS101");
        s.deadline = fromIsoString("2026-11-10 12:00:00");
        s.assignment = assignments.createAssignment(s.lectCs, "CS101", "Essay", s.deadline, 20.0);
        return s;
    }

    std::string makeFile(const std::string& name, std::size_t bytes = 100) {
        const std::filesystem::path path = dir / name;
        std::ofstream file(path, std::ios::binary);
        file << std::string(bytes, 'x');
        return path.string();
    }
};

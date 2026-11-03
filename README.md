# Moderated Learning Management System

A C++17 console program for a school, with an SQLite database. It works offline.

Students submit work, lecturers mark it, and the Head of Department checks the marks and releases
them. Students only see a mark after it is released. Every mark change is logged.

## What you need

- A C++17 compiler (GCC 9 or newer, MinGW-W64 on Windows)
- CMake 3.14 or newer
- On Linux or macOS only: OpenSSL development files (`libssl-dev` on Ubuntu)

SQLite is already included in `third_party/sqlite`.

## Build and run on Windows (PowerShell)

    cmake -S . -B build -G "MinGW Makefiles"
    cmake --build build
    .\build\lms.exe --demo

## Build and run on Linux or macOS

    cmake -S . -B build
    cmake --build build
    ./build/lms --demo

The first build downloads GoogleTest, so it needs internet once. To skip the tests and the
download, add `-DLMS_BUILD_TESTS=OFF` to the first command.

## Logging in

`--demo` adds sample accounts the first time you run it. They all use the password `Password123`:

    admin   hod.cs   lecturer1   student1   student2   student3

Without `--demo`, the first run asks you to create an administrator.

The program makes `lms.db` and a `submissions` folder in the folder you run it from. Run it from
the project folder so it can find `sql/schema.sql`.

## Tests

    .\build\lms_tests.exe

On Linux or macOS use `./build/lms_tests` instead.

## Folders

    src/domain/     the main objects: users, courses, assignments, submissions
    src/data/       the database layer
    src/services/   the rules of the system
    src/security/   password hashing
    src/storage/    checking and saving submitted files
    src/ui/         the console menus and demo data
    sql/            the database schema
    tests/          the automated tests
    docs/           design notes, reviews and test evidence

See also `CODING_STANDARD.md` and `CONTRIBUTING.md`.
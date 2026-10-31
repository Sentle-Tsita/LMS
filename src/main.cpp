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
#include "data/Database.hpp"
#include "data/Statement.hpp"
#include <fstream>
#include <sstream>

using namespace std;

// 5. Services

// 6. Console interface

void initializeDatabase(Database& db) {
    if (db.tableExists("user")) {
        cout << "Database already initialized." << endl;
        return;
    }

    ifstream file(LMS_SCHEMA_PATH);

    if (!file.is_open()) {
        throw DatabaseError(
            "Could not open schema file: " LMS_SCHEMA_PATH
        );
    }

    stringstream buffer;
    buffer << file.rdbuf();

    if (file.bad()) {
        throw DatabaseError("Could not read schema file.");
    }

    db.execute("BEGIN;");

    try {
        db.execute(buffer.str());
        db.execute("COMMIT;");
    }
    catch (...) {
        try {
            db.execute("ROLLBACK;");
        }
        catch (...) {
            // Preserve the original exception.
        }

        throw;
    }

    cout << "Database initialized successfully." << endl;
}

#ifndef LMS_TESTING
int main() {
    cout << "LMS v0.1\n";
     try {
        Database db("lms.db");
        initializeDatabase(db);

        cout << "Database connection established." << endl;
    }
    catch (const DatabaseError& e) {
        cerr << "Database error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
#endif
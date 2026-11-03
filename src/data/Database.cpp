#include "data/Database.hpp"
#include <fstream>
#include <memory>
#include <sstream>
#include "data/Statement.hpp"

Database::Database(const std::string& filePath) {
    int result = sqlite3_open(filePath.c_str(), &db);

    if (result != SQLITE_OK) {
        std::string message;

        if (db != nullptr) {
            message = sqlite3_errmsg(db);
            sqlite3_close_v2(db);
            db = nullptr;
        }
        else {
            message = sqlite3_errstr(result);
        }

        throw DatabaseError(message);
    }

    try {
        execute("PRAGMA foreign_keys = ON;");
    }
    catch (...) {
        sqlite3_close_v2(db);
        db = nullptr;
        throw;
    }
}


Database::~Database() {
    if (db != nullptr) {
        sqlite3_close_v2(db);
    }
}


sqlite3* Database::getConnection() const {
    return db;
}


void Database::execute(const std::string& sql) {
    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {
        std::unique_ptr<char, decltype(&sqlite3_free)> guard(
            errorMessage,
            sqlite3_free
        );

        std::string message;

        if (errorMessage != nullptr) {
            message = errorMessage;
        }
        else {
            message = sqlite3_errmsg(db);
        }

        throw DatabaseError(message);
    }
}


bool Database::tableExists(const std::string& name) {
    Statement statement(
        *this,
        "SELECT 1 FROM sqlite_master "
        "WHERE type = 'table' AND name = ?;"
    );

    statement.bindText(1, name);

    return statement.step();
}


bool Database::initialiseSchema(const std::string& schemaPath) {
    if (tableExists("user")) {
        return false;
    }

    std::ifstream file(schemaPath);
    if (!file.is_open()) {
        throw DatabaseError("Could not open schema file: " + schemaPath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (file.bad()) {
        throw DatabaseError("Could not read schema file: " + schemaPath);
    }

    execute("BEGIN;");
    try {
        execute(buffer.str());
        execute("COMMIT;");
    } catch (...) {
        try {
            execute("ROLLBACK;");
        } catch (...) {
            // Keep the original error.
        }
        throw;
    }
    return true;
}

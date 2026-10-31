#include "data/Database.hpp"
#include <memory>


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
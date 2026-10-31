
#include "data/Statement.hpp"


Statement::Statement(Database& database, const std::string& sql) {
    db = database.getConnection();

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &stmt,
        nullptr
    );

    if (result != SQLITE_OK) {
        std::string message = sqlite3_errmsg(db);

        if (stmt != nullptr) {
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }

        throw DatabaseError(message);
    }
}


Statement::~Statement() {
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
    }
}


void Statement::bindText(int index, const std::string& value) {
    int result = sqlite3_bind_text(
        stmt,
        index,
        value.c_str(),
        static_cast<int>(value.size()),
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK) {
        throw DatabaseError(sqlite3_errmsg(db));
    }
}


void Statement::bindInt(int index, int value) {
    int result = sqlite3_bind_int(stmt, index, value);

    if (result != SQLITE_OK) {
        throw DatabaseError(sqlite3_errmsg(db));
    }
}


void Statement::bindDouble(int index, double value) {
    int result = sqlite3_bind_double(stmt, index, value);

    if (result != SQLITE_OK) {
        throw DatabaseError(sqlite3_errmsg(db));
    }
}


void Statement::bindNull(int index) {
    int result = sqlite3_bind_null(stmt, index);

    if (result != SQLITE_OK) {
        throw DatabaseError(sqlite3_errmsg(db));
    }
}


bool Statement::step() {
    int result = sqlite3_step(stmt);

    if (result == SQLITE_ROW) {
        return true;
    }

    if (result == SQLITE_DONE) {
        return false;
    }

    throw DatabaseError(sqlite3_errmsg(db));
}


bool Statement::isNull(int column) const {
    return sqlite3_column_type(stmt, column) == SQLITE_NULL;
}


std::string Statement::getText(int column) const {
    if (isNull(column)) {
        return "";
    }

    const unsigned char* value = sqlite3_column_text(stmt, column);

    return reinterpret_cast<const char*>(value);
}


int Statement::getInt(int column) const {
    return sqlite3_column_int(stmt, column);
}


double Statement::getDouble(int column) const {
    return sqlite3_column_double(stmt, column);
}

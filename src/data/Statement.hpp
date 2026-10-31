
#pragma once

#include "data/Database.hpp"
#include <string>

class Statement {
private:
    sqlite3_stmt* stmt = nullptr;
    sqlite3* db = nullptr;

public:
    // Prepare the SQL statement.
    Statement(Database& database, const std::string& sql);

    // Finalize the prepared statement.
    ~Statement();

    // Prevent copying.
    Statement(const Statement& other) = delete;
    Statement& operator=(const Statement& other) = delete;

    // Bind values to ? placeholders. Indexes start at 1.
    void bindText(int index, const std::string& value);
    void bindInt(int index, int value);
    void bindDouble(int index, double value);
    void bindNull(int index);

    // Returns true when a row is available, false when finished.
    bool step();

    // Read column values. Column indexes start at 0.
    // Check isNull() before reading nullable columns.
    bool isNull(int column) const;
    std::string getText(int column) const;
    int getInt(int column) const;
    double getDouble(int column) const;
};

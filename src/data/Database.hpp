#pragma once

#include <string>
#include <stdexcept>
#include <sqlite3.h>

class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string& message)
        : std::runtime_error(message) {}
};


class Database {
private:
    sqlite3* db = nullptr;

public:
    // Constructor and destructor
    explicit Database(const std::string& filePath);
    ~Database();

    // Prevent copying
    Database(const Database& other) = delete;
    Database& operator=(const Database& other) = delete;

    // Getter
    sqlite3* getConnection() const;

    // Executes SQL without parameters or result rows.
    // Use prepared statements for SQL containing user data.
    void execute(const std::string& sql);

    bool tableExists(const std::string& name);

    // Creates all tables from the schema file, in one transaction, unless they already exist.
    // Returns true if the schema was created, false if the database was already set up.
    bool initialiseSchema(const std::string& schemaPath);
};
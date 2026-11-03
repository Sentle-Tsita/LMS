#pragma once
#include <string>
#include <vector>
#include "data/Statement.hpp"

// Knows how one entity maps to one table. The SELECT order is always: key column, then columns().
template <typename T, typename K>
class EntityMapper {
public:
    virtual ~EntityMapper() = default;

    virtual std::string tableName() const = 0;
    virtual std::string keyColumn() const = 0;
    virtual std::vector<std::string> columns() const = 0;  // every column except the key
    // true: the database assigns the key (key 0 means "not saved yet").
    // false: the key is supplied by the user (Course), so save() is an upsert.
    virtual bool generatedKey() const = 0;

    virtual K keyOf(const T& entity) const = 0;
    // Binds the values of columns() to parameters 1..n of the statement.
    virtual void bind(Statement& statement, const T& entity) const = 0;
    // Builds an entity from the current row: column 0 is the key, then columns() in order.
    virtual T fromRow(Statement& statement) const = 0;
};

#pragma once
#include <sqlite3.h>

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include "data/Database.hpp"
#include "data/EntityMapper.hpp"
#include "data/Repository.hpp"
#include "data/Statement.hpp"

// Generic SQLite implementation. All values are bound as parameters (never joined into the SQL).
// Table and column names come only from the mapper, and criteria columns are checked against it.
template <typename T, typename K>
class SqliteRepository : public Repository<T, K> {
public:
    SqliteRepository(Database& database, const EntityMapper<T, K>& mapper) : db_(database), mapper_(mapper) {}

    std::optional<T> findById(const K& key) override {
        Statement statement(db_, selectFrom() + " WHERE " + mapper_.keyColumn() + " = ?;");
        bindKey(statement, 1, key);
        if (!statement.step()) {
            return std::nullopt;
        }
        return mapper_.fromRow(statement);
    }

    std::vector<T> findAll() override {
        Statement statement(db_, selectFrom() + " ORDER BY " + mapper_.keyColumn() + ";");
        return readAll(statement);
    }

    std::vector<T> findWhere(const std::vector<Criterion>& criteria) override {
        std::string sql = selectFrom();
        for (std::size_t i = 0; i < criteria.size(); ++i) {
            requireKnownColumn(criteria[i].column);
            sql += (i == 0 ? " WHERE " : " AND ") + criteria[i].column + " = ?";
        }
        sql += " ORDER BY " + mapper_.keyColumn() + ";";
        Statement statement(db_, sql);
        int index = 1;
        for (const Criterion& criterion : criteria) {
            if (std::holds_alternative<int>(criterion.value)) {
                statement.bindInt(index, std::get<int>(criterion.value));
            } else {
                statement.bindText(index, std::get<std::string>(criterion.value));
            }
            ++index;
        }
        return readAll(statement);
    }

    K save(const T& entity) override {
        const std::vector<std::string> columns = mapper_.columns();
        const int count = static_cast<int>(columns.size());
        const K key = mapper_.keyOf(entity);

        if (mapper_.generatedKey() && key == K{}) {
            // New row: the database picks the key.
            Statement statement(db_, "INSERT INTO " + mapper_.tableName() + " (" + join(columns, "", ", ") +
                                         ") VALUES (" + placeholders(count) + ");");
            mapper_.bind(statement, entity);
            statement.step();
            return newKey();
        }

        if (mapper_.generatedKey()) {
            // Existing row: update it.
            Statement statement(db_, "UPDATE " + mapper_.tableName() + " SET " + join(columns, "", " = ?, ") +
                                         " = ? WHERE " + mapper_.keyColumn() + " = ?;");
            mapper_.bind(statement, entity);
            bindKey(statement, count + 1, key);
            statement.step();
            if (sqlite3_changes(db_.getConnection()) == 0) {
                throw DatabaseError("Cannot update " + mapper_.tableName() + ": row not found");
            }
            return key;
        }

        // Natural key (Course): insert, or update if the key already exists.
        std::string update;
        for (int i = 0; i < count; ++i) {
            update += (i == 0 ? "" : ", ") + columns[i] + " = excluded." + columns[i];
        }
        Statement statement(db_, "INSERT INTO " + mapper_.tableName() + " (" + join(columns, "", ", ") + ", " +
                                     mapper_.keyColumn() + ") VALUES (" + placeholders(count + 1) +
                                     ") ON CONFLICT(" + mapper_.keyColumn() + ") DO UPDATE SET " + update + ";");
        mapper_.bind(statement, entity);
        bindKey(statement, count + 1, key);
        statement.step();
        return key;
    }

protected:
    // "<key>, <columns>", with an optional alias prefix such as "s.".
    std::string selectList(const std::string& prefix = "") const {
        std::string list = prefix + mapper_.keyColumn();
        for (const std::string& column : mapper_.columns()) {
            list += ", " + prefix + column;
        }
        return list;
    }

    std::vector<T> readAll(Statement& statement) const {
        std::vector<T> result;
        while (statement.step()) {
            result.push_back(mapper_.fromRow(statement));
        }
        return result;
    }

    Database& db_;
    const EntityMapper<T, K>& mapper_;

private:
    std::string selectFrom() const { return "SELECT " + selectList() + " FROM " + mapper_.tableName(); }

    static std::string placeholders(int count) {
        std::string text;
        for (int i = 0; i < count; ++i) {
            text += (i == 0 ? "?" : ", ?");
        }
        return text;
    }

    static std::string join(const std::vector<std::string>& items, const std::string& prefix,
                            const std::string& separator) {
        std::string text;
        for (std::size_t i = 0; i < items.size(); ++i) {
            text += (i == 0 ? "" : separator) + prefix + items[i];
        }
        return text;
    }

    void requireKnownColumn(const std::string& column) const {
        const std::vector<std::string> known = mapper_.columns();
        if (column == mapper_.keyColumn() ||
            std::any_of(known.begin(), known.end(), [&](const std::string& name) { return name == column; })) {
            return;
        }
        throw DatabaseError("Unknown column '" + column + "' for table " + mapper_.tableName());
    }

    static void bindKey(Statement& statement, int index, const K& key) {
        if constexpr (std::is_same_v<K, int>) {
            statement.bindInt(index, key);
        } else {
            statement.bindText(index, key);
        }
    }

    K newKey() const {
        if constexpr (std::is_integral_v<K>) {
            return static_cast<K>(sqlite3_last_insert_rowid(db_.getConnection()));
        } else {
            throw DatabaseError("Generated keys must be integers");
        }
    }
};

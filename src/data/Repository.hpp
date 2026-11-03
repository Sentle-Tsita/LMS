#pragma once
// Repository pattern: services ask for objects and never see SQL.
#include <optional>
#include <string>
#include <variant>
#include <vector>

// One "column = value" condition for findWhere (conditions are joined with AND).
struct Criterion {
    std::string column;
    std::variant<int, std::string> value;
};

// T = entity type, K = type of its key (int for most tables, std::string for Course).
template <typename T, typename K>
class Repository {
public:
    virtual ~Repository() = default;

    virtual std::optional<T> findById(const K& key) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual std::vector<T> findWhere(const std::vector<Criterion>& criteria) = 0;
    // Inserts or updates, and returns the key of the saved row.
    virtual K save(const T& entity) = 0;
};

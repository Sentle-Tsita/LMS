#pragma once
// Factory pattern: the only place that decides which User subclass to create.
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include "Administrator.hpp"
#include "HeadOfDepartment.hpp"
#include "Lecturer.hpp"
#include "Student.hpp"

inline std::unique_ptr<User> makeUser(const std::string& role, int id, std::string username,
                                      std::string passwordHash, std::string salt, bool active) {
    if (role == "STUDENT") {
        return std::make_unique<Student>(id, std::move(username), std::move(passwordHash), std::move(salt), active);
    }
    if (role == "LECTURER") {
        return std::make_unique<Lecturer>(id, std::move(username), std::move(passwordHash), std::move(salt), active);
    }
    if (role == "HOD") {
        return std::make_unique<HeadOfDepartment>(id, std::move(username), std::move(passwordHash), std::move(salt), active);
    }
    if (role == "ADMINISTRATOR") {
        return std::make_unique<Administrator>(id, std::move(username), std::move(passwordHash), std::move(salt), active);
    }
    throw std::invalid_argument("Unknown role: " + role);
}

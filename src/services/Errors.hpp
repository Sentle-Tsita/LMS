#pragma once
#include <stdexcept>
#include <string>

// Input or business-rule problem: the user can fix it (shown as a message, never a crash).
class ValidationError : public std::runtime_error {
public:
    explicit ValidationError(const std::string& message) : std::runtime_error(message) {}
};

// The user is not allowed to do this (role or ownership rule, FR-02).
class AccessDeniedError : public std::runtime_error {
public:
    explicit AccessDeniedError(const std::string& message) : std::runtime_error(message) {}
};

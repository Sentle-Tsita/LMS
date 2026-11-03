#pragma once
#include "User.hpp"

class Lecturer : public User {
public: using User::User;
    void showMenu(std::ostream& out) const override {
        out << "\nLecturer menu\n"
            << "1. Create assignment\n"
            << "2. Grant extension\n"
            << "3. Mark submissions and write feedback\n"
            << "0. Log out\n";
    }
    std::string roleName() const override { return "LECTURER"; }
};

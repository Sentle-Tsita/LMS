#pragma once
#include "User.hpp"

class Student : public User {
public: using User::User;
    void showMenu(std::ostream& out) const override {
        out << "\nStudent menu\n"
            << "1. View my courses and assignments\n"
            << "2. Submit assignment\n"
            << "3. View released marks and feedback\n"
            << "0. Log out\n";
    }
    std::string roleName() const override { return "STUDENT"; }
};

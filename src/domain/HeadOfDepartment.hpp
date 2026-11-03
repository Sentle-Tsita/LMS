#pragma once
#include "User.hpp"

class HeadOfDepartment : public User {
public: using User::User;
    void showMenu(std::ostream& out) const override {
        out << "\nHead of Department menu\n"
            << "1. Review and moderate marks\n"
            << "2. Release approved marks\n"
            << "0. Log out\n";
    }
    std::string roleName() const override { return "HOD"; }
};

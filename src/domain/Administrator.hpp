#pragma once
#include "User.hpp"

class Administrator : public User {
public: using User::User;
    void showMenu(std::ostream& out) const override {
        out << "\nAdministrator menu\n"
            << "1. Manage accounts\n"
            << "2. Create department\n"
            << "3. Create course\n"
            << "4. Manage courses (assign lecturer, enrol student)\n"
            << "0. Log out\n";
    }
    std::string roleName() const override { return "ADMINISTRATOR"; }
};

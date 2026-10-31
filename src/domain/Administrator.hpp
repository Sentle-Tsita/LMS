#pragma once
#include <string>
#include <utility>
#include "User.hpp"
#include <iostream>

class Administrator : public User {
public: using User::User;
    void showMenu() const override{
        std::cout << "Administrator menu\n"
         << "1. Manage Account(s)\n"
         << "2. Create Department\n"
         << "3. Create Course\n"
         << "4. Manage Course(s)\n"
         << "0. Log out\n";
    }
};
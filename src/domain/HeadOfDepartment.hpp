#pragma once
#include <string>
#include <utility>
#include "User.hpp"
#include <iostream>

class HeadOfDepartment : public User {
public: using User::User;
    void showMenu() const override{
        std::cout << "HOD menu\n"
         << "1. Review and moderate marks\n"
         << "2. Release approved marks\n"
         << "0. Log out\n";
    }
};
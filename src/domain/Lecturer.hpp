#pragma once
#include <string>
#include <utility>
#include "User.hpp"
#include <iostream>

class Lecturer : public User {
public: using User::User;
    void showMenu() const override{
        std::cout << "Lecturer menu\n"
         << "1. Create assignment\n"
         << "2. Grant extension\n"
         << "3. Mark submissions and write feedback\n"
         << "0. Log out\n";
    }
};

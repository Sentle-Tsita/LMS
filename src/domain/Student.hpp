#pragma once
#include <string>
#include <utility>
#include "User.hpp"
#include <iostream>

class Student : public User {
public: using User::User;
    void showMenu() const override{
        std::cout << "Student menu\n"
         << "1. View my courses and assignments\n"
         << "2. Submit assignment\n"
         << "3. View released marks and feedback\n"
         << "0. Log out\n";
    }
};
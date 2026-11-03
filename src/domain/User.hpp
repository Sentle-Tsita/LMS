#pragma once
#include <iostream>
#include <string>
#include <utility>

class User {
    private:
        int userId;
        std::string username;
        std::string passwordHash;
        std::string salt;
        bool active = true;
    public:
        User(int id, std::string un, std::string ph, std::string sl, bool ac = true)
            : userId(id),
            username(std::move(un)),
            passwordHash(std::move(ph)),
            salt(std::move(sl)),
            active(ac) {}

        virtual ~User() = default;

        // Prints the role's menu to the given stream (polymorphism: one menu per role).
        virtual void showMenu(std::ostream& out) const = 0;
        // Role name as stored in the database: ADMINISTRATOR, LECTURER, HOD or STUDENT.
        virtual std::string roleName() const = 0;

        int getUserId() const { return userId; }
        const std::string& getUsername() const { return username; }
        const std::string& getPasswordHash() const { return passwordHash; }
        const std::string& getSalt() const { return salt; }
        bool isActive() const { return active; }
        void setActive(bool ac) { active = ac; }
};

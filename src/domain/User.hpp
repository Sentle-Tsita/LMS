#pragma once
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
            username(move(un)),
            passwordHash(move(ph)),
            salt(move(sl)),
            active(ac) {}

        virtual ~User() = default;

        virtual void showMenu() const = 0;

        int getUserId() const { return userId; }
        const std::string& getUsername() const { return username; }
        const std::string& getPasswordHash() const { return passwordHash; }
        const std::string& getSalt() const { return salt; }
        bool isActive() const { return active; }
        void setActive(bool ac) { active = ac; }
};
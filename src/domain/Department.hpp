#pragma once
#include <string>
#include <utility>

class Department {
    private:
        int departmentId;
        std::string name;
        std::optional<int> hodUserId;
    public:
        Department(int did, std::string n, std::optional<int> hid)
            : departmentId(did),
            name(std::move(n)),
            hodUserId(hid) {}

        int getDepartmentId() const { return departmentId; }
        std::string getName() const { return name; }
        std::optional<int> getHodUserId() const { return hodUserId; }
        void setName(std::string newName) { name = newName; }
        void setHodUserId(int newHodUserId) { hodUserId = newHodUserId; }

};

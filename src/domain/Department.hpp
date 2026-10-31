#pragma once
#include <string>
#include <utility>
#include <optional>

class Department {
    private:
        std::optional<int> departmentId;
        std::string name;
        std::string hodUserId;
    public:
        Department(int did, std::string n, std::string hid)
            : departmentId(did),
            name(std::move(n)),
            hodUserId(std::move(hid)) {}

        std::optional<int> getDepartmentId() const { return departmentId; }
        std::string getName() const { return name; }
        std::string getHodUserId() const { return hodUserId; }
        void setName(std::string newName) { name = newName; }
        void setHodUserId(std::string newHodUserId) { hodUserId = newHodUserId; }

};

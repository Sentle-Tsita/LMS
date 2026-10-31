#pragma once
#include <string>
#include <utility>
#include <optional>

class Course{
    private:
        std::string courseCode;
        std::string courseName;
        int departmentId;
        std::optional<int> lecturerId;
    public:
        Course(std::string cc, std::string cn, int did, int lid)
            : courseCode(std::move(cc)),
            courseName(std::move(cn)),
            departmentId(did),
            lecturerId(lid) {}

        std::string getCourseCode() const { return courseCode; }
        std::string getCourseName() const { return courseName; }
        int getDepartmentId() const { return departmentId;}
        std::optional<int> getLecturerId() const { return lecturerId; }
        void setCourseName(std::string newCourseName) { courseName = newCourseName; }
        void setDepartmentId(int newDepartmentId) { departmentId = newDepartmentId; }
        void setLecturerId(int newLecturerId) { lecturerId = newLecturerId; }
};
#pragma once
#include "DateTime.hpp"

class Assignment{
    private:
        int assignmentId;
        std::string courseCode;
        std::string title;
        DateTime deadline;
        double maxMark;
    public:
        Assignment(int aid, std::string cc, std::string t, DateTime d, double mm)
            : assignmentId(aid),
            courseCode(std::move(cc)),
            title(std::move(t)),
            deadline(d),
            maxMark(mm) {}

        int getAssignmentId() const { return assignmentId;}
        std::string getCourseCode() const { return courseCode;}
        std::string getTitle() const { return title; }
        DateTime getDeadline() const { return deadline;}
        double getMaxMark() const {return maxMark;}
};
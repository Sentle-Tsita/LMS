#pragma once
#include "DateTime.hpp"

class Extension{
    private:
        int extensionId;
        int assignmentId;
        int studentId;
        DateTime newDeadline;
    public:
        Extension(int eid, int aid, int sid, DateTime nd)
            : extensionId(eid),
            assignmentId(aid),
            studentId(sid),
            newDeadline(nd) {}

        int getExtensionId() const{ return extensionId; }
        int getAssignmentId() const { return assignmentId;}
        int getStudentId() const { return studentId; }

        DateTime getNewDeadline() { return newDeadline; }
};
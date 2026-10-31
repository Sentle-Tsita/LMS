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

        int getExtensionId() { return extensionId; }
        int getAssignmentId() { return assignmentId;}
        int getStudentId() { return studentId; }

        DateTime getNewDeadline() { return newDeadline; }
};
#pragma once
// One mapper per table. Grouped in one header for brevity; each is a small, self-contained class.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "data/EntityMapper.hpp"
#include "domain/Assignment.hpp"
#include "domain/AuditLogEntry.hpp"
#include "domain/Course.hpp"
#include "domain/Department.hpp"
#include "domain/Extension.hpp"
#include "domain/Submission.hpp"
#include "domain/UserFactory.hpp"
#include "util/DateTimeUtil.hpp"

inline void bindOptionalInt(Statement& s, int index, const std::optional<int>& v) {
    if (v) { s.bindInt(index, *v); } else { s.bindNull(index); }
}
inline void bindOptionalDouble(Statement& s, int index, const std::optional<double>& v) {
    if (v) { s.bindDouble(index, *v); } else { s.bindNull(index); }
}
inline std::optional<int> readOptionalInt(const Statement& s, int column) {
    if (s.isNull(column)) { return std::nullopt; }
    return s.getInt(column);
}
inline std::optional<double> readOptionalDouble(const Statement& s, int column) {
    if (s.isNull(column)) { return std::nullopt; }
    return s.getDouble(column);
}

class UserMapper : public EntityMapper<std::unique_ptr<User>, int> {
public:
    std::string tableName() const override { return "user"; }
    std::string keyColumn() const override { return "user_id"; }
    std::vector<std::string> columns() const override { return {"username", "password_hash", "salt", "role", "is_active"}; }
    bool generatedKey() const override { return true; }
    int keyOf(const std::unique_ptr<User>& u) const override { return u->getUserId(); }
    void bind(Statement& s, const std::unique_ptr<User>& u) const override {
        s.bindText(1, u->getUsername());
        s.bindText(2, u->getPasswordHash());
        s.bindText(3, u->getSalt());
        s.bindText(4, u->roleName());
        s.bindInt(5, u->isActive() ? 1 : 0);
    }
    std::unique_ptr<User> fromRow(Statement& s) const override {
        try {
            return makeUser(s.getText(4), s.getInt(0), s.getText(1), s.getText(2), s.getText(3), s.getInt(5) != 0);
        } catch (const std::invalid_argument& e) {
            throw DatabaseError(e.what());
        }
    }
};

class DepartmentMapper : public EntityMapper<Department, int> {
public:
    std::string tableName() const override { return "department"; }
    std::string keyColumn() const override { return "department_id"; }
    std::vector<std::string> columns() const override { return {"name", "hod_user_id"}; }
    bool generatedKey() const override { return true; }
    int keyOf(const Department& d) const override { return d.getDepartmentId(); }
    void bind(Statement& s, const Department& d) const override {
        s.bindText(1, d.getName());
        bindOptionalInt(s, 2, d.getHodUserId());
    }
    Department fromRow(Statement& s) const override { return Department(s.getInt(0), s.getText(1), readOptionalInt(s, 2)); }
};

class CourseMapper : public EntityMapper<Course, std::string> {
public:
    std::string tableName() const override { return "course"; }
    std::string keyColumn() const override { return "course_code"; }
    std::vector<std::string> columns() const override { return {"course_name", "department_id", "lecturer_id"}; }
    bool generatedKey() const override { return false; }
    std::string keyOf(const Course& c) const override { return c.getCourseCode(); }
    void bind(Statement& s, const Course& c) const override {
        s.bindText(1, c.getCourseName());
        s.bindInt(2, c.getDepartmentId());
        bindOptionalInt(s, 3, c.getLecturerId());
    }
    Course fromRow(Statement& s) const override { return Course(s.getText(0), s.getText(1), s.getInt(2), readOptionalInt(s, 3)); }
};

class AssignmentMapper : public EntityMapper<Assignment, int> {
public:
    std::string tableName() const override { return "assignment"; }
    std::string keyColumn() const override { return "assignment_id"; }
    std::vector<std::string> columns() const override { return {"course_code", "title", "deadline", "max_mark"}; }
    bool generatedKey() const override { return true; }
    int keyOf(const Assignment& a) const override { return a.getAssignmentId(); }
    void bind(Statement& s, const Assignment& a) const override {
        s.bindText(1, a.getCourseCode());
        s.bindText(2, a.getTitle());
        s.bindText(3, toIsoString(a.getDeadline()));
        s.bindDouble(4, a.getMaxMark());
    }
    Assignment fromRow(Statement& s) const override {
        return Assignment(s.getInt(0), s.getText(1), s.getText(2), fromIsoString(s.getText(3)), s.getDouble(4));
    }
};

class ExtensionMapper : public EntityMapper<Extension, int> {
public:
    std::string tableName() const override { return "extension"; }
    std::string keyColumn() const override { return "extension_id"; }
    std::vector<std::string> columns() const override { return {"assignment_id", "student_id", "new_deadline"}; }
    bool generatedKey() const override { return true; }
    int keyOf(const Extension& e) const override { return e.getExtensionId(); }
    void bind(Statement& s, const Extension& e) const override {
        s.bindInt(1, e.getAssignmentId());
        s.bindInt(2, e.getStudentId());
        s.bindText(3, toIsoString(e.getNewDeadline()));
    }
    Extension fromRow(Statement& s) const override {
        return Extension(s.getInt(0), s.getInt(1), s.getInt(2), fromIsoString(s.getText(3)));
    }
};

class SubmissionMapper : public EntityMapper<Submission, int> {
public:
    std::string tableName() const override { return "submission"; }
    std::string keyColumn() const override { return "submission_id"; }
    std::vector<std::string> columns() const override {
        return {"assignment_id", "student_id", "file_path", "submitted_at", "status", "mark", "feedback"};
    }
    bool generatedKey() const override { return true; }
    int keyOf(const Submission& s) const override { return s.getSubmissionId(); }
    void bind(Statement& st, const Submission& s) const override {
        st.bindInt(1, s.getAssignmentId());
        st.bindInt(2, s.getStudentId());
        st.bindText(3, s.getFilePath());
        st.bindText(4, toIsoString(s.getSubmittedAt()));
        st.bindText(5, toString(s.getStatus()));
        bindOptionalDouble(st, 6, s.getMark());
        st.bindText(7, s.getFeedback());
    }
    Submission fromRow(Statement& st) const override {
        Submission s(st.getInt(2), st.getInt(1), st.getText(3), fromIsoString(st.getText(4)));
        s.setSubmissionId(st.getInt(0));
        try {
            s.setStatus(statusFromString(st.getText(5)));
        } catch (const std::invalid_argument& e) {
            throw DatabaseError(e.what());
        }
        if (auto mark = readOptionalDouble(st, 6)) {
            s.setMark(*mark);
        }
        s.setFeedback(st.getText(7));
        return s;
    }
};

class AuditLogMapper : public EntityMapper<AuditLogEntry, int> {
public:
    std::string tableName() const override { return "audit_log"; }
    std::string keyColumn() const override { return "entry_id"; }
    std::vector<std::string> columns() const override {
        return {"submission_id", "changed_by", "changed_at", "old_mark", "new_mark", "reason"};
    }
    bool generatedKey() const override { return true; }
    int keyOf(const AuditLogEntry& e) const override { return e.getEntryId(); }
    void bind(Statement& s, const AuditLogEntry& e) const override {
        s.bindInt(1, e.getSubmissionId());
        s.bindInt(2, e.getChangedBy());
        s.bindText(3, toIsoString(e.getChangedAt()));
        bindOptionalDouble(s, 4, e.getOldMark());
        s.bindDouble(5, e.getNewMark());
        s.bindText(6, e.getReason());
    }
    AuditLogEntry fromRow(Statement& s) const override {
        return AuditLogEntry(s.getInt(0), s.getInt(1), s.getInt(2), readOptionalDouble(s, 4), s.getDouble(5),
                             fromIsoString(s.getText(3)), s.getText(6));
    }
};

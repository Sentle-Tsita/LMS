PRAGMA foreign_keys = ON;

-- User: one table for all four roles
CREATE TABLE user (
    user_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username       TEXT    NOT NULL UNIQUE,
    password_hash  TEXT    NOT NULL,
    salt           TEXT    NOT NULL,
    role           TEXT    NOT NULL
                   CHECK (role IN ('ADMINISTRATOR', 'LECTURER', 'HOD', 'STUDENT')),
    is_active      INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1))
);

CREATE TABLE department (
    department_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    name           TEXT    NOT NULL UNIQUE,
    hod_user_id    INTEGER UNIQUE REFERENCES user (user_id)
);

CREATE TABLE course (
    course_code    TEXT    PRIMARY KEY,
    course_name    TEXT    NOT NULL,
    department_id  INTEGER NOT NULL REFERENCES department (department_id),
    lecturer_id    INTEGER REFERENCES user (user_id)
);

-- Link table: student and course (many-to-many)
CREATE TABLE enrol (
    student_id     INTEGER NOT NULL REFERENCES user (user_id),
    course_code    TEXT    NOT NULL REFERENCES course (course_code),
    PRIMARY KEY (student_id, course_code)
);

CREATE TABLE assignment (
    assignment_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    course_code    TEXT    NOT NULL REFERENCES course (course_code),
    title          TEXT    NOT NULL,
    deadline       TEXT    NOT NULL,                    -- ISO 8601: YYYY-MM-DD HH:MM:SS
    max_mark       REAL    NOT NULL CHECK (max_mark > 0)
);

CREATE TABLE extension (
    extension_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignment (assignment_id),
    student_id     INTEGER NOT NULL REFERENCES user (user_id),
    new_deadline   TEXT    NOT NULL,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE submission (
    submission_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignment (assignment_id),
    student_id     INTEGER NOT NULL REFERENCES user (user_id),
    file_path      TEXT    NOT NULL,
    submitted_at   TEXT    NOT NULL,
    status         TEXT    NOT NULL DEFAULT 'SUBMITTED'
                   CHECK (status IN ('SUBMITTED', 'LATE', 'MARKED',
                                     'MODERATED', 'RELEASED', 'UNDER_REVIEW')),
    mark           REAL    CHECK (mark >= 0),
    feedback       TEXT,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE audit_log (
    entry_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id  INTEGER NOT NULL REFERENCES submission (submission_id),
    changed_by     INTEGER NOT NULL REFERENCES user (user_id),
    changed_at     TEXT    NOT NULL,
    old_mark       REAL,
    new_mark       REAL    NOT NULL,
    reason         TEXT    NOT NULL CHECK (length(trim(reason)) > 0)
);

CREATE INDEX idx_course_department   ON course (department_id);
CREATE INDEX idx_assignment_course   ON assignment (course_code);
CREATE INDEX idx_submission_student  ON submission (student_id);
CREATE INDEX idx_audit_submission     ON audit_log (submission_id);

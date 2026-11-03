# Design deviations

The submitted design differed from the code in the places below. This file feeds the "design
changes during implementation" part of the report. Where documents disagreed, the SQL schema won,
then the class diagram, then the sequence and activity diagrams.

- **D1.** `Repository<T>` became `Repository<T, K>`, because `Course` has a text key.
- **D2.** Added `findWhere` for lookups by student, username and course.
- **D3.** `save` now returns the new key, because domain classes cannot take a new id.
- **D4.** Mappers bind and read through `Statement&`, so NULL and numbers are handled.
- **D5.** `CourseRepository` gained `enrol`, `isEnrolled` and `findByStudent` for the `enrol` table.
- **D6.** `SubmissionRepository` gained the join queries the HoD and lecturer views need.
- **D7.** Added `Database` and `Statement` to own the connection and prepared statements.
- **D8.** `markSubmission` moved to `SubmissionService`, which has the penalty and deadline logic.
- **D9.** Added `approveMark`, for the separate MARKED to MODERATED step.
- **D10.** `showMenu` takes an output stream, and `roleName()` was added.
- **D11.** Added the `makeUser` factory.
- **D12.** Added `FileStorage` and `PasswordHasher`.
- **D13.** The late penalty is applied once, at marking, not as a separate activity step.
- **D14.** Submit now calls `AssignmentService.effectiveDeadline`, matching the class diagram.
- **D15.** Foreign-key ids kept as class attributes, because repositories need them.
- **D16.** ERD mistakes (Extension columns, "sumitted_at", `hod_user_id`) fixed. Code follows `schema.sql`.

## Not implemented

- FR-16 correcting a released mark (a Should). `UNDER_REVIEW` exists but no screen reaches it, and the
  schema has nowhere to keep a proposed mark.
- FR-04 renaming a department. Departments can only be created.
- A screen to read the audit log. Entries are written and tested.
- Changing a password, and hiding the password as it is typed.
- NFR-05 lecturer usability test. Not run.
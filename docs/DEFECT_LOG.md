# Defect log

**D-001 (minor).** Found by system test ST-10. After the HoD entered a mark change with no reason,
the error showed but the HoD was thrown back to the main menu. Cause: only the main menu caught the
error. Fix: `hodAction` now catches it inside the review loop. Retest: ST-10 passes.

**D-002 (low).** Found by the first `cppcheck` run: 10 findings (one `const&`, two by-value
parameters, seven raw loops). Fix: all fixed. Retest: `cppcheck` reports nothing.

**D-003 (low).** Found in code review. `.gitignore` could stop ignoring `*.db` if a line was added
to a file with no final newline. Fix: rewrote the file with one entry per line. Retest: `git status`
shows no database file.

Design problems found before coding are in `DESIGN_DEVIATIONS.md`.
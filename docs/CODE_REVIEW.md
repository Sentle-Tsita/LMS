# Self code review

Checked against `CONTRIBUTING.md` before the pull request `feature/application`.

- **Builds clean, tests pass:** yes (`-Wall -Wextra -Wpedantic -Werror`, 132 of 132 tests).
- **Follows the design:** yes, with 16 differences logged in `DESIGN_DEVIATIONS.md`.
- **Security:** yes. Every SQL value is bound, `findWhere` rejects unknown columns, and passwords are
  stored only as a salt and a PBKDF2 hash.
- **Errors shown, no crashes:** yes, after fixing D-001.
- **Names and style:** mostly. Domain classes have no trailing `_` on members and later layers do.
  Recorded in `CODING_STANDARD.md` instead of renaming working code.
- **cppcheck clean:** yes, after fixing 10 findings (D-002).

## Metrics (lizard, `src/`)

2088 lines, 235 functions, average complexity 2.1 (details in `docs/evidence/code-metrics.txt`).

Functions above complexity 8: `fromIsoString` (19, one validation chain with a test per branch, kept
whole), `studentAction` (16), `hodAction` (15), `adminAction` (13) and `lecturerAction` (9). The four
menu functions are menu dispatch and could be split into one function per option. All business-logic
functions are below 8.
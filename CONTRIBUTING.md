# How we use Git

## Branches
- `main` always builds and passes the tests. Releases are tagged on it (`v1.0`, `v1.1`).
- Do each piece of work on its own short branch from `main`:
  `feature/<name>`, `fix/<name>` or `docs/<name>`.
- Merge a branch into `main` with a pull request, only when the build is green.

## Commit messages
Write `area: what changed`, one change per commit.

    domain: move User into its own header
    fix: use optional for lecturer id

## Before you merge
Check your own pull request and tick each item:

- [x] It builds with no warnings and all tests pass.
- [x] New behaviour has tests.
- [x] It follows the design, or the difference is noted in `docs/DESIGN_DEVIATIONS.md`.
- [x] No SQL is built from user text, and no password is stored as plain text.
- [x] Errors are thrown as exceptions and the menu shows them without crashing.
- [x] It follows `CODING_STANDARD.md`.
- [x] `cppcheck` reports nothing.

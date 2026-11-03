# Coding standard

## Language and layers
- C++17, with no compiler extensions.
- Menus call services, services call repositories, and only the data layer touches SQLite.
  No layer skips another or calls upward.
- Headers use `#pragma once` and never contain `using namespace`.

## Names
- Classes and enums: `PascalCase` (`SubmissionService`)
- Functions and variables: `camelCase` (`effectiveDeadline`)
- Private members in services, repositories and the UI: camelCase with a trailing `_` (`submissions_`)
- Constants: `UPPER_CASE` (`MAX_BYTES`)
- SQL tables and columns: `snake_case` (`submitted_at`)

## Layout
- 4 spaces, no tabs, lines up to about 120 characters.
- Always use braces on `if`, `for` and `while`.

## Safe code
- No raw `new` or `delete`. Resources like the database and statements clean up themselves.
- Pass by `const&` unless the value is small. Mark things `const` when you can.
- Use `std::optional` for values that may be missing, and `enum class` for states.

## Errors
- A broken business rule throws `ValidationError`.
- A role or ownership problem throws `AccessDeniedError`.
- A database problem throws `DatabaseError`.
- Menus catch every error, show the message and carry on. The program must never crash on bad input.
- Services check everything again. The menus are never the only check.

## Security
- SQL values always go in as bound parameters, never joined into the text.
- Passwords are stored only as a random salt and a PBKDF2-HMAC-SHA-256 hash.
- Submitted files: only `.docx`, `.pdf` and source code, up to 5 MB. The saved name is made from
  ids and the time, never from the user's file name.

## Comments and tests
- A comment explains why, not what.
- Every service rule has a test, including the edge of each limit and one step past it.
- Tests use in-memory databases and a temporary folder, so they never touch real data.
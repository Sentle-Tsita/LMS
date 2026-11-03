#pragma once
#include <filesystem>
#include <string>
#include "domain/DateTime.hpp"

// Checks and stores submitted files (NFR-07: .docx, .pdf or source code, up to 5 MB).
class FileStorage {
public:
    static constexpr std::uintmax_t MAX_BYTES = 5ull * 1024 * 1024;

    explicit FileStorage(std::filesystem::path root) : root_(std::move(root)) {}

    // Throws ValidationError if the file is missing, has a disallowed type, or is over 5 MB.
    static void validate(const std::filesystem::path& file);

    // Validates, then copies the file into the storage folder under a name made only from ids and
    // the time (never from the user's file name). Returns the stored path.
    std::string store(const std::filesystem::path& source, int assignmentId, int studentId, DateTime when);

private:
    std::filesystem::path root_;
};

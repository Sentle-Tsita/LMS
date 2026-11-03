#include "storage/FileStorage.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <set>
#include <system_error>

#include "services/Errors.hpp"

namespace {

const std::set<std::string>& allowedExtensions() {
    static const std::set<std::string> extensions = {".docx", ".pdf", ".c",  ".cpp", ".cc", ".h",   ".hpp", ".py",
                                                     ".java", ".js",  ".ts", ".cs",  ".go", ".rs",  ".rb",  ".php",
                                                     ".sql",  ".html", ".css", ".sh"};
    return extensions;
}

std::string lowerExtension(const std::filesystem::path& file) {
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}

}  // namespace

void FileStorage::validate(const std::filesystem::path& file) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(file, error)) {
        throw ValidationError("File not found: " + file.string());
    }
    const std::string extension = lowerExtension(file);
    if (allowedExtensions().count(extension) == 0) {
        throw ValidationError("File type '" + extension + "' is not allowed. Use .docx, .pdf or a source-code file.");
    }
    const std::uintmax_t size = std::filesystem::file_size(file, error);
    if (error) {
        throw ValidationError("Cannot read the file size of " + file.string());
    }
    if (size > MAX_BYTES) {
        throw ValidationError("File is larger than 5 MB.");
    }
}

std::string FileStorage::store(const std::filesystem::path& source, int assignmentId, int studentId, DateTime when) {
    validate(source);
    const long long seconds = std::chrono::duration_cast<std::chrono::seconds>(when.time_since_epoch()).count();

    std::error_code error;
    std::filesystem::create_directories(root_, error);
    if (error) {
        throw ValidationError("Cannot create the submissions folder: " + root_.string());
    }
    const std::filesystem::path target = root_ / ("a" + std::to_string(assignmentId) + "_s" + std::to_string(studentId) +
                                                  "_" + std::to_string(seconds) + lowerExtension(source));
    std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing, error);
    if (error) {
        throw ValidationError("Could not save the file: " + error.message());
    }
    return target.string();
}

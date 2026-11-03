#pragma once
#include <stdexcept>
#include <string>

enum class SubmissionStatus {
    Submitted,
    Late,
    Marked,
    Moderated,
    Released,
    UnderReview
};

// Text stored in the database (matches the CHECK constraint in schema.sql).
inline std::string toString(SubmissionStatus status) {
    switch (status) {
        case SubmissionStatus::Submitted:   return "SUBMITTED";
        case SubmissionStatus::Late:        return "LATE";
        case SubmissionStatus::Marked:      return "MARKED";
        case SubmissionStatus::Moderated:   return "MODERATED";
        case SubmissionStatus::Released:    return "RELEASED";
        case SubmissionStatus::UnderReview: return "UNDER_REVIEW";
    }
    throw std::invalid_argument("Unknown submission status");
}

inline SubmissionStatus statusFromString(const std::string& text) {
    if (text == "SUBMITTED")    return SubmissionStatus::Submitted;
    if (text == "LATE")         return SubmissionStatus::Late;
    if (text == "MARKED")       return SubmissionStatus::Marked;
    if (text == "MODERATED")    return SubmissionStatus::Moderated;
    if (text == "RELEASED")     return SubmissionStatus::Released;
    if (text == "UNDER_REVIEW") return SubmissionStatus::UnderReview;
    throw std::invalid_argument("Unknown submission status: " + text);
}

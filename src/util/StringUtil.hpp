#pragma once
#include <cctype>
#include <string>

inline std::string trim(const std::string& text) {
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) { ++begin; }
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) { --end; }
    return text.substr(begin, end - begin);
}

#pragma once
// Dates are stored as text "YYYY-MM-DD HH:MM:SS" in UTC (see schema.sql).
// Conversion uses the civil-date algorithms of H. Hinnant, so it does not depend
// on the platform's time-zone functions.
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "domain/DateTime.hpp"

namespace datetime_detail {

inline std::int64_t daysFromCivil(std::int64_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
}

inline void civilFromDays(std::int64_t z, std::int64_t& y, unsigned& m, unsigned& d) {
    z += 719468;
    const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += (m <= 2);
}

}  // namespace datetime_detail

// Drops the fraction of a second, so stored and in-memory times always agree.
inline DateTime floorToSeconds(DateTime value) {
    return DateTime(std::chrono::duration_cast<std::chrono::seconds>(value.time_since_epoch()));
}

inline std::string toIsoString(DateTime value) {
    const std::int64_t secs = std::chrono::duration_cast<std::chrono::seconds>(value.time_since_epoch()).count();
    std::int64_t days = secs / 86400;
    std::int64_t rem = secs % 86400;
    if (rem < 0) {
        rem += 86400;
        days -= 1;
    }
    std::int64_t year = 0;
    unsigned month = 0;
    unsigned day = 0;
    datetime_detail::civilFromDays(days, year, month, day);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02u-%02u %02d:%02d:%02d", static_cast<int>(year), month, day,
                  static_cast<int>(rem / 3600), static_cast<int>((rem % 3600) / 60), static_cast<int>(rem % 60));
    return buffer;
}

// Accepts "YYYY-MM-DD HH:MM:SS" or "YYYY-MM-DD HH:MM". Throws std::invalid_argument otherwise.
inline DateTime fromIsoString(const std::string& text) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    const int fields = std::sscanf(text.c_str(), "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s);
    if (fields != 5 && fields != 6) {
        throw std::invalid_argument("Invalid date and time: '" + text + "' (use YYYY-MM-DD HH:MM)");
    }
    if (fields == 5) {
        s = 0;
    }
    if (y < 1970 || y > 9999 || mo < 1 || mo > 12 || d < 1 || d > 31 || h < 0 || h > 23 || mi < 0 || mi > 59 ||
        s < 0 || s > 59) {
        throw std::invalid_argument("Invalid date and time: '" + text + "'");
    }
    const std::int64_t days = datetime_detail::daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d));
    std::int64_t y2 = 0;
    unsigned m2 = 0;
    unsigned d2 = 0;
    datetime_detail::civilFromDays(days, y2, m2, d2);
    if (y2 != y || static_cast<int>(m2) != mo || static_cast<int>(d2) != d) {
        throw std::invalid_argument("Invalid date: '" + text + "'");  // e.g. 30 February
    }
    const std::int64_t secs = days * 86400 + h * 3600 + mi * 60 + s;
    return DateTime(std::chrono::seconds(secs));
}

// Whole seconds between two times, expressed in hours (positive when `later` is after `earlier`).
inline double hoursBetween(DateTime later, DateTime earlier) {
    const std::int64_t secs = std::chrono::duration_cast<std::chrono::seconds>(later - earlier).count();
    return static_cast<double>(secs) / 3600.0;
}

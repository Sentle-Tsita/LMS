#include <gtest/gtest.h>

#include "util/DateTimeUtil.hpp"

TEST(DateTime, EpochIsZero) {
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::seconds>(fromIsoString("1970-01-01 00:00:00").time_since_epoch()).count(), 0);
}

TEST(DateTime, KnownValue) {
    // 2000-01-01 00:00:00 UTC is 946684800 seconds after the epoch.
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::seconds>(fromIsoString("2000-01-01 00:00:00").time_since_epoch()).count(), 946684800);
}

TEST(DateTime, RoundTrip) {
    for (const char* text : {"2026-11-01 12:30:45", "2028-02-29 23:59:59", "1999-12-31 00:00:00", "2026-01-01 00:00:00"}) {
        EXPECT_EQ(toIsoString(fromIsoString(text)), text);
    }
}

TEST(DateTime, MinutesOnlyFormatMeansZeroSeconds) {
    EXPECT_EQ(toIsoString(fromIsoString("2026-11-01 08:05")), "2026-11-01 08:05:00");
}

TEST(DateTime, LeapDayRules) {
    EXPECT_NO_THROW(fromIsoString("2028-02-29 00:00:00"));
    EXPECT_THROW(fromIsoString("2027-02-29 00:00:00"), std::invalid_argument);
    EXPECT_THROW(fromIsoString("2026-04-31 00:00:00"), std::invalid_argument);
}

TEST(DateTime, RejectsInvalidText) {
    for (const char* text : {"", "tomorrow", "2026-13-01 00:00:00", "2026-00-10 00:00", "2026-11-01 24:00:00",
                             "2026-11-01 12:60:00", "2026-11-01", "1960-01-01 00:00:00"}) {
        EXPECT_THROW(fromIsoString(text), std::invalid_argument) << text;
    }
}

TEST(DateTime, HoursBetweenUsesWholeSeconds) {
    const DateTime start = fromIsoString("2026-11-10 12:00:00");
    EXPECT_DOUBLE_EQ(hoursBetween(start + std::chrono::hours(24), start), 24.0);
    EXPECT_DOUBLE_EQ(hoursBetween(start + std::chrono::seconds(86401), start), 86401.0 / 3600.0);
    EXPECT_LT(hoursBetween(start, start + std::chrono::hours(1)), 0.0);
}

TEST(DateTime, FloorToSecondsDropsTheFraction) {
    const DateTime withFraction = DateTime(std::chrono::milliseconds(1500));
    EXPECT_EQ(floorToSeconds(withFraction), DateTime(std::chrono::seconds(1)));
}

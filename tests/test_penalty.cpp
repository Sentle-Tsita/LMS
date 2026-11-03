// Boundary value analysis of the late-penalty rule (FR-11):
// 10% of the maximum mark for every started 24 hours late; zero after 72 hours.
// An exact 24, 48 or 72 hours falls in the lower band (SRS assumption).
#include <gtest/gtest.h>

#include "domain/StandardLatePenalty.hpp"

namespace {

struct Case {
    long long secondsLate;
    double expectedDeduction;  // for a maximum mark of 20
};

class PenaltyBoundary : public ::testing::TestWithParam<Case> {};

TEST_P(PenaltyBoundary, DeductionForMaxMark20) {
    StandardLatePenalty penalty;
    const double hoursLate = static_cast<double>(GetParam().secondsLate) / 3600.0;
    EXPECT_DOUBLE_EQ(penalty.calculatePenalty(hoursLate, 20.0), GetParam().expectedDeduction)
        << GetParam().secondsLate << " seconds late";
}

INSTANTIATE_TEST_SUITE_P(Boundaries, PenaltyBoundary,
                         ::testing::Values(Case{-3600, 0.0},       // early
                                           Case{0, 0.0},           // exactly on time
                                           Case{1, 2.0},           // 1 second late: first band
                                           Case{3600, 2.0},        // 1 hour
                                           Case{86400, 2.0},       // exactly 24 h: lower band
                                           Case{86401, 4.0},       // 24 h 1 s: second band
                                           Case{172800, 4.0},      // exactly 48 h
                                           Case{172801, 6.0},      // 48 h 1 s
                                           Case{259200, 6.0},      // exactly 72 h: still 30%
                                           Case{259201, 20.0},     // 72 h 1 s: zero mark
                                           Case{86400 * 30, 20.0}  // a month late
                                           ),
                         [](const ::testing::TestParamInfo<Case>& info) {
                             const long long s = info.param.secondsLate;
                             return (s < 0 ? std::string("early_") : std::string("late_")) + std::to_string(s < 0 ? -s : s) + "s";
                         });

}  // namespace

TEST(Penalty, ScalesWithTheMaximumMark) {
    StandardLatePenalty penalty;
    EXPECT_DOUBLE_EQ(penalty.calculatePenalty(72.0, 100.0), 30.0);
    EXPECT_DOUBLE_EQ(penalty.calculatePenalty(24.0, 50.0), 5.0);
}

TEST(Penalty, WorksThroughTheStrategyInterface) {
    StandardLatePenalty standard;
    const LatePenaltyStrategy& strategy = standard;  // the services only know the interface
    EXPECT_DOUBLE_EQ(strategy.calculatePenalty(30.0, 20.0), 4.0);
}

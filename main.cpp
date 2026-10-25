#include <iostream>
using namespace std;

// ---------- 1. Domain: users ----------

// ---------- 2. Domain: academic data ----------

// ---------- 3. Late penalty (Strategy) ----------

const double HoursPerDay = 24.0;
const double MaxLateHours = 72.0;
const double PenaltyPerDay = 0.1;

class LatePenaltyStrategy{
    public:
        virtual ~LatePenaltyStrategy() = default;
        virtual double calculatePenalty(double hoursLate, double maxMark) const = 0;
};

class StandardLatePenalty: public LatePenaltyStrategy{
    public:
        double calculatePenalty(double hoursLate, double maxMark) const override{
            if(hoursLate <= 0.0){
                return 0.0;
            }
            if (hoursLate > MaxLateHours){
                return maxMark;
            }
            int count = 0;
            double hoursLeft = hoursLate; 
            while(hoursLeft > 0.0){
                count ++;
                hoursLeft -= HoursPerDay;
            }
            return maxMark*count*PenaltyPerDay;
        }
};
// ---------- 4. Data access (Repository) ----------

// ---------- 5. Services ----------

// ---------- 6. Console interface ----------

#ifndef LMS_TESTING
int main() {
    cout << "LMS v0.1\n";

    StandardLatePenalty penalty;
    double hours[] = {0, 1, 24, 24.01, 25, 72, 73};

    for (double h : hours) {
        cout << h << " hours late, max mark 20: deduct "
             << penalty.calculatePenalty(h, 20) << "\n";
    }
    return 0;
}
#endif
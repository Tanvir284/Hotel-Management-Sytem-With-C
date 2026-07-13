#ifndef DATEUTIL_HPP
#define DATEUTIL_HPP

#include <array>
#include <ctime>

// Small helper struct for a calendar date.
struct Date {
    int day;
    int month;
    int year;
};

// Date helpers replacing Get_Date_Time() and Put_date() from the C code.
class DateUtil {
public:
    // Fill a std::tm with the current local time. std::localtime is fine here
    // because the application is single-threaded.
    static std::tm localNow() {
        std::time_t t = std::time(nullptr);
        return *std::localtime(&t);
    }

    // Current local date (day/month/year).
    static Date now() {
        std::tm lt = localNow();
        return {lt.tm_mday, lt.tm_mon + 1, lt.tm_year + 1900};
    }

    // Current local time as {hour, minute}.
    static std::array<int, 2> nowTime() {
        std::tm lt = localNow();
        return {lt.tm_hour, lt.tm_min};
    }

    static bool isLeap(int year) {
        return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
    }

    // Compute the check-out date `days` after `in` (mirrors Put_date()).
    static Date addDays(const Date& in, int days) {
        int monthLen[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int d = in.day, m = in.month, y = in.year;
        if (isLeap(y)) monthLen[1] = 29;

        int remaining = days;
        while (remaining > 0) {
            int leftThisMonth = monthLen[m - 1] - d;
            if (remaining <= leftThisMonth) {
                d += remaining;
                remaining = 0;
            } else {
                remaining -= (leftThisMonth + 1);
                d = 1;
                if (++m > 12) {
                    m = 1;
                    ++y;
                    monthLen[1] = isLeap(y) ? 29 : 28;
                }
            }
        }
        return {d, m, y};
    }
};

#endif // DATEUTIL_HPP

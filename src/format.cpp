#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
    int hours = seconds/3600;
    seconds -= hours*3600;
    int minutes = seconds/60;
    seconds -= minutes*60;
    struct tm format_time = {.tm_sec = seconds,
                             .tm_min=minutes,
                             .tm_hour=hours
    };
    char buffer[10];
    strftime(buffer, sizeof buffer, "%T", &format_time);
    return string(buffer); 
}
#include "format.h"

#include <string>

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int hours = seconds / 3600;
  seconds -= hours * 3600;
  int minutes = seconds / 60;
  seconds -= minutes * 60;
  struct tm format_time = {.tm_sec = (int)seconds,
                           .tm_min = minutes,
                           .tm_hour = hours,
                           // just to remove warning
                           .tm_mday = 1,
                           .tm_mon = 0,
                           .tm_year = 2022,
                           .tm_wday = 0,
                           .tm_yday = 0,
                           .tm_isdst = 0,
                           .tm_gmtoff = 0,
                           .tm_zone = 0};
  char buffer[10];
  strftime(buffer, sizeof buffer, "%T", &format_time);
  return string(buffer);
}
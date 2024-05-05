//
// Created by root on 5/5/24.
//

#include <sstream>
#include <cstring>
#include <ctime>
#include "common_utils.h"

std::string common::date::format(sql_date date)
{
  using namespace std;
  stringstream ss;
  ss << date.year << '-';
  if (date.month >= 10) {
    ss << date.month;
  } else {
    ss << '0' << date.month;
  }
  ss << '-';
  if (date.day >= 10) {
    ss << date.day;
  } else {
    ss << '0' << date.day;
  }
  return ss.str();
}
sql_date common::date::parse(std::string str)
{
  sql_date date;
  char    *token = strtok(str.data(), "-");
  date.year      = atoi(token);
  strtok(nullptr, "-");
  date.month = atoi(token);
  strtok(nullptr, "-");
  date.day = atoi(token);
  return date;
}
bool common::date::is_validate(sql_date date)
{
  if (date.month > 12 || date.month == 0) {
    return false;
  }
  int month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (date.year % 4 == 0 && (date.year % 100 != 0 || date.year % 400 == 0)) {
    month[1] ++;
  }
  if (date.day <= 0 || date.day > month[date.month - 1]) {
    return false;
  }
  return true;
}
int common::date::get_timestamp(sql_date date) {
  std::tm  timeinfo     = {};
  timeinfo.tm_year      = date.year - 1900;  // 年份从1900年开始计算
  timeinfo.tm_mon       = date.month - 1;    // 月份从0开始计算
  timeinfo.tm_mday      = date.day;
  std::time_t timestamp = std::mktime(&timeinfo);
  return (int)timestamp;
}

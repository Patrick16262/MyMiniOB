#include "function.h"
#include "common/log/log.h"
#include "sql/parser/value.h"
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>

using namespace std;

RC FunctionExpression::get_value(const Tuple &tuple, Value &value) const
{
  RC                 rc;
  std::vector<Value> params;

  for (const auto &expr : param_exprs_) {
    Value param;
    if ((rc = expr->get_value(tuple, param)) != RC::SUCCESS) {
      LOG_WARN("Failed to get value from expression, rc=%d", rc);
      return rc;
    }
    params.push_back(param);
  }

  return function_body(params, value);
}

RC FunctionExpression::try_get_value(Value &value) const
{
  RC                 rc;
  std::vector<Value> params;

  for (const auto &expr : param_exprs_) {
    Value param;
    if ((rc = expr->try_get_value(param)) != RC::SUCCESS) {
      LOG_WARN("Failed to try get value from expression, rc=%d", rc);
      return rc;
    }
    params.push_back(param);
  }

  return function_body(params, value);
}

RC LengthFunction::function_body(std::vector<Value> params, Value &result) const
{
  if (params.size() != 1) {
    LOG_WARN("Invalid number of parameters for length function, expected 1, got %zu", params.size());
    return RC::INVALID_ARGUMENT;
  }

  try {
    params[0].get_string();
  } catch (bad_cast_exception) {
    result.set_null();
    return RC::SUCCESS;
  }

  result.set_int(params[0].get_string().size());
  return RC::SUCCESS;
}

RC RoundFunction::function_body(std::vector<Value> params, Value &result) const
{
  if (params.size() != 1 && params.size() != 2) {
    LOG_WARN("Invalid number of parameters for round function, expected 1 or 2, got %zu", params.size());
    return RC::INVALID_ARGUMENT;
  }

  if (params.size() == 1) {
    float num;
    try {
      num = params[0].get_float();
    } catch (bad_cast_exception) {
      result.set_null();
      return RC::SUCCESS;
    }

    result.set_float(do_round(num));
  }

  if (params.size() == 2) {
    float num;
    int   precision;
    try {
      num       = params[0].get_float();
      precision = params[1].get_int();
    } catch (bad_cast_exception) {
      result.set_null();
      return RC::SUCCESS;
    }

    result.set_float(do_round(num, precision));
  }

  return RC::SUCCESS;
}

float RoundFunction::do_round(float num, int precision) const
{
  float factor = pow(10, precision);
  return round(num * factor) / factor;
}

RC DateFormatFunction::function_body(std::vector<Value> params, Value &result) const
{
  int    year, month, day;
  Value  date;
  string format;
  RC     rc;

  if (params.size() != 2) {
    LOG_WARN("Invalid number of parameters for date_format function, expected 2, got %zu", params.size());
    return RC::INVALID_ARGUMENT;
  }

  try {
    string date_string = params[0].get_string();
    date.set_date(date_string.c_str());
    if (date.attr_type() == NULLS) {
      result.set_null();
      return RC::SUCCESS;
    }
    format = params[1].get_string();

    int ymd = date.get_int();
    year    = ymd / 10000;
    month   = (ymd % 10000) / 100;
    day     = ymd % 100;
  } catch (bad_cast_exception) {
    result.set_null();
    return RC::SUCCESS;
  }

  string formatted_date;
  rc = do_date_format(year, month, day, format, formatted_date);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to format date, rc=%d", rc);
    return rc;
  }

  result.set_string(formatted_date.c_str());
  return RC::SUCCESS;
}

RC DateFormatFunction::do_date_format(
    int year, int month, int day, const std::string &format, std::string &result) const
{
  stringstream ss;
  for (int i = 0; i < format.size(); i++) {
    char c = format[i];
    if (c == '%') {
      if (i + 1 >= format.size()) {
        LOG_WARN("Invalid format string");
        return RC::INCORRECT_DATE_FORMAT;
      }
      i += 1;
      switch (format[i]) {
        case 'a': ss << format::get_abbreviated_weekday_name(year, month, day); break;
        case 'b': ss << format::get_abbreviated_month_name(month); break;
        case 'c': ss << format::get_month_numeric(month); break;
        case 'D': ss << format::get_day_of_month_with_english_suffix(day); break;
        case 'd': ss << format::get_day_of_month_numeric_zero_fill(day); break;
        case 'e': ss << format::get_day_of_month_numeric(day); break;
        case 'j': ss << format::get_day_of_year_zero_fill(year, month, day); break;
        case 'M': ss << format::get_month_name(month); break;
        case 'm': ss << format::get_month_numeric_zero_fill(month); break;
        case 'W': ss << format::get_weekday_name(year, month, day); break;
        case 'w': ss << format::get_day_of_week(year, month, day); break;
        case 'Y': ss << format::get_year_four_digits(year); break;
        case 'y': ss << format::get_year_two_digits(year); break;
        case '%': ss << '%'; break;
        default: LOG_WARN("Invalid format string"); return RC::INCORRECT_DATE_FORMAT;
      }
    } else {
      ss << c;
    }
  }
  result = ss.str();
  return RC::SUCCESS;
}

namespace format {
inline bool is_luner_year(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }

inline int day_of_year(int year, int month, int day)
{
  static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  int day_count = day;

  for (int i = 1; i < month; ++i) {
    day_count += days_in_month[i];
  }

  // 处理闰年的情况
  if (month > 2 && is_luner_year(year)) {
    day_count += 1;
  }

  return day_count;
}

// 已知1000-01-01是星期三
// 返回值：0-6，0表示星期一，1表示星期二，以此类推
inline int day_of_week(int year, int month, int day)
{
  int day_count = day_of_year(year, month, day);

  // 增加年份的天数
  for (int i = 1000; i < year; ++i) {
    day_count += 365;
    if (is_luner_year(i)) {
      day_count += 1;
    }
  }

  // 增加偏移量
  // 星期三(2)减去当前天(1)
  return (day_count + 1) % 7;
}

//%a Abbreviated weekday name (Sun..Sat)
inline std::string get_abbreviated_weekday_name(int year, int month, int day)
{
  static const std::string weekday_names[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  return weekday_names[day_of_week(year, month, day)];
}

//%b Abbreviated month name (Jan..Dec)
inline std::string get_abbreviated_month_name(int month)
{
  static const std::string month_names[] = {
      "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return month_names[month];
}

//%c Month, numeric (0..12)
inline std::string get_month_numeric(int month) { return std::to_string(month - 1); }

//%D Day of the month with English suffix (0th, 1st, 2nd, 3rd, …)
inline std::string get_day_of_month_with_english_suffix(int day)
{
  static const std::string suffixes[] = {"th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"};
  return std::to_string(day) + suffixes[day % 10];
}

//%d Day of the month, numeric (00..31)
inline std::string get_day_of_month_numeric_zero_fill(int day)
{
  return day < 10 ? "0" + std::to_string(day) : std::to_string(day);
}

//%e Day of the month, numeric (0..31)
inline std::string get_day_of_month_numeric(int day) { return std::to_string(day); }

//%j Day of year (001..366)
inline std::string get_day_of_year_zero_fill(int year, int month, int day)
{
  return std::to_string(day_of_year(year, month, day));
}

//%M Month name (January..December)
inline std::string get_month_name(int month)
{
  static const std::string month_names[] = {"",
      "January",
      "February",
      "March",
      "April",
      "May",
      "June",
      "July",
      "August",
      "September",
      "October",
      "November",
      "December"};
  return month_names[month];
}

//%m Month, numeric (00..12)
inline std::string get_month_numeric_zero_fill(int month)
{
  return month < 10 ? "0" + std::to_string(month) : std::to_string(month);
}

//%W Weekday name (Sunday..Saturday)
inline std::string get_weekday_name(int year, int month, int day)
{
  static const std::string weekday_names[] = {
      "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  return weekday_names[day_of_week(year, month, day)];
}

//%w Day of the week (0=Sunday..6=Saturday)
inline std::string get_day_of_week(int year, int month, int day)
{
  return std::to_string((day_of_week(year, month, day) + 1) % 7);
}

//%Y Year, numeric, four digits
inline std::string get_year_four_digits(int year)
{
  char buf[5];
  sprintf(buf, "%04d", year);
  return buf;
}

//%y Year, numeric (two digits)
inline std::string get_year_two_digits(int year)
{
  char buf[3];
  sprintf(buf, "%02d", year % 100);
  return buf;
}

}  // namespace format
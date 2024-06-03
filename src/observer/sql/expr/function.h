#pragma once

#include "expression.h"
#include "sql/parser/value.h"
#include <memory>
#include <string>
#include <vector>

class FunctionExpression : public Expression
{
public:
  FunctionExpression(std::vector<std::unique_ptr<Expression>> &param_exprs) : param_exprs_(std::move(param_exprs)) {}
  virtual ~FunctionExpression() = default;
  RC                get_value(const Tuple &tuple, Value &value) const override;
  RC                try_get_value(Value &value) const override;
  ExprType          type() const override { return ExprType::FUNCTION; };

public:
  virtual AttrType     value_type() const override                                   = 0;
  virtual FunctionType function_type() const                                         = 0;
  virtual RC           function_body(std::vector<Value> params, Value &result) const = 0;
  virtual RC           check_params() const                                          = 0;

protected:
  std::vector<std::unique_ptr<Expression>> param_exprs_;
};

class LengthFunction : public FunctionExpression
{
public:
  LengthFunction(std::vector<std::unique_ptr<Expression>> &param_exprs) : FunctionExpression(param_exprs) {}
  virtual ~LengthFunction() = default;
  AttrType     value_type() const override { return AttrType::INTS; }
  FunctionType function_type() const override { return FunctionType::LENGTH; }
  RC           function_body(std::vector<Value> params, Value &result) const override;
  RC           check_params() const override;
};

class RoundFunction : public FunctionExpression
{
public:
  RoundFunction(std::vector<std::unique_ptr<Expression>> &param_exprs) : FunctionExpression(param_exprs) {}
  virtual ~RoundFunction() = default;
  AttrType     value_type() const override { return AttrType::FLOATS; }
  FunctionType function_type() const override { return FunctionType::ROUND; }
  RC           function_body(std::vector<Value> params, Value &result) const override;
  RC           check_params() const override;

private:
  float do_round(float num, int precision = 0) const;
};

class DateFormatFunction : public FunctionExpression
{
public:
  DateFormatFunction(std::vector<std::unique_ptr<Expression>> &param_exprs) : FunctionExpression(param_exprs) {}
  virtual ~DateFormatFunction() = default;
  AttrType     value_type() const override { return AttrType::CHARS; }
  FunctionType function_type() const override { return FunctionType::DATE_FORMAT; }
  RC           function_body(std::vector<Value> params, Value &result) const override;
  RC           check_params() const override;

private:
  RC do_date_format(int year, int month, int day, const std::string &format, std::string &result) const;
};

namespace format {
inline bool is_luner_year(int year);
inline int  day_of_year(int year, int month, int day);
inline int  day_of_week(int year, int month, int day);

//%a Abbreviated weekday name (Sun..Sat)
inline std::string get_abbreviated_weekday_name(int year, int month, int day);

//%b Abbreviated month name (Jan..Dec)
inline std::string get_abbreviated_month_name(int month);

//%c Month, numeric (0..12)
inline std::string get_month_numeric(int month);

//%D Day of the month with English suffix (0th, 1st, 2nd, 3rd, …)
inline std::string get_day_of_month_with_english_suffix(int day);

//%d Day of the month, numeric (00..31)
inline std::string get_day_of_month_numeric_zero_fill(int day);

//%e Day of the month, numeric (0..31)
inline std::string get_day_of_month_numeric(int day);

//%j Day of year (001..366)
inline std::string get_day_of_year_zero_fill(int year, int month, int day);

//%M Month name (January..December)
inline std::string get_month_name(int month);

//%m Month, numeric (00..12)
inline std::string get_month_numeric_zero_fill(int month);

//%W Weekday name (Sunday..Saturday)
inline std::string get_weekday_name(int year, int month, int day);

//%w Day of the week (0=Sunday..6=Saturday)
inline std::string get_day_of_week(int year, int month, int day);

//%Y Year, numeric, four digits
inline std::string get_year_four_digits(int year);

//%y Year, numeric (two digits)
inline std::string get_year_two_digits(int year);
}  // namespace format
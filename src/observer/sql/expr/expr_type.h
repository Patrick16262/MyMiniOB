#pragma once
/**
 * @brief 表达式类型
 * @ingroup Expression
 */
#include <cctype>
#include <cstring>

enum class ExprType
{
  NONE,
  STAR,         ///< 星号，表示所有字段，现在暂未使用(do not use now)
  FIELD,        ///< 字段。在实际执行时，根据行数据内容提取对应字段的值
  CELL_REF,     ///< 元组引用。根据元组内容提取对应字段的值
  VALUE,        ///< 常量值
  CAST,         ///< 需要做类型转换的表达式
  COMPARISON,   ///< 需要做比较的表达式
  CONJUNCTION,  ///< 多个表达式使用同一种关系(AND或OR)来联结
  ARITHMETIC,   ///< 算术运算
  FUNCTION,     ///< 函数表达式
  NOT,          ///< NOT取反表达式
  LIKE,         ///< LIKE表达式
  IN,           ///< IN表达式
  EXISTS,       ///< EXIST表达式
  AGGREGATE,    ///< 聚合函数, 现在只用在解析GROUP BY时使用
  SUBQUERY,     ///< 子查询
  IS_NULL,      ///< IS NULL表达式
};

enum class ArithmeticType
{
  ADD,  ///< 加法
  SUB,  ///< 减法
  MUL,  ///< 乘法
  DIV,  ///< 除法
  NEGATIVE,
};

enum class AggregateType
{
  INVALID,
  COUNT,
  SUM,
  AVG,
  MAX,
  MIN,
  GROUP,  ///< GROUP BY的字段，这个是在解析GROUP BY时使用的
};

// length、round和date_format。
enum class FunctionType
{
  INVALID,
  LENGTH,
  ROUND,
  DATE_FORMAT
};

// 在mysql中，关联聚合函数不可以存在于from子句中，无论是单蹦出来一个还是藏join里面

enum class SubqueryType
{
  SINGLE_CELL,  ///< 子查询返回的结果只有一个单元
  SINGLE_COL,   ///< 子查询返回的结果只有多个单元的一列，用于IN，返回值是一个json数组
  EXISTS,       ///< 子查询返回的结果是一个bool值, 用于EXISTS
  TABLE,        ///< 子查询返回的结果是一个表
};

namespace common {

bool is_aggregate_function(const char *function_name);

FunctionType get_function_type(const char *function_name);

AggregateType get_aggregate_type(const char *function_name);

}  // namespace common

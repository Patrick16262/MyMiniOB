#pragma once
/**
 * @brief 表达式类型
 * @ingroup Expression
 */
enum class ExprType
{
  NONE,
  STAR,         ///< 星号，表示所有字段，现在暂未使用(do not use now)
  FIELD,        ///< 字段。在实际执行时，根据行数据内容提取对应字段的值
  VALUE,        ///< 常量值
  CAST,         ///< 需要做类型转换的表达式
  COMPARISON,   ///< 需要做比较的表达式
  CONJUNCTION,  ///< 多个表达式使用同一种关系(AND或OR)来联结
  ARITHMETIC,   ///< 算术运算
  AGGREGATE,    ///< 聚合函数
  SUBQUERY,     ///< 子查询
};

enum class ArithmeticType
{
  ADD, ///< 加法
  SUB, ///< 减法
  MUL, ///< 乘法
  DIV, ///< 除法
  NEGATIVE,
};
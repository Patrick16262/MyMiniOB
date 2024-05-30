#pragma once

/**
 * @brief 描述比较运算符
 * @ingroup SQLParser
 */
enum CompOp
{
  EQUAL_TO,     ///< "="
  LESS_EQUAL,   ///< "<="
  NOT_EQUAL,    ///< "<>"
  LESS_THAN,    ///< "<"
  GREAT_EQUAL,  ///< ">="
  GREAT_THAN,   ///< ">"
  NO_OP
};

enum class AggregateType
{
  COUNT,
  SUM,
  AVG,
  MAX,
  MIN
};

enum class ConjunctionType {
  AND,
  OR
};

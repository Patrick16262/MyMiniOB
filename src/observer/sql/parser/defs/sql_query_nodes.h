#include "expression_sql_defs.h"
#include "vector"

/**
 * @brief 描述一个select语句
 * @ingroup SQLParser
 * @details 一个正常的select语句描述起来比这个要复杂很多，这里做了简化。
 * 一个select语句由三部分组成，分别是select, from, where。
 * select部分表示要查询的字段，from部分表示要查询的表，where部分表示查询的条件。
 * 比如 from 中可以是多个表，也可以是另一个查询语句，这里仅仅支持表，也就是 relations。
 * where 条件 conditions，这里表示使用AND串联起来多个条件。正常的SQL语句会有OR，NOT等，
 * 甚至可以包含复杂的表达式。
 */

struct SelectSqlNode
{
  std::vector<ExpressionSqlNode *>   attributes;  ///< attributes in select clause
  std::vector<std::string>      relations;      ///< 查询的表
  ExpressionSqlNode *condition = nullptr;                 ///< 查询条件，使用AND串联起来多个条件
};

class SubqueryExpressionSqlNode : public ExpressionSqlNode
{
public:
  SelectSqlNode subquery;

  SubqueryExpressionSqlNode() { ExpressionSqlNode::expr_type = ExprType::SUBQUERY; }
};

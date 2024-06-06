#pragma once

#include "sql/expr/expr_type.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include <cassert>
#include <memory>
#include <vector>
#include "sql/parser/defs/sql_query_nodes.h"

/**
 * @brief 验证单个表达式中聚合/子查询结构是否合法
 * 可查出错误包括：
 * count(count(*)) -> 聚合中包含聚合
 * count(*) and field -> 聚合和字段混用
 * count(field1, field2) -> 聚合函数中包含多个字段
 * select(select(field1, field2)) -> 子查询在非Exists情况下返回超过一个col的行数据
 *
 * @note 该类不会检查表达式的语义是否正确，如字段是否存在，函数是否存在等
 *        该类是一次性的，不可重用
 */

class ExpressionStructValidator
{
public:
  // validate 前会重置状态
  RC validate(const ExpressionSqlNode *sql_node);

  bool got_aggregate() const { return !allow_non_aggregate; }
  bool got_field() const { return !allow_aggregate; }

private:
  // 向下传递的状态
  struct State
  {
    bool in_aggregate;
    bool allow_multiple_col = false;
  };

  // 向上返回的状态
  bool allow_aggregate     = true;
  bool allow_non_aggregate = true;

  RC validate(const ExpressionSqlNode *sql_node, State state);
};

/**
 * @brief 对表达式树结构进行重构，以符合计划执行需要
 *  重构规则：
 *  1. 将groupby表达式在表达式树中找出来，将其标记为Groupby
 *  2. 将function表达式中的聚合函数转换为aggregate表达式
 *  3. 将aggregate外部的表达式拆分为内外两部分
 *  4. 提取subquery表达式, 根据表达式上下文设置subquery的类型
 *  5. 将count(*) 转换为count(1)
 *
 *  在重构之前，会先使用ExpressionStructValidator验证表达式树结构是否合法
 *
 *  在对表达式树结构进行重构时，也会验证以下错误
 * select id, count(*) from tb                    >>> 聚合函数和非Groupby表达式混用
 * select id, age, count(*) from tb group by id   >>> 出现Groupby表达式和选择的字段不一致
 *
 * @note 为了提高性能，会在原表达式树上进行修改
 *       该类是一次性的，不可重用
 */
class ExpressionStructRefactor
{
public:
  RC refactor(std::vector<ExpressionSqlNode *> query_sql_list, const std::vector<ExpressionSqlNode *> &groupby = {});
  RC refactor(ExpressionSqlNode *&sql_node, const std::vector<ExpressionSqlNode *> &groupby = {});

  std::vector<AggregateType> aggregate_types() const { return aggregate_types_; }

  std::vector<std::unique_ptr<ExpressionSqlNode>> &aggregate_childs() { return aggregate_childs_; }

  std::vector<std::unique_ptr<SelectSqlNode>> &subqueries()
  {
    assert(subqueries_.size() == subquery_types_.size());
    return subqueries_;
  }
  std::vector<SubqueryType> &subquery_types()  { return subquery_types_; }

  std::vector<TupleCellSpec> &subquery_cells()  { return subquery_cells_; }

private:
  RC refactor_internal(ExpressionSqlNode *&sql_node);
  RC init();

private:
  std::vector<ExpressionSqlNode *> groupby_;

  std::vector<AggregateType>                      aggregate_types_;
  std::vector<std::unique_ptr<ExpressionSqlNode>> aggregate_childs_;

  std::vector<std::unique_ptr<SelectSqlNode>> subqueries_;
  std::vector<SubqueryType>                   subquery_types_;
  std::vector<TupleCellSpec>                  subquery_cells_;

  SubqueryType current_subquery_type_ = SubqueryType::SINGLE_CELL;

  ExpressionStructValidator validator_;
};
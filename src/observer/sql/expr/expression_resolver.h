#pragma once

#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_refactor.h"
#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "sql/stmt/table_ref_desc.h"
#include "storage/db/db.h"
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief 表达式生成器，用于将sql语法树中的表达式节点转换为表达式对象
 * 该对象可复用
 */

class ExpressionGenerator
{
public:
  ExpressionGenerator() = default;
  ExpressionGenerator(
      Db *db, std::vector<TableFactorDesc> table_desc, std::vector<TupleCellSpec> tuple_schema_ = {} /*子查询用到的*/)
      : db_(db)
  {
    for (auto &desc : table_desc) {
      for (auto &field : desc.fields()) {
        field_table_map_.insert({field.field_name(), desc});
      }
    }
    for (auto &cell : tuple_schema_) {
      outter_alias_set_.insert(cell.alias());
    }
  }
  virtual ~ExpressionGenerator() = default;

  RC generate_expression(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

private:
  RC generate_expression(const ValueExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const FieldExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ArithmeticExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ComparisonExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ConjunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const CastExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const LikeExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const NotExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const FunctionExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const TupleCellExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const IsNullExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const InExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);
  RC generate_expression(const ExistsExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

private:
  std::unordered_multimap<std::string, TableFactorDesc> field_table_map_;   // 字段->表
  std::unordered_set<std::string>                       outter_alias_set_;  // 父查询中的别名
  Db                                                   *db_ = nullptr;      // nullable
};

/**
 * @brief 常量表达式解析器，目前暂时没有用到
 */
class ConstExpressionResovler
{
public:
  ConstExpressionResovler()          = default;
  virtual ~ConstExpressionResovler() = default;

  RC resolve(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

private:
  ExpressionGenerator generator_;
};

/**
 * @brief 即不包括子查询也不包括聚合函数的表达式解析器，用于解析join条件中的表达式
 * @note 该对象是有状态的一次性对象，不要重用
 *
 */
class JoinConditionExpressionResolver
{
public:
  JoinConditionExpressionResolver(Db *db, std::vector<TableFactorDesc> table_desc) : generator_(db, table_desc){};
  virtual ~JoinConditionExpressionResolver() = default;

  RC resolve(const ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

private:
  ExpressionGenerator generator_;
};

/**
 * @brief 不包括聚合函数的表达式解析器，用于解析where条件中的表达式
 * @note 该对象是有状态的一次性对象，不要重用
 */
class WhereConditionExpressionResolver
{
public:
  WhereConditionExpressionResolver(
      Db *db, std::vector<TableFactorDesc> table_desc, std::vector<TupleCellSpec> tuple_schema)
      : generator_(db, table_desc), db_(db), table_desc_(std::move(table_desc)), tuple_schema_(tuple_schema){};
  virtual ~WhereConditionExpressionResolver() = default;

  RC resolve(ExpressionSqlNode *sql_node, std::unique_ptr<Expression> &expr);

public:
  std::vector<std::unique_ptr<SubqueryExpressionSqlNode>> &subquery_sqls() { return refactor_.subqueries(); }
  std::vector<SubqueryType>                               &subquery_types() { return refactor_.subquery_types(); }
  std::vector<TupleCellSpec>                              &subquery_cell_desc() { return refactor_.subquery_cells(); }

private:
  ExpressionGenerator      generator_;
  ExpressionStructRefactor refactor_;

  Db                          *db_ = nullptr;
  std::vector<TableFactorDesc> table_desc_;
  std::vector<TupleCellSpec>   tuple_schema_;
};

/**
 * @brief 聚合函数表达式解析器, 不解析聚合函数
 * @note 当前将Groupby的resovler认为和where相同
 *
 */
using GroupByExpressionResolver = WhereConditionExpressionResolver;

/**
 * @brief 对查询的属性进行解析
 * @note 该对象是有状态的一次性对象，不要重用
 */

class ProjectExpressionResovler
{
public:
  ProjectExpressionResovler() = default;
  ProjectExpressionResovler(Db *db, std::vector<TableFactorDesc> table_desc, std::vector<TupleCellSpec> outter_tuple,
      std::vector<ExpressionSqlNode *> group_exprs)
      : generator_(db, table_desc, outter_tuple),
        db_(db),
        table_desc_(std::move(table_desc)),
        outter_tuple_(std::move(outter_tuple)),
        group_exprs_(std::move(group_exprs))

  {
    assert(db_ != nullptr);
  }
  virtual ~ProjectExpressionResovler() = default;

  RC generate_query_list(
    const vector<ExpressionWithAliasSqlNode *> &sql_nodes, vector<unique_ptr<Expression>> &query_exprs);

public:
  std::vector<SubqueryType>                          &subquery_types() { return subquery_types_; }
  std::vector<unique_ptr<SubqueryExpressionSqlNode>> &subquerys() { return subquerys_; }
  std::vector<TupleCellSpec>                         &subquery_cell_desc() { return subquery_cell_desc_; }
  std::vector<unique_ptr<AggregateDesc>>             &aggregate_desc() { return aggregate_desc_; }
  std::vector<TupleCellSpec>                         &attr_tuple() { return attr_tuple_; }

private:
  RC wildcard_fields(FieldExpressionSqlNode *wildcard_expression, vector<unique_ptr<Expression>> &query_exprs);

private:
  std::vector<SubqueryType>                          subquery_types_;
  std::vector<unique_ptr<SubqueryExpressionSqlNode>> subquerys_;
  std::vector<TupleCellSpec>                         subquery_cell_desc_;

  std::vector<unique_ptr<AggregateDesc>> aggregate_desc_;

  std::vector<TupleCellSpec> attr_tuple_;

private:
  ExpressionGenerator generator_;

  Db                              *db_ = nullptr;
  std::vector<TableFactorDesc>     table_desc_;
  std::vector<TupleCellSpec>       outter_tuple_;
  std::vector<ExpressionSqlNode *> group_exprs_;
};
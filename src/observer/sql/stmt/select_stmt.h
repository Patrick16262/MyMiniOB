/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/6/5.
//

#pragma once

#include <memory>
#include <vector>

#include "common/rc.h"
#include "sql/expr/expr_type.h"
#include "sql/expr/expression.h"

#include "sql/expr/tuple_cell.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/table_ref_desc.h"
#include "table_stmt.h"

class FieldMeta;
class Db;
class Table;
class SubqueryStmt;

/**
 * @brief 表示select语句
 * @ingroup Statement
 */
class SelectStmt : public Stmt
{
public:
  SelectStmt() = default;

  StmtType type() const override { return StmtType::SELECT; }

public:
  static RC create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt);
  static RC create(Db *db, const SelectSqlNode &select_sql, const std::vector<TupleCellSpec> outter_tuple,
      const std::vector<TableFactorDesc> outter_table, Stmt *&stmt);

  std::vector<std::unique_ptr<Expression>>    &project_expr_list() { return project_expr_list_; }
  std::vector<TupleCellSpec>                  &tuple_schema() { return tuple_schema_; }
  std::unique_ptr<TableStmt>                  &table_stmt() { return table_stmt_; }
  std::vector<std::unique_ptr<SubqueryStmt>>  &subquery_list() { return subquery_list_; }
  std::vector<std::unique_ptr<AggregateDesc>> &aggregate_list() { return aggregate_list_; }
  std::vector<std::unique_ptr<Expression>>    &group_by_list() { return group_by_list_; }
  std::vector<std::unique_ptr<ExprWithOrder>> &order_by_list() { return order_by_list_; }
  std::unique_ptr<Expression>                 &filter() { return filter_; }
  std::vector<TableFactorDesc>                &table_descs() { return table_descs_; }

  const std::vector<std::unique_ptr<Expression>>    &project_expr_list() const { return project_expr_list_; }
  const std::vector<TupleCellSpec>                  &tuple_schema() const { return tuple_schema_; }
  const std::unique_ptr<TableStmt>                  &table_stmt() const { return table_stmt_; }
  const std::vector<std::unique_ptr<SubqueryStmt>>  &subquery_list() const { return subquery_list_; }
  const std::vector<std::unique_ptr<AggregateDesc>> &aggregate_list() const { return aggregate_list_; }
  const std::vector<std::unique_ptr<Expression>>    &group_by_list() const { return group_by_list_; }
  const std::vector<std::unique_ptr<ExprWithOrder>> &order_by_list() const { return order_by_list_; }
  const std::unique_ptr<Expression>                 &filter() const { return filter_; }
  const std::vector<TableFactorDesc>                &table_descs() const { return table_descs_; }

private:
  /**
   * SelectStmt的解析过程
   * 解析表 -> 解析属性 -> 解析where表达式
   * 
   */

  RC resolve_table(const std::vector<TableReferenceSqlNode *> &table_refs);
  RC resovle_attributes(const std::vector<ExpressionWithAliasSqlNode *> &attributes);
  RC resolve_where( ExpressionSqlNode *where_expr);

private:
  Db                              *db_ = nullptr;
  std::vector<TableFactorDesc>     table_descs_;
  std::vector<TupleCellSpec>       outter_tuple_schema_;  // 子查询用到的
  std::vector<ExpressionSqlNode *> group_exprs_;          // 子查询用到的

  std::vector<std::unique_ptr<Expression>> project_expr_list_;  // 用于生成ProjectOperator
  std::vector<TupleCellSpec>               tuple_schema_;       // 显示的名字,
                                             // 由于投影的名称可能是别名，所以需要计算出应该显示的名字

  std::unique_ptr<TableStmt> table_stmt_ = nullptr;  // 用于生成相应的表

  std::vector<std::unique_ptr<SubqueryStmt>> subquery_list_;  // 在表达式中的子查询

  std::vector<std::unique_ptr<AggregateDesc>> aggregate_list_;  // 用于生成AggregateOperator, 当不需要聚合时，该字段为空
  std::vector<std::unique_ptr<Expression>> group_by_list_;  // 用于生成GroupByOperator, 当不需要group by时，该字段为空

  std::vector<std::unique_ptr<ExprWithOrder>> order_by_list_;  // 用于生成SortOperator, 当不需要排序时，该字段为空

  std::unique_ptr<Expression> filter_ = nullptr;  // 用于生成PerdictOperator
};

/**
 * @brief 单列子查询
 */

class SubqueryStmt : Stmt
{
public:
  virtual StmtType type() const { return StmtType::SUBQUERY; }

  SubqueryStmt(std::unique_ptr<SelectStmt> &subquery, SubqueryType type, TupleCellSpec cell_spec)
      : subquery_(std::move(subquery)), subquery_type_(type), cell_spec_(std::move(cell_spec))
  {}

  std::unique_ptr<SelectStmt> &subquery() { return subquery_; }
  SubqueryType                 subquery_type() const { return subquery_type_; }
  TupleCellSpec                cell_spec() const { return cell_spec_; }

public:
  static RC create(
      Db *db, const SelectSqlNode &subquery_sql, SubqueryType type, TupleCellSpec cell_spec, SubqueryStmt *&stmt);

private:
  std::unique_ptr<SelectStmt> subquery_;
  SubqueryType                subquery_type_;
  TupleCellSpec               cell_spec_;
};

/**
 * @brief 子查询语句生成器
 * @note 一次性对象，不要重用
 */
class SubqueryStmtGenerator
{
public:
  SubqueryStmtGenerator(
      Db *db, const std::vector<TableFactorDesc> &table_descs, std::vector<TupleCellSpec> tuple_schema)
      : db_(db), outter_table_descs_(table_descs), outter_tuple_schema_(std::move(tuple_schema))
  {}

  RC create(const SelectSqlNode *subquery_sql, SubqueryType type, TupleCellSpec cell_spec,
      std::unique_ptr<SubqueryStmt> &stmt);
  RC create(const std::vector<SelectSqlNode *> subquery_sqls, const std::vector<SubqueryType> &types,
      const std::vector<TupleCellSpec> &cell_specs, std::vector<std::unique_ptr<SubqueryStmt>> &stmts);

private:
  Db                          *db_ = nullptr;
  std::vector<TableFactorDesc> outter_table_descs_;
  std::vector<TupleCellSpec>   outter_tuple_schema_;
};
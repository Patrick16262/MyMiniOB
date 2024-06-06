#pragma once

#include "sql/expr/expression.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "storage/table/table.h"
#include <memory>
#include <vector>
#include "sql/stmt/table_ref_desc.h"

class Db;
class SelectStmt;
class TableStmt;

/**
 * @brief TableStmt生成器，用于将sql语法树中的表引用节点转换为TableStmt对象
 * @note 该对象是有状态的一次性对象，不要重用
 *
 */

class TableStmtGenerator
{
public:
  RC create(Db *db, const std::vector<TableReferenceSqlNode *> table_sqls, TableStmt *&stmt);

  std::vector<TableFactorDesc> &table_descs() { return table_descs_; }

private:
  RC create(Db *db, const TableReferenceSqlNode *table_sql, TableStmt *&stmt);
  RC create(Db *db, const TablePrimarySqlNode *table_sql, TableStmt *&stmt);
  RC create(Db *db, const TableSubquerySqlNode *table_sql, TableStmt *&stmt);
  RC create(Db *db, const TableJoinSqlNode *table_sql, TableStmt *&stmt);

private:
  std::vector<TableFactorDesc> table_descs_;
};

/**
 * @brief 表
 *
 */

class TableStmt
{
public:
  virtual ~TableStmt();

  friend class TableStmtGenerator;

  RelationType type() const { return type_; }
  const char  *alias_name() const { return alias_name_.c_str(); }

  // table primary
  Table *table() const { return table_; }

  // table subquery
  std::unique_ptr<SelectStmt> &subquery() { return subquery_; }

  // table join
  std::unique_ptr<TableStmt>  &left_table() { return left_table_; }
  std::unique_ptr<TableStmt>  &right_table() { return right_table_; }
  std::unique_ptr<Expression> &join_condition() { return join_condition_; }

private:
  TableStmt() = default;

private:
  RelationType type_;
  std::string  alias_name_;

  // table primary
  Table *table_;

  // table subquery
  std::unique_ptr<SelectStmt> subquery_;

  // table join
  std::unique_ptr<TableStmt>  left_table_;
  std::unique_ptr<TableStmt>  right_table_;
  std::unique_ptr<Expression> join_condition_;
};
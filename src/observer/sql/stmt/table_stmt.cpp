#include "table_stmt.h"
#include "common/log/log.h"
#include "select_stmt.h"
#include "sql/expr/expression_resolver.h"
#include "sql/parser/defs/expression_sql_defs.h"
#include "sql/parser/defs/sql_node_fwd.h"
#include "sql/parser/defs/sql_query_nodes.h"
#include "sql/stmt/table_ref_desc.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

RC TableSqlResovler::create(Db *db, const std::vector<TableReferenceSqlNode *> table_sqls, TableStmt *&stmt)
{
  assert(table_sqls.size() > 0);
  if (table_descs_.size() > 0) {
    LOG_WARN("reuse one-time object: TableStmtGenerator");
    table_descs_.clear();
  }

  // join all tables
  TableReferenceSqlNode     *sql_node = table_sqls[0];
  vector<TableJoinSqlNode *> to_delete;

  // 注意：这里的资源为借用，不应该直接delete
  for (size_t i = 1; i < table_sqls.size(); i++) {
    TableJoinSqlNode *join_node = new TableJoinSqlNode();
    join_node->left             = sql_node;
    join_node->right            = table_sqls[i];
    to_delete.push_back(join_node);
    sql_node = join_node;
  }

  RC rc = create(db, sql_node, stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("create table stmt failed");
    return rc;
  }

  for (auto node : to_delete) {
    node->left  = nullptr;
    node->right = nullptr;
    delete node;
  }

  return RC::SUCCESS;
}

RC TableSqlResovler::create(Db *db, const TableReferenceSqlNode *table_sql, TableStmt *&stmt)
{
  switch (table_sql->type) {
    case RelationType::TABLE: {
      return create(db, static_cast<const TablePrimarySqlNode *>(table_sql), stmt);
    } break;
    case RelationType::SUBQUERY: {
      return create(db, static_cast<const TableSubquerySqlNode *>(table_sql), stmt);
    } break;
    case RelationType::JOIN: {
      return create(db, static_cast<const TableJoinSqlNode *>(table_sql), stmt);
    } break;
    default: {
      assert(false);
      return RC::UNIMPLENMENT;
    } break;
  }
}

RC TableSqlResovler::create(Db *db, const TablePrimarySqlNode *table_sql, TableStmt *&stmt)
{
  string            relation_name = table_sql->relation_name;
  Table            *table         = db->find_table(relation_name.c_str());
  string            table_name    = table_sql->alias.empty() ? relation_name : table_sql->alias;
  vector<FieldDesc> fields;

  if (table == nullptr) {
    LOG_WARN("table not exist: %s", relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  for (auto &meta : *table->table_meta().field_metas()) {
    fields.emplace_back(meta.name(), meta.visible());
  }

  table_descs_.emplace_back(table_name, fields, RelationType::TABLE, table);

  // everything are ok
  stmt                 = new TableStmt();
  stmt->relation_type_ = RelationType::TABLE;
  stmt->table_         = table;
  stmt->alias_name_    = table_sql->alias;
  return RC::SUCCESS;
}

RC TableSqlResovler::create(Db *db, const TableSubquerySqlNode *table_sql, TableStmt *&stmt)
{
  RC                rc;
  SelectStmt       *subquery   = nullptr;
  Stmt            *&tmp        = *reinterpret_cast<Stmt **>(&subquery);
  string            table_name = table_sql->alias;
  vector<FieldDesc> fields;

  if (table_name.empty()) {
    LOG_WARN("Every derived table must have its own alias");
    return RC::ALIAS_NOT_EXIST;
  }

  rc = SelectStmt::create(db, table_sql->subquery, tmp);
  if (rc != RC::SUCCESS) {
    LOG_WARN("create subquery failed, rc = %d: %s", rc, strrc(rc));
    return rc;
  }

  for (auto &attr : table_sql->subquery.attributes) {
    fields.emplace_back(attr->alias.empty() ? attr->expr->name
                                            : static_cast<FieldExpressionSqlNode *>(attr->expr)->field.attribute_name,
        true);
  }

  table_descs_.emplace_back(table_name.c_str(), fields, RelationType::SUBQUERY);

  stmt                 = new TableStmt();
  stmt->relation_type_ = RelationType::SUBQUERY;
  stmt->subquery_.reset(subquery);
  stmt->alias_name_ = table_sql->alias;

  return RC::SUCCESS;
}

RC TableSqlResovler::create(Db *db, const TableJoinSqlNode *table_sql, TableStmt *&stmt)
{
  RC                     rc;
  TableStmt             *left_table     = nullptr;
  TableStmt             *right_table    = nullptr;
  unique_ptr<Expression> join_condition = nullptr;

  rc = create(db, table_sql->left, left_table);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  rc = create(db, table_sql->right, right_table);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  if (table_sql->condition) {
    JoinConditionExpressionResolver expr_resolver(db, table_descs_);
    rc = expr_resolver.resolve(table_sql->condition, join_condition);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }

  stmt                 = new TableStmt();
  stmt->relation_type_ = RelationType::JOIN;
  stmt->left_table_.reset(left_table);
  stmt->right_table_.reset(right_table);
  stmt->join_condition_ = std::move(join_condition);

  return RC::SUCCESS;
}

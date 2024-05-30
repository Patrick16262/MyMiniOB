#include "filter_stmt.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/expr/expression_resolver.h"
#include <memory>


RC FilterStmt::create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> &tables,
      ExpressionSqlNode *condition, FilterStmt *&stmt) {
  ExpressionGenerator expression_generator(default_table, tables);
  unique_ptr<Expression> expression;
  FilterStmt *filter_stmt = new FilterStmt();
  RC rc;

  rc = expression_generator.generate_expression(condition, expression);
  if (rc != RC::SUCCESS) {
    LOG_WARN("generate expression failed, due to %d:%s", rc, strrc(rc));
    delete filter_stmt;
    return rc;
  }
  filter_stmt->expression_ = std::move(expression);
  stmt = filter_stmt;
  return RC::SUCCESS;
}

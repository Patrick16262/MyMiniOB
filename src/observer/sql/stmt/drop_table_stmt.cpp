

#include "drop_table_stmt.h"
#include "common/log/log.h"


RC DropTableStmt::create(Db *db, const DropTableSqlNode &drops, Stmt *&stmt) {
  Table * non_null_ptr = db->find_table(drops.relation_name.c_str());
  
  if (non_null_ptr == nullptr) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), drops.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  
  stmt = new DropTableStmt(drops.relation_name, db);
  
  return RC::SUCCESS;
}

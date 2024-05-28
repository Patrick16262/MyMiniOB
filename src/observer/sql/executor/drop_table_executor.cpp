#include "drop_table_executor.h"
#include "common/rc.h"

RC DropTableExecutor::execute(Db *db, const std::string &table_name)
{
  RC rc = db->drop_table(table_name.c_str());
  if (rc != RC::SUCCESS) {
    LOG_WARN("drop table failed. db=%s, table_name=%s, due to %s", db->name(), table_name.c_str(), strrc(rc));
  }
  return rc;
}

#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <string>

class DropTableStmt : public Stmt
{
public:
  StmtType type() const override { return StmtType::DROP_TABLE; }

  static RC create(Db *db, const DropTableSqlNode &drops, Stmt *&stmt);

  std::string get_table_name() const { return table_name_; }
  Db *get_db() const { return db_; }

private:
  DropTableStmt(const std::string &table_name, Db *db)
      : table_name_(table_name), db_(db)
  {}

private:
  std::string table_name_;
  Db         *db_;
};
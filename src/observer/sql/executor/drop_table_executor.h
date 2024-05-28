#include "common/rc.h"
#include "storage/db/db.h"

class DropTableExecutor
{
public:
  DropTableExecutor()          = default;
  virtual ~DropTableExecutor() = default;

  RC execute(Db *db, const std::string &table_name);
};
#include "sql/operator/update_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "storage/record/record.h"
#include <cassert>
#include <memory>
#include <vector>

RC UpdatePhysicalOperator::open(Trx *trx)
{
  assert(children_.size() == 1);

  RC                         rc;
  RowTuple                  *child_tuple;
  PhysicalOperator          *child = children_[0].get();
  vector<unique_ptr<Record>> records_to_delete;
  vector<unique_ptr<Record>> records_to_insert;

  rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to open child operator, rc = %s", strrc(rc));
    return rc;
  }

  // 查出所有要更新的记录
  while ((rc = child->next()) == RC::SUCCESS) {
    child_tuple              = static_cast<RowTuple *>(child->current_tuple());
    Record *record_to_insert = new Record();

    rc = update_record_maker_.update(child_tuple, new_values_, *record_to_insert);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to update record, rc = %s", strrc(rc));
      return rc;
    }

    records_to_delete.push_back(std::make_unique<Record>(child_tuple->record()));
    records_to_insert.emplace_back(record_to_insert);
  }

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("Failed to get next tuple from child operator, rc = %s", strrc(rc));
    return rc;
  }

  rc = child->close();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to close child operator, rc = %s", strrc(rc));
    return rc;
  }

  // 删除所有要更新的记录
  for (auto &reocrd : records_to_delete) {
    rc = trx->delete_record(table_, *reocrd);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to delete record, rc = %s", strrc(rc));
      return rc;
    }
  }

  // 插入所有更新后的记录
  for (auto &record : records_to_insert) {
    rc = trx->insert_record(table_, *record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("Failed to insert record, rc = %s", strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}

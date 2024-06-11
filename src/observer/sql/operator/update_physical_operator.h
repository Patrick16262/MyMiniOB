#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/operator/update_record_maker.h"
#include "storage/trx/trx.h"
#include <cassert>
#include <vector>

class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table, const std::vector<Field> &fields) : table_(table)
  {
    update_record_maker_.init(table, fields);
  }
  ~UpdatePhysicalOperator() = default;

  void set_new_values(const vector<Value> &new_values) { new_values_ = new_values; }

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }

  virtual RC open(Trx *trx) override;
  virtual RC next() override { return RC::RECORD_EOF; }
  virtual RC close() override { return RC::SUCCESS; }

  virtual Tuple *current_tuple() override
  {
    assert(false);
    return nullptr;
  };

private:
  UpdateRecordMaker update_record_maker_;
  vector<Value>     new_values_;
  Table            *table_;
};

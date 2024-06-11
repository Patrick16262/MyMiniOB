#include "sql/operator/update_record_maker.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "storage/field/field_meta.h"
#include <cstring>
#include <memory>
#include <vector>

using namespace std;

RC UpdateRecordMaker::init(Table *table, const std::vector<Field> &fields)
{
  this->~UpdateRecordMaker();
  new (this) UpdateRecordMaker();

  int normal_field_start = table->table_meta().sys_field_num();

  for (int i = normal_field_start; i < table->table_meta().field_num(); i++) {
    const FieldMeta &cur_field_meta = table->table_meta().field_metas()->at(i);
    table_field_epxrs_.push_back(std::make_unique<FieldExpr>(table, &cur_field_meta));

    for (int j = 0; j < fields.size(); j++) {
      if (!strcmp(fields[j].field_name(), cur_field_meta.name())) {
        update_field_order_.push_back(i);
        update_field_types_.push_back(cur_field_meta.type());
        break;
      }
    }
  }

  table_ = table;

  return RC::SUCCESS;
}

RC UpdateRecordMaker::update(const Tuple *tuple, std::vector<Value> new_values, Record &record)
{
  vector<Value> values;
  RC            rc;

  if (new_values.size() != update_field_order_.size()) {
    LOG_ERROR("The number of new values is not equal to the number of fields to be updated");
    return RC::INVALID_ARGUMENT;
  }

  int normal_field_start = table_->table_meta().sys_field_num();

  for (int i = normal_field_start; i <  normal_field_start + table_field_epxrs_.size(); i++) {
    Value tmp;
    bool  found_in_new_values = false;

    for (int j = 0; j < update_field_order_.size(); j++) {
      if (update_field_order_[j] == i) {
        tmp                 = new_values[j];
        found_in_new_values = true;
        values.push_back(tmp);
        break;
      }
    }

    if (!found_in_new_values) {
      rc = table_field_epxrs_[i - normal_field_start]->get_value(*tuple, tmp);
      if (rc != RC::SUCCESS) {
        return rc;
      }

      values.push_back(tmp);
    }
  }

  rc = table_->make_record(values.size(), values.data(), record);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to make record for update, rc = %s", strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
}
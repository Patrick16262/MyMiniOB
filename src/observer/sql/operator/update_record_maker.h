#pragma once

#include "sql/expr/expression.h"
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include "storage/table/table.h"
#include <memory>
#include <vector>

class UpdateRecordMaker
{
public:
  UpdateRecordMaker() {};

  RC init(Table *table,const  std::vector<Field>& fields);

  RC update(const Tuple *tuple, std::vector<Value> new_values, Record &record);

private:
  std::vector<std::unique_ptr<FieldExpr>> table_field_epxrs_;
  std::vector<AttrType>                   update_field_types_;
  std::vector<int>                        update_field_order_;
  Table                                  *table_;
};
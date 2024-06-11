#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/stmt/table_ref_desc.h"
#include <memory>
#include <utility>
#include <vector>

class GroupLogicalOperator : public LogicalOperator
{
public:
  GroupLogicalOperator( std::vector<std::unique_ptr<AggregateDesc>> descs) : aggr_descs_(std::move(descs)) {}

  virtual LogicalOperatorType type() const override { return LogicalOperatorType::GROUP; }

   std::vector<std::unique_ptr<AggregateDesc>> &aggr_descs()  { return aggr_descs_; }

private:
  std::vector<std::unique_ptr<AggregateDesc>> aggr_descs_;
};
#pragma once

#include "sql/operator/logical_operator.h"

class GroupLogicalOperator : public LogicalOperator
{
public:
  GroupLogicalOperator(std::vector<std::unique_ptr<Expression>> &aggr_exprs, std::vector<AggregateType> &aggr_types,
      std::vector<TupleCellSpec> &aggr_specs)
      : aggr_exprs_(std::move(aggr_exprs)), aggr_types_(std::move(aggr_types)), aggr_specs_(std::move(aggr_specs))
  {}
  virtual LogicalOperatorType type() const override { return LogicalOperatorType::GROUP; }

private:
  const std::vector<std::unique_ptr<Expression>> aggr_exprs_;
  const std::vector<AggregateType>               aggr_types_;
  const std::vector<TupleCellSpec>               aggr_specs_;
};
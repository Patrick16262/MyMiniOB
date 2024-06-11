#pragma once

#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "sql/stmt/table_ref_desc.h"
#include <memory>
#include <vector>

class GroupPhysicalOperator : public PhysicalOperator
{
public:
  GroupPhysicalOperator(std::vector<std::unique_ptr<AggregateDesc>> descs) : tuple_factory_(std::move(descs))
  {}

  PhysicalOperatorType type() const override { return PhysicalOperatorType::GROUP; };

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

private:
  AggregateTupleManager       tuple_factory_;
  std::vector<AggregateTuple> tuples_;
  int                         current_tuple_index_ = -1;
  std::vector<Expression *>   group_exprs;
};
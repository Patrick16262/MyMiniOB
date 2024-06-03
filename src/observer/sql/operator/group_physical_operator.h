#pragma once

#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include <utility>
#include <vector>

class GroupPhysicalOperator : public PhysicalOperator
{
public:
  GroupPhysicalOperator(AggregateTupleManager tuple_factory) : tuple_factory_(std::move(tuple_factory)) {
    group_exprs = tuple_factory_.group_exprs();
  }
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
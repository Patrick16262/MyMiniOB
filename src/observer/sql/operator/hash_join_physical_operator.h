
/**
 * @brief
 *
 */

#pragma once

#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "sql/parser/value.h"
#include <memory>
#include <unordered_map>

/**
 * @brief hashjoin算子
 * @ingroup PhysicalOperator
 * @details 当前将左表的内容全部缓存在内存，当数据量较大时会出问题.
 *          同时，当时只有在expr(left_field) = expr(right_field)时才能使用hashjoin
 */

class HashJoinPhysicalOperator : public PhysicalOperator
{
public:
  virtual PhysicalOperatorType type() const override { return PhysicalOperatorType::HASH_JOIN; };

  virtual RC open(Trx *trx) override;
  virtual RC next() override;
  virtual RC close() override;

  virtual Tuple *current_tuple() override;

private:
  RC generate_hash_table();
  RC fetch_next_tuple_in_table();
  RC fetch_next_tuple_in_range();

private:
  PhysicalOperator                                      *left_table_;
  PhysicalOperator                                      *right_table_;
  std::unordered_multimap<Value, std::unique_ptr<Tuple>> left_hash_table_;
  decltype(left_hash_table_.equal_range(Value()))        eq_range_it_;
  std::unique_ptr<Expression>                            left_expr_;
  std::unique_ptr<Expression>                            right_expr_;
  CacheTupleManager                                      cache_manager_;
  Trx                                                   *trx_;
  JoinedTuple                                            current_tuple_;
};
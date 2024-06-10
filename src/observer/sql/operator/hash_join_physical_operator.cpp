#include "hash_join_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "sql/parser/value.h"
#include <cassert>
#include <memory>
#include <utility>

using namespace std;

RC RbTreeJoinPhysicalOperator::open(Trx *trx)
{
  assert(children_.size() == 2);

  RC rc;
  eq_range_it_ = {left_hash_table_.end(), left_hash_table_.end()};
  trx_         = trx;
  left_table_  = children_[0].get();
  right_table_ = children_[1].get();

  if ((rc = right_table_->open(trx)) != RC::SUCCESS) {
    LOG_WARN("open right table failed");
    return rc;
  }

  if ((rc = generate_hash_table()) != RC::SUCCESS) {
    LOG_WARN("generate hash table failed");
    return rc;
  }

  return RC::SUCCESS;
}

RC RbTreeJoinPhysicalOperator::next()
{
  RC rc;

  if ((rc = fetch_next_tuple_in_range()) == RC::SUCCESS) {
    return rc;
  }
  if (rc != RC::RECORD_EOF) {
    LOG_WARN("get next tuple in range failed");
    return rc;
  }

  if ((rc = fetch_next_tuple_in_table()) == RC::SUCCESS) {
    return rc;
  }
  if (rc != RC::RECORD_EOF) {
    LOG_WARN("get next tuple in table failed");
    return rc;
  }

  return RC::RECORD_EOF;
}

RC RbTreeJoinPhysicalOperator::close() {
  RC rc;

  if ((rc = left_table_->close()) != RC::SUCCESS) {
    LOG_WARN("close left table failed");
    return rc;
  }

  if ((rc = right_table_->close()) != RC::SUCCESS) {
    LOG_WARN("close right table failed");
    return rc;
  }

  left_hash_table_.clear();
  return RC::SUCCESS;
}

Tuple *RbTreeJoinPhysicalOperator::current_tuple() {
  return &current_tuple_;
}

RC RbTreeJoinPhysicalOperator::generate_hash_table()
{
  Value             value;
  unique_ptr<Tuple> cache;
  RC                rc;

  rc = left_table_->open(trx_);
  if (rc != RC::SUCCESS) {
    LOG_WARN("open left table failed");
    return rc;
  }

  while ((rc = left_table_->next()) == RC::SUCCESS) {
    cache_manager_.cloneTuple(*left_table_->current_tuple(), cache);

    rc = left_expr_->get_value(*cache, value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("get value failed");
      break;
    }

    left_hash_table_.insert({value, std::move(cache)});
  }

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("next left table failed");
    return rc;
  }

  return RC::SUCCESS;
}

RC RbTreeJoinPhysicalOperator::fetch_next_tuple_in_table()
{
  RC rc;
  if ((rc = right_table_->next()) != RC::SUCCESS) {
    return rc;
  }

  Value value;
  rc = right_expr_->get_value(*right_table_->current_tuple(), value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("get value failed");
    return rc;
  }

  eq_range_it_ = left_hash_table_.equal_range(value);
  current_tuple_.set_left(eq_range_it_.first->second.get());
  current_tuple_.set_right(right_table_->current_tuple());

  return RC::SUCCESS;
}

RC RbTreeJoinPhysicalOperator::fetch_next_tuple_in_range()
{
  eq_range_it_.first++;

  if (eq_range_it_.first == eq_range_it_.second) {
    return RC::RECORD_EOF;
  }

  current_tuple_.set_left(eq_range_it_.first->second.get());
  current_tuple_.set_right(right_table_->current_tuple());

  return RC::SUCCESS;
}

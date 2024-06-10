#include "group_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "sql/parser/value.h"
#include <cassert>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

RC GroupPhysicalOperator::open(Trx *trx)
{
  assert(children_.size() == 1);
  map<vector<Value>, AggregateTuple> grouped_tuples;
  vector<Value>                                current_group_by_values;
  Tuple                                       *tuple = nullptr;
  PhysicalOperator                            *child = children_[0].get();
  RC                                           rc;

  rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to open child operator rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  while ((rc = child->next()) == RC::SUCCESS) {
    tuple = child->current_tuple();
    Value tmp;
    for (auto &expr : group_exprs) {
      rc = expr->get_value(*tuple, tmp);
      if (rc != RC::SUCCESS) {
        LOG_WARN("Failed to get value from expression rc=%d:%s", rc, strrc(rc));
        return rc;
      }

      current_group_by_values.push_back(tmp);
    }

    auto it = grouped_tuples.find(current_group_by_values);
    if (it == grouped_tuples.end()) {
      grouped_tuples.insert({std::move(current_group_by_values), tuple_factory_.generateTuple(current_group_by_values)});
      it = grouped_tuples.find(current_group_by_values);
      assert(it != grouped_tuples.end());
    }

    rc = it->second.aggregate_tuple(*tuple);
    if (rc != RC::SUCCESS) {
      string str = tuple->to_string();
      LOG_WARN("Failed to aggregate tuple rc=%d:%s, tuple=%s", rc, strrc(rc), str.c_str());
      return rc;
    }
  }

  if (rc != RC::RECORD_EOF) {
    LOG_WARN("Failed to get next tuple rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  for (auto &it : grouped_tuples) {
    tuples_.push_back(std::move(it.second));
  }

  rc = child->close();
  if (rc != RC::SUCCESS) {
    LOG_WARN("Failed to close child operator rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
}

RC GroupPhysicalOperator::next()
{
  if (current_tuple_index_ >= tuples_.size()) {
    return RC::RECORD_EOF;
  }

  current_tuple_index_++;
  return RC::SUCCESS;
}

RC GroupPhysicalOperator::close()
{
  tuples_.clear();
  current_tuple_index_ = -1;
  return RC::SUCCESS;
}

Tuple *GroupPhysicalOperator::current_tuple() {
  if (current_tuple_index_ < 0 || current_tuple_index_ >= tuples_.size()) {
    return nullptr;
  }

  return &tuples_[current_tuple_index_];
}

//
// Created by root on 5/5/24.
//

#pragma once

#include <iostream>
#include "types.h"

namespace common::date {
  std::string format(sql_date date);
  sql_date parse(std::string str);
  bool is_validate(sql_date date);
  int get_timestamp(sql_date date);
}


//
// Created by root on 5/6/24.
//

#pragma once

#include "storage/record/record.h"

Record *generate_expanded_data(const char *data, int length, int sep_len);
void move_to_last(Record& record, Record* record_list);

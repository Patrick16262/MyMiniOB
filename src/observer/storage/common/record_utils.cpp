//
// Created by root on 5/6/24.
//

#include "record_utils.h"

/* nullable */
Record *generate_expanded_data(const char *data, int length, int sep_len) {
    using namespace std;
    assert(sep_len > 0);

    Record *res = nullptr;
    int cur_offset = 0;
    while (cur_offset < length) {
        char *cur_data = new char[sep_len];
        int copy_len = min(sep_len, length - sep_len);
        Record *cur_record = new Record;
        memcpy(cur_data + cur_offset, data , copy_len);
        cur_record->set_data_owner(cur_data, sep_len);
        if (res == nullptr) {
            res = cur_record;
        } else {
            move_to_last(*res, cur_record);
        }
        cur_offset += sep_len;
    }
    return res;
}

void move_to_last(Record &record, Record *record_list) {
    Record *cur = &record;
    while (cur->next_record() != nullptr) {
        cur = cur->next_record();
    }
    cur->move_as_near(*record_list);
}

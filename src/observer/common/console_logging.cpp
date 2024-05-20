//
// Created by root on 5/14/24.
//

#include "console_logging.h"
#include <deque>

using namespace std;

namespace common::logging {
    deque<string> traces;
}



string common::logging::simplify_filename(const char *file_name) {
    string file_name_str = file_name;
    size_t pos = file_name_str.find_last_of('/');
    if (pos != string::npos) {
        file_name_str = file_name_str.substr(pos + 1);
    }
    return file_name_str;
}

string
common::logging::make_log_string(const char *file_name, int line, const char *log_level, const char *formatted_message) {

    char *buf = new char[4096];
    sprintf(buf, "[%s:%d] %s: ", file_name, line, log_level);
    string res = buf;
    res += formatted_message;
    delete[] buf;
    return res;
}

void common::logging::print_trace(int log_level) {
    if (!traces.empty()) {
        print_log(log_level, "Trace (most recently):");
    }
    while (!traces.empty()) {
        print_log(log_level, traces.front().c_str());
        traces.pop_front();
    }
}

void common::logging::print_log(int log_level, const char *log_str) {
    const char *base_color;
    switch (log_level) {
        case 3:
            base_color = "\033[1;33m";
            break;
        case 4:
            base_color = "\033[1;31m";
            break;
        default:
            base_color = "\033[1;0m";
            break;
    }
    printf("%s%s\033[0m\n", base_color, log_str);
}

void common::logging::save_trace(const char *log_str) {
    traces.emplace_back(log_str);
    if (traces.size() > MAX_TRACE_LOG_SIZE) {
        traces.pop_front();
    }
}

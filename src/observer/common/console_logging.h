//
// Created by root on 5/14/24.
//

#pragma once

#include "string"
#include <cstring>
#include "iostream"
#include "deque"


#ifdef DEBUG

namespace common::logging {
    using namespace std;

    extern deque<string> traces;

#define MAX_TRACE_LOG_SIZE 64

    string simplify_filename(const char *file_name);

    string make_log_string(const char *file_name,
                           int line,
                           const char *log_level,
                           const char *formatted_message);

    void save_trace(const char *log_str);

    void print_log(int log_level, const char *log_str);

    void print_trace(int log_level);

}

#define FORMAT_STRING_INTERNAL(...)[&](){ \
using namespace common::logging;\
char buf_PATRICKSHAOPLACEHOLDER[4096]; \
sprintf(buf_PATRICKSHAOPLACEHOLDER, __VA_ARGS__);      \
std::string res = buf_PATRICKSHAOPLACEHOLDER;  \
return res;                     \
}() \

#define MAKE_LOG_STRING_INTERNAL(level_code, log_level, ...) [&](){ \
using namespace common::logging;\
std::string fmt_PATRICKSHAOPLACEHOLDER = FORMAT_STRING_INTERNAL(__VA_ARGS__);  \
return make_log_string(simplify_filename(__FILE__).c_str(),        \
__LINE__,                                                          \
log_level,                                                         \
fmt_PATRICKSHAOPLACEHOLDER.c_str()\
); \
}()
                                                                   \
#define STD_SAVE_TRACE_INTERNAL(level_code, level_name, ...) [&](){ \
common::logging::save_trace(MAKE_LOG_STRING_INTERNAL(level_code, level_name, __VA_ARGS__).c_str());\
}()

#define STD_CON_OUT_INTERNAL(level_code, level_name, ...) [&](){\
common::logging::print_log(level_code, MAKE_LOG_STRING_INTERNAL(level_code, level_name, __VA_ARGS__).c_str());\
}()

#define STD_CON_OUT_ERROR_INTERNAL(level_code, level_name, ...) [&](){ \
STD_CON_OUT_INTERNAL(level_code, level_name, __VA_ARGS__);             \
common::logging::print_trace(level_code - 1);\
}()

#else
#define STD_TRACE(...)
#define STD_DEBUG(...)
#define STD_INFO(...)
#define STD_WARN(...)
#define STD_ERROR(...)
#endif

#define STD_TRACE(...) STD_SAVE_TRACE_INTERNAL(0, "TRACE", __VA_ARGS__)
#define STD_DEBUG(...) STD_SAVE_TRACE_INTERNAL(1, "DEBUG", __VA_ARGS__)
#define STD_INFO(...) STD_CON_OUT_INTERNAL(2, "INFO", __VA_ARGS__)
#define STD_WARN(...) STD_CON_OUT_INTERNAL(3, "WARN", __VA_ARGS__)
#define STD_ERROR(...) STD_CON_OUT_ERROR_INTERNAL(4, "ERROR", __VA_ARGS__)

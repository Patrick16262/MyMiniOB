//
// Created by root on 5/15/24.
//
#include <mutex>
#include <cassert>
#include "backward.hpp"
#include "unordered_map"
#include <dlfcn.h>
#include "mem_detection.h"


using sep_t = unsigned long long int;

#define SEP_SIZE 8
#define SEP_VALUE 0xdeadbeefdeadbeef
#define APPLICATION_DATA_SIZE (1024 * 1024 * 512)

using namespace std;
using namespace backward;


struct allocate_info {
    size_t size;
    StackTrace st;
    sep_t *start_sep;
    sep_t *end_sep;
};

void print_lb64(uint64_t num) {
    string res[8];
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; j++) {
            if (num & ((uint64_t) 1 << (i * 8 + j))) {
                res[i].insert(0, "1");
            } else {
                res[i].insert(0, "0");
            }
        }
    }
    for (int i = 0; i < 8; ++i) {
        printf("%s ", res[i].c_str());
    }
}

class OverflowDetectPool {
public:

    void *allocate(size_t size) {
        if (size + available_ > data_ + APPLICATION_DATA_SIZE) {
            printf("waring: memory pool full\n");
            return nullptr;
        }
        void *res = available_ + SEP_SIZE;
        allocated_[res] = {size,
                           StackTrace(),
                           (sep_t *) available_,
                           (sep_t *) (available_ + size + SEP_SIZE)};
        allocated_[res].st.load_here();
        available_ += size + 2 * SEP_SIZE;
        *allocated_[res].start_sep = SEP_VALUE;
        *allocated_[res].end_sep = SEP_VALUE;
        return res;
    }

    bool deallocate(void *ptr) {
        auto it = allocated_.find(ptr);
        if (it == allocated_.end()) {
            return false;
        }
        check_overflow(*it);
        allocated_.erase(it);
        return true;
    }

    void enable_memory_detect() {
        enabled = true;
    }

    void check_overflow(const pair<void *, allocate_info> &pair) {
        auto &info = pair.second;
        bool is_overflow = false;
        if (*info.start_sep != SEP_VALUE) {
            is_overflow = true;

            printf("overflow detected on %p, position: left\n", pair.first);
            printf("original: ");
            print_lb64(SEP_VALUE);
            printf("\n");
            printf("current :");
            print_lb64(*info.start_sep);
            printf("\n");
        }

        if (*info.end_sep != SEP_VALUE) {
            is_overflow = true;

            printf("overflow detected on %p, position: right\n", pair.first);
            printf("original: ");
            print_lb64(SEP_VALUE);
            printf("\n");
            printf("current : ");
            print_lb64(*info.end_sep);
            printf("\n");
        }

        if (is_overflow) {
            p.print(info.st, stdout);
            printf("overflow detected\n");
        }
    }

    OverflowDetectPool() {
        p.address = true;
        p.object = true;
        p.color_mode = ColorMode::always;
        data_ = new char[APPLICATION_DATA_SIZE];
        available_ = data_;
    }

    bool is_enabled() const {
        return enabled;
    }


private:
    Printer p;
    unordered_map<void *, allocate_info> allocated_;
    char *data_ = nullptr;
    char *available_ = nullptr;
    bool enabled = false;
};

static OverflowDetectPool pool;


typedef void *(*malloc_t)(size_t size);

static malloc_t malloc_f = NULL;

typedef void (*free_t)(void *p);

static free_t free_f = NULL;

//要hook的同名函数
void *malloc(size_t size) {
    if (malloc_f == NULL) {
        malloc_f = (malloc_t) dlsym(RTLD_NEXT, "malloc"); //除了RTLD_NEXT 还有一个参数RTLD_DEFAULT
    }
    static bool recursive = false;
    if (!recursive && pool.is_enabled()) {
        recursive = true;
        void *res = pool.allocate(size);
        if (res != nullptr) {
            return res;
        }
        recursive = false;
    }
    return malloc_f(size);
}

void free(void *p) {
    if (free_f == NULL) {
        free_f = (free_t) dlsym(RTLD_NEXT, "free");
    }
    static bool recursive = false;
    if (!recursive && pool.is_enabled()) {
        recursive = true;
        if (pool.deallocate(p)) {
            recursive = false;
            return;
        }
        recursive = false;
    }
    free_f(p);
}

void enable_mem_debug() {
    pool.enable_memory_detect();
}
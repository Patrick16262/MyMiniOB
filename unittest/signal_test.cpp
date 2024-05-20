//
// Created by root on 5/15/24.
//
#include "gtest/gtest.h"
#include <malloc.h>
#include "backward.hpp"

backward::SignalHandling sh;

void dofree(void *t) {
    free(t);
}

void delete_stack_memory() { //no signal
    char p[100];
    dofree(p);
}

void division_by_zero() { //pass
    int a = 1;
    int b = 0;
    int c = a / b;
    printf("%d",c);
}

void stack_smashing() { //no signal
    char im_a_samll_char_arr[1];
    char *b = im_a_samll_char_arr;
    strcpy(b, "Im a big string");
}
void heap_overflow() { //no signal
    int *a = (int *)malloc(10 * sizeof(int));
    for (int i = 0; i < 11; i++) {
        a[i] = i;
    }
//    free(a);
}

void termination()  {//pass
    using namespace std;
    terminate();
}


TEST(signaltest, test1) {
    heap_overflow();
}
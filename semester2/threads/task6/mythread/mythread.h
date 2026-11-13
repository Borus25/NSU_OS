#pragma once
#include <stddef.h>

typedef struct mythread {
    int tid;
    void* mythread_stack;
    size_t stack_size;

    void* (*mythread_func)(void *);
    void* arg;
    void* retval; // for exit from start_routine
} mythread_t;

int mythread_create(mythread_t* tid, void *(*start_routine)(void *), void *arg);
int mythread_destroy(mythread_t *tid);



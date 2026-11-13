#ifndef UTHREAD_H
#define UTHREAD_H

#include <ucontext.h>

typedef struct uthread {
    ucontext_t context;
    void* stack;

    void* (*start_routine)(void*);
    void* arg;

    int finished;
} uthread_t;

int uthread_create(uthread_t* thread, void* (*start_routine)(void*), void* arg);

void uthread_yield(void);

#endif
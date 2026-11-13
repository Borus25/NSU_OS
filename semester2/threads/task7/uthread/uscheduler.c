#include "uscheduler.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 16

static ucontext_t scheduler_ctx;
static uthread_t* threads[MAX_THREADS];

static size_t thread_count;
static uthread_t* current;

int scheduler_add(uthread_t *thread) {
    if (thread_count == MAX_THREADS) {
        errno = EAGAIN;
        return -1;
    }

    threads[thread_count++] = thread;
    return 0;
}

uthread_t* scheduler_current(void) {
    return current;
}

ucontext_t* scheduler_context(void) {
    return &scheduler_ctx;
}

void scheduler_run(void) {
    for (;;) {
        int has_ready = 0;

        for (size_t i = 0; i < thread_count; i++) {
            uthread_t *thread = threads[i];

            if (thread->finished) {
                continue;
            }

            has_ready = 1;
            current = thread;

            if (swapcontext(&scheduler_ctx, &thread->context) == -1) {
                perror("swapcontext");
                abort();
            }

            current = NULL;

            if (thread->finished) {
                free(thread->stack);
                thread->stack = NULL;
            }
        }

        if (!has_ready)
            break;
    }

    thread_count = 0;
}
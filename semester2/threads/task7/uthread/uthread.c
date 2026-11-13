#include "uthread.h"
#include "uscheduler.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 64 * 1024

void uthread_entry(void) {
    uthread_t *thread = scheduler_current();

    thread->start_routine(thread->arg);
    thread->finished = 1;
}

int uthread_create(uthread_t* thread, void* (*start_routine) (void*), void* arg) {
    if (thread == NULL || start_routine == NULL) {
        errno = EINVAL;
        return -1;
    }

    thread->stack = malloc(STACK_SIZE);
    if (thread->stack == NULL) {
        return -1;
    }

    if (getcontext(&thread->context) == -1) {
        int error = errno;
        free(thread->stack);
        thread->stack = NULL;
        errno = error;
        return -1;
    }

    thread->start_routine = start_routine;
    thread->arg = arg;
    thread->finished = 0;

    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = STACK_SIZE;
    thread->context.uc_stack.ss_flags = 0;
    thread->context.uc_link = scheduler_context();

    makecontext(&thread->context, uthread_entry, 0);

    if (scheduler_add(thread) == -1) {
        int error = errno;
        free(thread->stack);
        thread->stack = NULL;
        errno = error;
        return -1;
    }

    return 0;
}

void uthread_yield(void) {
    uthread_t *thread = scheduler_current();
    if (thread == NULL) {
        return;
    }

    if (swapcontext(&thread->context, scheduler_context()) == -1) {
        perror("swapcontext");
        abort();
    }
}
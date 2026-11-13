#define _GNU_SOURCE
#include "mythread.h"

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <linux/futex.h>

#define STACK_SIZE 8392704

typedef struct {
    void   *mapping;
    size_t  mapping_size;

    void   *stack_bottom;
    size_t  stack_size;
    void   *stack_top;
} mythread_stack_t;

int mythread_stack_alloc(mythread_stack_t *out, size_t requested_size) {
    if (out == NULL || requested_size == 0) {
        return EINVAL;
    }

    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) {
        return EINVAL;
    }

    size_t page_size = (size_t)page;

    if (requested_size > SIZE_MAX - (page_size - 1)) {
        return EOVERFLOW;
    }

    size_t stack_size = ((requested_size + page_size - 1) / page_size) * page_size;

    if (stack_size > SIZE_MAX - page_size) {
        return EOVERFLOW;
    }

    size_t mapping_size = page_size + stack_size;

    void *mapping = mmap(NULL,mapping_size,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK,-1,0);
    if (mapping == MAP_FAILED) {
        return errno;
    }

    char* stack_bottom = (char*)mapping + page_size;

    if (mprotect(stack_bottom, stack_size,PROT_READ | PROT_WRITE) == -1) {
        int error = errno;
        munmap(mapping, mapping_size);
        return error;
    }

    *out = (mythread_stack_t) {
        .mapping      = mapping,
        .mapping_size = mapping_size,
        .stack_bottom = stack_bottom,
        .stack_size   = stack_size,
        .stack_top    = stack_bottom + stack_size
    };

    return 0;
}

int mythread_entry(void *arg) {
    mythread_t *thread = arg;
    thread->retval = thread->mythread_func(thread->arg);
    return 0;
}

int mythread_create(mythread_t *tid, void *(*start_routine)(void *), void *arg) {
    if (tid == NULL || start_routine == NULL) {
        return EINVAL;
    }

    *tid = (mythread_t) {
        .tid = 0,
        .mythread_stack = NULL,
        .stack_size = 0,
        .mythread_func = start_routine,
        .arg = arg,
        .retval = NULL
    };

    mythread_stack_t stack;
    int error = mythread_stack_alloc(&stack, 1024 * 1024);
    if (error != 0) {
        return error;
    }

    tid->mythread_stack = stack.mapping;
    tid->stack_size = stack.mapping_size;

    int clone_flags = CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID;

    int tid_local = clone(mythread_entry, stack.stack_top, clone_flags, tid, &tid->tid, NULL, &tid->tid);
    if (tid_local == -1) {
        munmap(stack.mapping, stack.mapping_size);
        error = errno;
        return error;
    }

    return 0;
}

int mythread_destroy(mythread_t *tid) {
    if (tid == NULL || tid->mythread_stack == NULL) {
        return EINVAL;
    }

    long self_tid = syscall(SYS_gettid);

    for (;;) {
        int tid_local = __atomic_load_n(&tid->tid,__ATOMIC_ACQUIRE);
        if (tid_local == 0)
            break;

        if ((long)tid_local == self_tid) {
            return EDEADLK;
        }

        long rc = syscall(SYS_futex, &tid->tid, FUTEX_WAIT, tid_local, NULL, NULL, 0);
        if (rc == -1) {
            int error = errno;

            if (error == EAGAIN || error == EINTR) {
                continue;
            }

            return error;
        }
    }

    if (munmap(tid->mythread_stack, tid->stack_size) == -1) {
        return errno;
    }

    tid->mythread_stack = NULL;
    tid->stack_size = 0;

    printf("mythread_destroy: tid=%p\n successfully", tid);

    return 0;
}

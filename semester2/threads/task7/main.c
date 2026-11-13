#include <stdio.h>
#include <stdlib.h>

#include "uthread/uthread.h"
#include "uthread/uscheduler.h"

void* uthread_func(void *arg) {
    const char *name = arg;

    for (int i = 0; i < 3; ++i) {
        printf("%s: %d\n", name, i);
        uthread_yield();
    }

    return NULL;
}

int main(void) {
    uthread_t first;
    uthread_t second;

    if (uthread_create(&first, uthread_func, "U1") == -1) {
        perror("uthread_create");
        return EXIT_FAILURE;
    }

    if (uthread_create(&second, uthread_func, "U2") == -1) {
        perror("uthread_create");
        scheduler_run();
        return EXIT_FAILURE;
    }

    scheduler_run();

    printf("main: done");
    return EXIT_SUCCESS;
}
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mythread/mythread.h"

void *thread_func(void *arg) {
    printf("mythread [%d %d %d]: Hello from custom mythread!\n", getpid(), getppid(), gettid());
    return NULL;
}

int main() {
    mythread_t tid;

    printf("main: Hello world, pid %d, tid %d\n", getpid(), gettid());

    int error = mythread_create(&tid, thread_func, NULL);
    if (error != 0) {
        fprintf(stderr, "create: %s\n", strerror(error));
        return 1;
    }

    error = mythread_destroy(&tid);
    if (error != 0) {
        fprintf(stderr, "destroy: %s\n", strerror(error));
        return 1;
    }

    return 0;
}
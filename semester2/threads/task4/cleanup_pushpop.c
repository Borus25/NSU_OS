#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cleanup(void *arg) {
    free(arg);
    printf("cleanup: success\n");
}

void *thread_func(void* arg) {
    char* str = malloc(20);
    strcpy(str, "Hello world");

    // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(cleanup, str);

    for (;;) {
        printf("%s\n", str);
    }

    pthread_cleanup_pop(1);

}

int main() {
    int err;
    pthread_t tid;

    printf("main: Hello world, pid %d, tid %d\n", getpid(), gettid());

    err = pthread_create(&tid, NULL, thread_func, NULL);
    if (err != 0) {
        printf("ERROR: pthread_create() failed: %d\n", err);
    }

    sleep(1);

    pthread_cancel(tid);

    void * result;
    err = pthread_join(tid, &result);
    if (err != 0) {
        printf("ERROR: pthread_join() failed: %d\n", err);
    }

    if (result == PTHREAD_CANCELED) {
        printf("main: thread canceled");
    } else {
        printf("main: thread finished normally");
    }

    pthread_exit(NULL);
}

// #define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct thread_arg {
    int number;
    char *text;
};

void check(int err, const char *name) {
    if (err) {
        fprintf(stderr, "%s failed: %s\n", name, strerror(err));
        exit(EXIT_FAILURE);
    }
}

void *mythread(void *arg) {
    struct thread_arg *data = arg;

    printf("mythread: number=%d, text=%s\n",
           data->number, data->text);

    return NULL;
}

int main(void) {
    pthread_t tid;

    struct thread_arg data = {
        .number = 42,
        .text = "hello world"
    };

    check(pthread_create(&tid, NULL, mythread, &data), "main: pthread_create()");

    check(pthread_join(tid, NULL), "main: pthread_join()");

    return 0;
}
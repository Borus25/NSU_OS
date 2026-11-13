// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct thread_arg {
    int number;
    char *text;
};

void check(int err, const char *name) {
    if (err) {
        printf("main: %s() failed: %s\n", name, strerror(err));
        exit(EXIT_FAILURE);
    }
}

void *mythread(void *arg) {
    struct thread_arg *data = arg;

    printf("mythread: number=%d, text=%s\n",
           data->number, data->text);

    free(data);

    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int err;

    struct thread_arg *data = malloc(sizeof(*data));
    if (data == NULL) {
        perror("malloc");
        return 1;
    }

    data->number = 42;
    data->text = "hello world";

    check(pthread_attr_init(&attr), "pthread_attr_init");

    check(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED),
        "pthread_attr_setdetachstate");

    err = pthread_create(&tid, &attr, mythread, data);
    if (err) {
        free(data);
    }

    check(pthread_attr_destroy(&attr), "pthread_attr_destroy");
    check(err, "pthread_create");

    pthread_exit(NULL);
}
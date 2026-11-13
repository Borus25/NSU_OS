#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

static void check(int err, const char *name) {
    if (err) {
        fprintf(stderr, "%s failed: %s\n", name, strerror(err));
        exit(EXIT_FAILURE);
    }
}

static void cleanup(void *arg) {
    free(arg);
    puts("cleanup: string freed");
}

void *mythread(void *arg) {
    const char *mode = arg;

    if (strcmp(mode, "a") == 0) {
        while (1) {
            puts("mythread: hello world");
            pthread_testcancel();
        }
    }

    if (strcmp(mode, "b") == 0) {
        volatile unsigned long counter = 0;

        while (1) {
            counter++;
        }
    }

    if (strcmp(mode, "b-fixed") == 0) {
        unsigned long counter = 0;

        while (1) {
            counter++;
            pthread_testcancel();
        }
    }

    if (strcmp(mode, "c") == 0) {
        char *string = malloc(sizeof("hello world"));

        if (string == NULL) {
            perror("malloc");
            return NULL;
        }

        strcpy(string, "hello world");

        pthread_cleanup_push(cleanup, string);

        while (1) {
            puts(string);
            pthread_testcancel();
        }

        pthread_cleanup_pop(1);
    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tid;
    void *result = NULL;
    int err;

    if (argc != 2 ||
        (strcmp(argv[1], "a") != 0 &&
         strcmp(argv[1], "b") != 0 &&
         strcmp(argv[1], "b-fixed") != 0 &&
         strcmp(argv[1], "c") != 0)) {
        fprintf(stderr, "Usage: %s a|b|b-fixed|c\n", argv[0]);
        return EXIT_FAILURE;
    }

    err = pthread_create(&tid, NULL, mythread, argv[1]);
    check(err, "pthread_create");

    sleep(1);

    err = pthread_cancel(tid);
    check(err, "pthread_cancel");

    puts("main: cancellation requested");

    err = pthread_join(tid, &result);
    check(err, "pthread_join");

    if (result == PTHREAD_CANCELED) {
        puts("main: thread canceled");
    } else {
        puts("main: thread finished normally");
    }

    return 0;
}
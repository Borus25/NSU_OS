#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

static char mode;

void check(int err, const char *name) {
    if (err) {
        fprintf(stderr, "%s failed: %s\n", name, strerror(err));
        exit(EXIT_FAILURE);
    }
}

void *mythread(void *arg) {

    if (mode == 'e') {
        check(pthread_detach(pthread_self()), "pthread_detach");
    }

    printf("mythread [%d %d %d]: Hello from mythread!\n",
           getpid(), getppid(), gettid());

    if (mode == 'b') {
        // int local_int = 42;
        int* local_int = malloc(sizeof(int));
        *local_int = 42;
        return local_int;
    }

    if (mode == 'c') {
        char string[] = "hello world";
        return string;
    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tid;
    pthread_attr_t attr;
    void *result;
    int err;

    if (argc != 2 || strlen(argv[1]) != 1 || argv[1][0] < 'a' || argv[1][0] > 'f') {
        fprintf(stderr, "Usage: %s a|b|c|d|e|f\n", argv[0]);
        return EXIT_FAILURE;
    }

    mode = argv[1][0];

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    if (mode <= 'c') {
        err = pthread_create(&tid, NULL, mythread, NULL);
        check(err, "pthread_create");

        check(pthread_join(tid, &result), "pthread_join");

        if (mode == 'b') {
            printf("main: result = %d\n", *(int*)result);
        }

        if (mode == 'c') {
            printf("main: result = %s\n", (char*)result);
        }

        return 0;
    }

    if (mode == 'f') {
        check(pthread_attr_init(&attr), "pthread_attr_init");

        check(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED),
            "pthread_attr_setdetachstate");
    }

    for (;;) {
        err = pthread_create(&tid, mode == 'f' ? &attr : NULL, mythread, NULL);

        if (err) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(err));

            if (mode == 'f') {
                check(pthread_attr_destroy(&attr), "pthread_attr_destroy");
            }

            return EXIT_FAILURE;
        }
    }
}
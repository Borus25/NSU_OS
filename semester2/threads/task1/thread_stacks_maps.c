#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_stack(const char *name)
{
    pthread_attr_t attr;
    void *base;
    size_t size;
    size_t guard;

    int error = pthread_getattr_np(pthread_self(), &attr);
    if (error != 0) {
        fprintf(stderr, "pthread_getattr_np: %s\n", strerror(error));
        return;
    }

    error = pthread_attr_getstack(&attr, &base, &size);
    if (error != 0) {
        fprintf(stderr, "pthread_attr_getstack: %s\n", strerror(error));
        pthread_attr_destroy(&attr);
        return;
    }

    error = pthread_attr_getguardsize(&attr, &guard);
    if (error != 0) {
        fprintf(stderr, "pthread_attr_getguardsize: %s\n", strerror(error));
        pthread_attr_destroy(&attr);
        return;
    }

    printf(
        "%s: stack=[%p, %p), size=%zu, guard=%zu\n",
        name,
        base,
        (void *)((char *)base + size),
        size,
        guard
    );

    pthread_attr_destroy(&attr);
}

static void *worker(void *arg)
{
    int local;

    print_stack(arg);

    printf("worker: TID=%ld, stack address=%p\n",
           (long)gettid(), (void *)&local);

    // Не завершаем поток, чтобы можно было изучить его стек.
    for (;;) {
        pause();
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[2];
    int local;

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("main: PID=%ld, TID=%ld, stack address=%p\n",
           (long)getpid(), (long)gettid(), (void *)&local);

    int error;
    for (int i = 0; i < 2; ++i) {
        if (i == 0) {
            error = pthread_create(&threads[i], NULL, worker, "thread 0");
        } else {
            error = pthread_create(&threads[i], NULL, worker, "thread 1");
        }

        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            return EXIT_FAILURE;
        }
    }

    for (;;) {
        pause();
    }
}
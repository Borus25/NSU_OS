#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>

#define NUM_THREADS 5

void *mythread(void *arg) {
    int id = *(int *)arg;
    // pthread_t self = pthread_self();
    // pid_t pid = getpid();
    // pid_t ppid = getppid();
    // pid_t tid = gettid();

    // локальные переменные
    // int local_var = id;
    // static int local_static_var = 100;
    // const int local_const_var = 200;
    //
    // // сравнение идентификаторов, полученных через: pthread_self() и сохранённого, который получил при pthread_create()
    // extern pthread_t tids[NUM_THREADS];
    //
    // printf("[mythread %d] pid=%d, ppid=%d, pthread_self()=%lu, tids[%d]=%lu, gettid()=%d\n", id, pid, ppid, (unsigned long)self, id-1, (unsigned long)tids[id-1], tid);
    //
    // // printf("[mythread %d] pthread_equal(self, tids[%d]) = %d\n", id, id-1, pthread_equal(self, tids[id-1]));
    //
    // // адреса переменных
    // printf("[mythread %d] &local_var = %p\n", id, (void *)&local_var);
    // printf("[mythread %d] &local_static_var = %p\n", id, (void *)&local_static_var);
    // printf("[mythread %d] &local_const_var = %p\n", id, (void *)&local_const_var);
    // printf("[mythread %d] &global_var = %p\n\n", id, (void *)&global_var);

    pthread_attr_t attr;
    void *stack_addr;
    size_t stack_size;
    int stack_marker;
    int err;

    err = pthread_getattr_np(pthread_self(), &attr);
    if (err) {
        printf("mythread: pthread_getattr_np() failed: %s\n",
               strerror(err));
        return NULL;
    }

    err = pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    if (err) {
        printf("mythread: pthread_attr_getstack() failed: %s\n",
               strerror(err));

        err = pthread_attr_destroy(&attr);
        if (err) {
            printf("mythread: pthread_attr_destroy() failed: %s\n",
                   strerror(err));
        }

        return NULL;
    }

    printf("mythread [%d]: stack = [%p, %p), size = %zu bytes\n",
           gettid(),
           stack_addr,
           (void *)((char *)stack_addr + stack_size),
           stack_size);

    printf("mythread [%d]: &stack_marker = %p\n",
           gettid(), (void *)&stack_marker);

    err = pthread_attr_destroy(&attr);
    if (err) {
        printf("mythread: pthread_attr_destroy() failed: %s\n",
               strerror(err));
        return NULL;
    }

    sleep(1000);

    return NULL;
}

pthread_t tids[NUM_THREADS]; // идентификаторы потоков

int main() {
    int thread_ids[NUM_THREADS];
    int err;

    printf("[main] pid=%d, ppid=%d, pthread_self()=%lu, gettid()=%d\n", getpid(), getppid(), (unsigned long)pthread_self(), gettid());

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;

        err = pthread_create(&tids[i], NULL, mythread, &thread_ids[i]);
        if (err) {
            printf("pthread_create() failed: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }

    // жду завершения всех пяти потоков
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tids[i], NULL);
    }

    sleep(10);

    printf("[main] все 5 потоков завершили работу (pthread_join() пройден для каждого)\n");
    return EXIT_SUCCESS;
}
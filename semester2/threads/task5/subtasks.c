#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void check_error(int err, const char *function) {
    if (err) {
        fprintf(stderr, "%s failed: %s\n",
                function, strerror(err));
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(int signo) {
    static const char message[] =
        "thread 2: SIGINT received by handler\n";

    (void)signo;

    write(STDOUT_FILENO, message, sizeof(message) - 1);
}

void *thread_block_all(void *arg) {
    sigset_t signals;
    int err;

    (void)arg;

    if (sigfillset(&signals) == -1) {
        perror("thread 1: sigfillset");
        exit(EXIT_FAILURE);
    }

    err = pthread_sigmask(SIG_SETMASK, &signals, NULL);
    check_error(err, "thread 1: pthread_sigmask");

    printf("thread 1: all blockable signals are blocked\n");

    /*
     * Обычные сигналы заблокированы.
     * main завершит этот поток через pthread_cancel().
     * pause() является точкой отмены.
     */
    for (;;) {
        pause();
    }

    return NULL;
}

void *thread_sigint(void *arg) {
    sigset_t signals;
    int err;

    (void)arg;

    if (sigemptyset(&signals) == -1) {
        perror("thread 2: sigemptyset");
        exit(EXIT_FAILURE);
    }

    if (sigaddset(&signals, SIGINT) == -1) {
        perror("thread 2: sigaddset");
        exit(EXIT_FAILURE);
    }

    /*
     * Все остальные сигналы остаются заблокированными.
     * Только этот поток разблокирует SIGINT.
     */
    err = pthread_sigmask(SIG_UNBLOCK, &signals, NULL);
    check_error(err, "thread 2: pthread_sigmask");

    printf("thread 2: SIGINT is unblocked\n");

    for (;;) {
        pause();
    }

    return NULL;
}

void *thread_sigquit(void *arg) {
    sigset_t signals;
    int signo;
    int err;

    (void)arg;

    if (sigemptyset(&signals) == -1) {
        perror("thread 3: sigemptyset");
        exit(EXIT_FAILURE);
    }

    if (sigaddset(&signals, SIGQUIT) == -1) {
        perror("thread 3: sigaddset");
        exit(EXIT_FAILURE);
    }

    printf("thread 3: waiting for SIGQUIT through sigwait()\n");

    err = sigwait(&signals, &signo);
    check_error(err, "thread 3: sigwait");

    printf("thread 3: received signal %d (SIGQUIT)\n", signo);

    return NULL;
}

int main(void) {
    pthread_t tids[3];
    sigset_t signals;
    struct sigaction action = {0};
    int err;

    /*
     * Чтобы сообщения при перенаправлении не оставались
     * в буфере до завершения программы.
     */
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        fprintf(stderr, "main: setvbuf failed\n");
        return EXIT_FAILURE;
    }

    /*
     * Блокируем сигналы ДО создания потоков.
     * Создаваемые потоки наследуют маску main.
     */
    if (sigfillset(&signals) == -1) {
        perror("main: sigfillset");
        return EXIT_FAILURE;
    }

    err = pthread_sigmask(SIG_SETMASK, &signals, NULL);
    check_error(err, "main: pthread_sigmask");

    /*
     * Диспозиция SIGINT общая для всего процесса.
     * Устанавливаем обработчик до запуска потоков.
     */
    action.sa_handler = sigint_handler;
    action.sa_flags = SA_RESTART;

    if (sigemptyset(&action.sa_mask) == -1) {
        perror("main: sigemptyset");
        return EXIT_FAILURE;
    }

    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("main: sigaction");
        return EXIT_FAILURE;
    }

    printf("main: PID=%ld\n", (long)getpid());
    printf("main: SIGINT -> handler, SIGQUIT -> finish program\n");

    err = pthread_create(&tids[0], NULL, thread_block_all, NULL);
    check_error(err, "main: pthread_create(thread 1)");

    err = pthread_create(&tids[1], NULL, thread_sigint, NULL);
    check_error(err, "main: pthread_create(thread 2)");

    err = pthread_create(&tids[2], NULL, thread_sigquit, NULL);
    check_error(err, "main: pthread_create(thread 3)");

    /*
     * После SIGQUIT третий поток завершится.
     * До этого main ожидает его через join.
     */
    err = pthread_join(tids[2], NULL);
    check_error(err, "main: pthread_join(thread 3)");

    /*
     * Завершаем два оставшихся потока.
     * Обычный printf() находится вне обработчика.
     */
    for (int i = 0; i < 2; i++) {
        err = pthread_cancel(tids[i]);
        check_error(err, "main: pthread_cancel");
    }

    for (int i = 0; i < 2; i++) {
        void *result = NULL;

        err = pthread_join(tids[i], &result);
        check_error(err, "main: pthread_join");

        if (result == PTHREAD_CANCELED) {
            printf("main: thread %d was canceled\n", i + 1);
        }
    }

    printf("main: all threads finished\n");

    return 0;
}
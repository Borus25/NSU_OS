#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
	printf("mythread [%d %d %d]: Hello from custom mythread!\n", getpid(), getppid(), gettid());
	return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	err = pthread_create(&tid, NULL, mythread, NULL);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

	pthread_exit(NULL);

	return 0;
}

/*
 *Todo: 1.Почему такой большой диапазон адресов между stack и heap главного потока?
 *	2. рассказать про завершение потоков через return
 *	3. pthread_exit - это?
 *	4. надежность sleep, или как убедиться что поток точно был поставлен на выполнение и был обработан? какие есть способы
 *
 */


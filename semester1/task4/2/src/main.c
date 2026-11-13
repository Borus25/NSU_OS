#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("My PID: %d\n", getpid());
    sleep(5);

    char dirpath[256];
    if (getcwd(dirpath, sizeof(dirpath)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    if (execve(strcat(dirpath, "/main"), argv, NULL) == -1) {
        printf("errno = %d, %s\n", errno, strerror(errno));
        printf("%s\n", dirpath);
        return 2;
    }
    printf("Hello world from execv!\n");
}

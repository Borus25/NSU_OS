#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    if (fork() == 0) {
        printf("[PARENT] Я родитель, но скоро стану сиротой..., мой PID: %d\n", getpid());
        // Мы в РОДИТЕЛЕ (для ребенка), но мы РЕБЕНОК (для Bash)
        if (fork() == 0) {
            // ВНУК (Ребенок)
            sleep(10);
            printf("[GRANDCHILD] Мой новый родитель (PPID): %d\n", getppid());
            exit(0);
        }
        printf("[PARENT] Я умираю, оставляя ребенка сиротой...\n");
        exit(0);
    }

    sleep(20);
    while (1) {}
    return 0;
}
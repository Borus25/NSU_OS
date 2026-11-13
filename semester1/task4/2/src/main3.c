#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>

void segfault_handler(int sig) {
    printf("\n[SIGNAL] Caught SIGSEGV (%d): Invalid memory access detected!\n", sig);
    printf("[SIGNAL] The system stopped the process to prevent memory corruption.\n");
    exit(sig);
}

int main() {
    int my_pid = getpid();
    printf("My PID: %d\n", my_pid);
    sleep(15);

    signal(SIGSEGV, segfault_handler);

    void *pointers[10];
    size_t buffer_size = 1024 * 1024;

    printf("\n--- STEP 1: Heap Allocation ---\n");
    for (int i = 0; i < 10; i++) {
        pointers[i] = malloc(buffer_size);
        if (pointers[i] == NULL) {
            perror("malloc failed");
            return 1;
        }
        printf("Iteration %d: Allocated 1MB at %p\n", i, pointers[i]);
        sleep(1);
    }

    printf("\n--- STEP 2: Freeing Memory ---\n");
    for (int i = 0; i < 10; i++) {
        free(pointers[i]);
    }
    printf("Memory has been released.\n");
    sleep(2);

    printf("\n--- STEP 3: mmap Allocation (10 pages) ---\n");
    size_t page_size = getpagesize();
    size_t mmap_size = 10 * page_size;

    void *mmap_ptr = mmap(NULL, mmap_size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if (mmap_ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Mmapped region address: %p, size: %zu bytes\n", mmap_ptr, mmap_size);
    strcpy((char*)mmap_ptr, "Memory Test Data");

    printf("\n--- Protection Test (mprotect) ---\n");
    printf("Setting memory to READ-ONLY mode...\n");
    mprotect(mmap_ptr, mmap_size, PROT_READ);

    // printf("Attempting to WRITE to read-only memory\n");
    // ((char*)mmap_ptr)[0] = 'X';

    printf("\n--- STEP 5: Unmapping middle pages (4 to 6) ---\n");
    void *middle_start = (char*)mmap_ptr + (3 * page_size);
    size_t unmap_len = 3 * page_size;
    if (munmap(middle_start, unmap_len) == -1) {
        perror("munmap failed");
    } else {
        printf("Pages 4, 5, and 6 have been unmapped.\n");
    }

    sleep(10);
    return 0;
}
// #include <stdio.h>
#include <dlfcn.h> // 4

int printf (const char *__restrict __format, ...);

typedef void (*psym)(void);

int main() {
    void *lib = dlopen("./libdyn_runtime.so", RTLD_LAZY); // загружаем библиотеку во время выполнения функции
    if (!lib) {
        printf("Error 1\n");
		printf("%s\n", dlerror());
        return 0;
    }

    psym hello = (hello_func)dlsym(lib, "hello_from_dyn_runtime_lib"); // получаем указатель на функцию из dyn_runtime_lib.c
    if (!hello) {
        printf("Error 2");
        dlclose(lib);
        return 0;
    }

    hello();

    dlclose(lib); // освобождаем библиотеку после использования функции
    return 0;
}
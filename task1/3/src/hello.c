// #include <stdio.h>
#include <dlfcn.h> // 4

int printf (const char *__restrict __format, ...);

int main() {
    void *lib = dlopen("./libdyn_runtime.so", RTLD_LAZY); // загружаем библиотеку во время выполнения функции
    if (!lib) {
        printf("Error 1");
        return 0;
    }

    void (*hello)() = dlsym(lib, "hello_from_dyn_runtime_lib"); // получаем указатель на функцию из dyn_runtime_lib.c
    if (!hello) {
        printf("Error 2");
        return 0;
    }

    hello();

    dlclose(lib); // освобождаем библиотеку после использования функции
    return 0;
}
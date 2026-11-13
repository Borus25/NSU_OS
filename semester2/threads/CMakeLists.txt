cmake_minimum_required(VERSION 3.16)

project(task1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(GLIBC_SOURCE_DIR "$ENV{HOME}/CLionProjects/glibc_src/glibc"
        CACHE PATH "Path to glibc sources")
file(GLOB_RECURSE GLIBC_NAV_SOURCES
        "${GLIBC_SOURCE_DIR}/*.c"
)
add_library(glibc_navigation OBJECT EXCLUDE_FROM_ALL
        ${GLIBC_NAV_SOURCES}
)

add_executable(task1
        task1/thread.c
)

target_link_libraries(task1
        PRIVATE
        Threads::Threads
)
#pragma once
#include <pthread.h>

typedef void (*task_func)(void* arg);

typedef struct task{
    task_func f;
    void* arg;
    struct task* next; 
    struct task* prev;
} task_t;

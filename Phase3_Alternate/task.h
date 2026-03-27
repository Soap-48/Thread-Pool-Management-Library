#pragma once
#include <pthread.h>

typedef void (*task_func)(void* arg);

typedef struct task{
    task_func f;
    void* arg;
} task_t;

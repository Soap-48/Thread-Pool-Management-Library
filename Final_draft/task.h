#pragma once
#include <functional>

typedef struct task{
    std::function<void()> f;
    struct task* next; 
    struct task* prev;
} task;
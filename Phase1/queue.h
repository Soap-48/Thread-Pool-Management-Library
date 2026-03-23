#pragma once

#include <pthread.h>
#include "task.h"
#include "worker.h"

class queue{
    task *head;
    task *tail;
    int task_count;
    //size_t max_size;

    public:
        queue();
        ~queue();
        void push(worker* w,task *t);
        task* pop();
        bool empty();
};
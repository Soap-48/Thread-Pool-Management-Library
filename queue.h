#pragma once

#include <pthread.h>
#include "task.h"
#include "worker.h"

class queue{
    task *head;
    task *tail;
    size_t task_count;
    //size_t max_size;

    pthread_mutex_t q_lock;
    pthread_cond_t q_cond;

    public:
        queue();
        ~queue();
        void push(task *t);
        task* pop();
};
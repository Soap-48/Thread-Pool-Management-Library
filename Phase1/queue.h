#pragma once

#include <pthread.h>
#include "task.h"
//#include "worker.h" cant include worker.h since worker class need queue class, circular dependency error

class worker; //Forward declaration

class queue{
    task *head;
    task *tail;
    int task_count;
    //size_t max_size;

    public:
        queue();
        ~queue();
        void push(worker* w,task *t);
        task* pop(worker* w);
        bool empty();
};
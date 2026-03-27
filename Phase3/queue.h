#pragma once

#include "task.h"

class worker; //Forward declaration

class queue{
    task *head;
    task *tail;

    public:
        int task_count;
        queue();
        // void push(task *t);
        // task* pop();
        void  push_back(task *t);
        task* pop_back();
        task* pop_front();
        bool empty();
};
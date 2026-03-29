#pragma once

#include "IO_task.h"

class IO_queue{
    public:
    IO_Request *head;
    IO_Request *tail;

    int task_count;
    IO_queue();
    void push_back(IO_Request *req);
    IO_Request* pop_front();
    bool empty();
};
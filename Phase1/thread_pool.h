#pragma once
#include <vector>
#include "worker.h"
#include "task.h"

class thread_pool {
    private:
        std::vector<worker*> workers;
        int num_workers;
        bool stop;

    public:
        thread_pool(int);
        ~thread_pool();
        void submit(void (*fptr)(void *),void *arg);
};
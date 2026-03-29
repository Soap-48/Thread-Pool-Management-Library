#pragma once
#include <vector>
#include <functional>
#include "worker.h"
#include "task.h"

class thread_pool {
    private:
        bool stop;

    public:
        int num_workers;
        std::vector<worker*> workers;

        thread_pool(int num_workers); 
        ~thread_pool();
        void submit(std::function<void()> task_func);
        void submit_n(std::function<void()> task_func, int id);
};
#pragma once
#include <vector>
//#include <random>
#include "worker.h"
#include "task.h"

class thread_pool {
    private:
        bool stop;

    public:
        int num_workers;
        std::vector<worker*> workers;
        //std::mt19937 rng;
        //std::uniform_int_distribution<std::mt19937::result_type> dist; // distribution in range [1, 6]
        thread_pool(int);
        ~thread_pool();
        void submit(void (*fptr)(void *),void *arg);
        void submit_n(void (*fptr)(void *),void *arg,int id);
};
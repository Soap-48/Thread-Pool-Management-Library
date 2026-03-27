#pragma once

#include <pthread.h>
#include <deque>
#include "task.h"
#include "rng.h"

class thread_pool;

class worker{
    public:
        pthread_t thread;
        std::deque <task*> worker_queue;

        int worker_id;
        thread_pool *pool;
    
        pthread_mutex_t lock;
        pthread_cond_t cond;
        bool stop;
        rng rng_o;

        worker(int id, thread_pool* pool_ptr);

        ~worker();

        void start();
        task* steal();
        static void* worker_loop(void* arg);
};
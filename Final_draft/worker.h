#pragma once

#include <pthread.h>
#include <stdexcept>
#include <atomic>
#include "queue.h"
#include "task.h"
#include "rng.h"

class thread_pool;

class worker {
    public:
        pthread_t thread;
        queue worker_queue1,worker_queue2;

        int worker_id;
        thread_pool *pool;
    
        pthread_mutex_t lock1;
        pthread_cond_t cond1;
        pthread_mutex_t lock2;
        pthread_cond_t cond2;
        std::atomic<bool> stop;
        rng rng_o;

        worker(int id, thread_pool* pool_ptr);
        ~worker();

        void start();
        void execute_task(task *t);
        task* steal();
        static void* worker_loop(void* arg);
};
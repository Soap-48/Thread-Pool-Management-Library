#pragma once

#include <pthread.h>
#include <stdexcept>
#include "queue.h"
#include "task.h"

class thread_pool;

class worker{
    public:
        pthread_t thread;
        queue worker_queue;

        int worker_id;
        thread_pool *pool;
    
        pthread_mutex_t lock;
        pthread_cond_t cond;
        bool stop;
        worker(int id, thread_pool* pool_ptr);

        ~worker();

        static void* worker_loop(void* arg);
};
#pragma once 

#include <pthread.h>
#include <unistd.h>
#include <vector>
#include "queue.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <iostream>
#include "IO_Core.h"
#include "thread_pool.h"

class IO_Pool {
private:
    std::vector<pthread_t> workers;
    IO_queue IO_tasks;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int num_threads;
    bool pool_shutdown;
    thread_pool* Compute_pool; //To avoid name shadowing

    static void* worker_routine(void* arg) {
        IO_Pool* pool = static_cast<IO_Pool*>(arg);
        pool->worker_loop();
        return nullptr;
    }

    void worker_loop();

public:
    IO_Pool();
    ~IO_Pool();
    void worker_loop();
};
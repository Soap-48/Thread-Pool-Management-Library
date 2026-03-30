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

public:
    IO_Pool(int n, thread_pool *p);
    ~IO_Pool();
    void worker_loop();
    void submit(int fd, size_t bytes_to_io, bool io_type,
    std::function<void(std::shared_ptr<std::vector<char>>)> callback,
    bool is_shutdown_flag,
    std::shared_ptr<std::vector<char>> buffer);
};
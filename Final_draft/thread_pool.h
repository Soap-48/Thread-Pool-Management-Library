#pragma once
#include "task.h"
#include "worker.h"
#include <exception>
#include <functional>
#include <vector>

enum class priority {
    HIGH,
    LOW
};
class thread_pool {
private:
    bool stop;

public:
    int num_workers;
    std::vector<worker *> workers;
    std::vector<std::exception_ptr> e_queue;
    pthread_mutex_t e_mutex;
    pthread_cond_t  e_cond;

    thread_pool(int num_workers);
    ~thread_pool();
    void submit(std::function<void()> task_func, priority pr);
    void submit_n(std::function<void()> task_func, int id);

    void push_exception(std::exception_ptr);
    bool check_exceptions(int timeout_ms = 0);
    bool check_exceptions_nonblocking();
    int pending_exception_count();
};
#include "thread_pool.h"
#include "task.h"
#include <atomic>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>

thread_pool::thread_pool(int n = sysconf(_SC_NPROCESSORS_ONLN)) : stop(false) {
    if (n <= 0 || n > (static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN))))
        n = (static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN)));
    num_workers = n;

    pthread_mutex_init(&e_mutex, nullptr);
    pthread_cond_init(&e_cond, nullptr);

    workers.reserve(num_workers);

    for (int i = 0; i < num_workers; i += 1) {
        workers.push_back(new worker(i, this));
    }

    for (int i = 0; i < num_workers; i += 1) {
        workers[i]->start();
    }
}

thread_pool::~thread_pool() {
    // Graceful shutdown
    for (int i = 0; i < num_workers; i += 1) {
        pthread_mutex_lock(&workers[i]->lock1);
        workers[i]->stop.store(true);
        pthread_cond_signal(&workers[i]->cond1);
        pthread_mutex_unlock(&workers[i]->lock1);
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i]->thread, nullptr);
        delete workers[i];
    }
    workers.clear();

    pthread_mutex_lock(&e_mutex);
    pthread_cond_signal(&e_cond);
    pthread_mutex_unlock(&e_mutex);

    pthread_mutex_destroy(&e_mutex);
    pthread_cond_destroy(&e_cond);
}

void thread_pool::submit(std::function<void()> task_func, priority pr) {
    task *t = new task;
    t->f = std::move(task_func); // std::move is more efficient for std::function
    t->prev = t->next = nullptr;

    // Load balancing (round robin)
    static std::atomic<int> w{0};
    int target = w.fetch_add(1, std::memory_order_relaxed) % num_workers;

    pthread_mutex_lock(&workers[target]->lock1);
    bool flag = workers[target]->worker_queue1.empty();
    if (pr == priority::HIGH)
        workers[target]->worker_queue1.push_back(t);
    else {
        if (flag)
            workers[target]->worker_queue1.push_back(t);
        else
            workers[target]->worker_queue2.push_back(t);
    }
    pthread_cond_signal(&workers[target]->cond1);
    pthread_mutex_unlock(&workers[target]->lock1);
}

// purely testing function
void thread_pool::submit_n(std::function<void()> task_func, int id) {
    task *t = new task;
    t->f = std::move(task_func);
    t->prev = t->next = nullptr;

    pthread_mutex_lock(&workers[id]->lock1);
    workers.at(id)->worker_queue1.push_back(t);
    pthread_cond_signal(&workers[id]->cond1);
    pthread_mutex_unlock(&workers[id]->lock1);
}

void thread_pool::push_exception(std::exception_ptr ptr) {
    pthread_mutex_lock(&e_mutex);
    e_queue.push_back(ptr);
    pthread_cond_signal(&e_cond);
    pthread_mutex_unlock(&e_mutex);
}

bool thread_pool::check_exceptions(int timeout_ms) {
    pthread_mutex_lock(&e_mutex);
    struct timespec ts;
    if (timeout_ms > 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += (long)timeout_ms * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
    }

    while (!stop && e_queue.empty()) {
        if (timeout_ms > 0) {
            int rc = pthread_cond_timedwait(&e_cond, &e_mutex, &ts);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&e_mutex);
                return false;
            }
        } else {
            pthread_cond_wait(&e_cond, &e_mutex);
        }
    }

    if (pending_exception_count()) {
        std::exception_ptr eptr = e_queue.front();
        e_queue.erase(e_queue.begin());

        pthread_mutex_unlock(&e_mutex);
        std::rethrow_exception(eptr);

        return true;
    } else
        return false;
}

bool thread_pool::check_exceptions_nonblocking() {
    pthread_mutex_lock(&e_mutex);
    if (e_queue.empty()) {
        pthread_mutex_unlock(&e_mutex);
        return false;
    }
    std::exception_ptr eptr = e_queue.front();
    e_queue.erase(e_queue.begin());
    pthread_mutex_unlock(&e_mutex);
    std::rethrow_exception(eptr);
    return true;
}

int thread_pool::pending_exception_count() {
    pthread_mutex_lock(&e_mutex);
    int count = (int)e_queue.size();
    pthread_mutex_unlock(&e_mutex);
    return count;
}
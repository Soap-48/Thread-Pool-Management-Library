#include "worker.h"
#include "rng.h"
#include "task.h"
#include "thread_pool.h"
#include <iostream>
#include <random>

worker::worker(int id, thread_pool *pool_ptr) : worker_id(id),
                                                pool(pool_ptr),
                                                stop(false),
                                                rng_o(std::random_device{}())

{
    if (pthread_mutex_init(&lock1, nullptr) != 0) {
        throw std::runtime_error("Mutex init failed.\n");
    }
    if (pthread_mutex_init(&lock2, nullptr) != 0) {
        throw std::runtime_error("Mutex init failed.\n");
    }
    if (pthread_cond_init(&cond1, nullptr)) {
        pthread_mutex_destroy(&lock1);
        throw std::runtime_error("Cond i");
    }
    if (pthread_cond_init(&cond2, nullptr)) {
        pthread_mutex_destroy(&lock2);
        throw std::runtime_error("Cond i");
    }
}

void worker::start() {
    pthread_create(&thread, nullptr, worker_loop, this);
}

worker::~worker() {
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);
}

void worker::execute_task(task *t) {
    try {
        t->f();
    } catch (...) {
        pool->push_exception(std::current_exception());
    }
}

task *worker::steal() {
    int num = pool->num_workers;
    int rnd = rng_o.next_in_range(num);

    task *t = nullptr;
    for (int i = 0; i < num; i += 1) {
        int target = (rnd + i) % num;
        if (target != worker_id) {
            pthread_mutex_lock(&pool->workers[target]->lock1);
            t = pool->workers[target]->worker_queue1.pop_back();
            pthread_mutex_unlock(&pool->workers[target]->lock1);
            if (t != nullptr) {
                //
                std::cerr << worker_id << " stole from queue 1 of" << target << std::endl;
                return t;
            }
        }
    }
    if(!t)
    {
        for (int i = 0; i < num; i += 1) {
            int target = (rnd + i) % num;
            if (target != worker_id) {
                pthread_mutex_lock(&pool->workers[target]->lock2);
                t = pool->workers[target]->worker_queue2.pop_back();
                pthread_mutex_unlock(&pool->workers[target]->lock2);
                if (t != nullptr) {
                    //
                    std::cerr << worker_id << " stole from queue 2 of " << target << std::endl;
                    return t;
                }
            }
        }
    }
    return nullptr;
}

void* worker::worker_loop(void *arg) {
    worker* w=(worker*) arg;
    int cts_run=0;
    while(!w->stop.load())
    {
        pthread_mutex_lock(&w->lock1);
        task* t=w->worker_queue1.pop_front();
        pthread_mutex_unlock(&w->lock1);
        if(t&&cts_run<5)
        {
            w->execute_task(t);
            delete t;
            t=nullptr;
            cts_run++;
            continue;
        }
        pthread_mutex_lock(&w->lock2);
        t=w->worker_queue2.pop_front();
        pthread_mutex_unlock(&w->lock2);
        if(t)
        {
            w->execute_task(t);
            delete t;
            t=nullptr;
            cts_run=0;
            continue;
        }
        pthread_mutex_lock(&w->lock1);
        bool flag=w->worker_queue1.empty();
        pthread_mutex_unlock(&w->lock1);
        while(flag&&!w->stop.load())
        {
            t = w->steal();
            if (t) {
                w->execute_task(t);
                delete t;
                t = nullptr;
            }
            else
            {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_nsec += 2000000;
                if (ts.tv_nsec >= 1000000000) {
                    ts.tv_sec++;
                    ts.tv_nsec -= 1000000000;
                }
                pthread_mutex_lock(&w->lock1);
                if (w->worker_queue1.empty())
                    pthread_cond_timedwait(&w->cond1, &w->lock1, &ts);
                pthread_mutex_unlock(&w->lock1);
            }
            pthread_mutex_lock(&w->lock1);
            flag=w->worker_queue1.empty();
            pthread_mutex_unlock(&w->lock1);
        }
        cts_run=0;
    }
    while (!w->worker_queue1.empty()) {
        pthread_mutex_lock(&w->lock1);
        task *t = w->worker_queue1.pop_front();
        pthread_mutex_unlock(&w->lock1);
        if (t != nullptr) {
            w->execute_task(t);
            delete t;
            t=nullptr;
        }
    }
    while (!w->worker_queue2.empty()) {
        pthread_mutex_lock(&w->lock2);
        task *t = w->worker_queue2.pop_front();
        pthread_mutex_unlock(&w->lock2);
        if (t != nullptr) {
            w->execute_task(t);
            delete t;
            t=nullptr;
        }
    }
    while(true)
    {
        task* t= nullptr;
        t=w->steal();
        if(t)
        {
            w->execute_task(t);
            delete t;
            t=nullptr;
        }
        else
        break;
    }
    return nullptr;
}

















#include "worker.h"
#include "rng.h"
#include "thread_pool.h"
#include <iostream>
#include <random>

worker::worker(int id, thread_pool *pool_ptr) : worker_queue(),
                                                worker_id(id),
                                                pool(pool_ptr),
                                                stop(false),
                                                rng_o(std::random_device{}())

{
    if (pthread_mutex_init(&lock, nullptr) != 0) {
        throw std::runtime_error("Mutex init failed.\n");
    }
    if (pthread_cond_init(&cond, nullptr) != 0) {
        pthread_mutex_destroy(&lock);
        throw std::runtime_error("Cond init failed.\n");
    }
}

void worker::start() {
    pthread_create(&thread, nullptr, worker_loop, this);
}

worker::~worker() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}

task *worker::steal() {
    int num = pool->num_workers;
    int rnd = rng_o.next_in_range(num);

    task *t = nullptr;
    for (int i = 0; i < num; i += 1) {
        int target = (rnd + i) % num;
        if (target != worker_id) {
            worker* w=pool->workers[target];
            pthread_mutex_lock(&w->lock);
            if(!w->worker_queue.empty())
            {
                t = w->worker_queue.back();
                w->worker_queue.pop_back();
            }
            pthread_mutex_unlock(&w->lock);
            if (t != nullptr) {
                //
                std::cerr << worker_id << " stole from " << target << std::endl;
                return t;
            }
        }
    }
    return nullptr;
}

void *worker::worker_loop(void *arg) {
    worker *w = (worker *)arg;

    // will write graceful shutdown later
    while (!w->stop) {
        task *t = nullptr;
        pthread_mutex_lock(&w->lock); // graceful shutdown, this code runs when w->stop is set to 1 by pool

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 5000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        if (!w->worker_queue.empty()) {
            t = w->worker_queue.front();
            w->worker_queue.pop_front();
        }
        pthread_mutex_unlock(&w->lock);
        if (t != nullptr) {
            t->f(t->arg);
            delete t;
        } else {
            pthread_mutex_lock(&w->lock);
            bool flag = w->worker_queue.empty();
            pthread_mutex_unlock(&w->lock);
            while (flag && !w->stop) {
                // pthread_cond_wait(&w->cond, &w->lock);
                t = w->steal();
                if (t != nullptr) {
                    t->f(t->arg);
                    delete t;
                    t = nullptr;
                } else {
                    // struct timespec ts;
                    // clock_gettime(CLOCK_REALTIME, &ts);
                    // ts.tv_nsec += 5000000;
                    // if (ts.tv_nsec >= 1000000000) {
                    //     ts.tv_sec++;
                    //     ts.tv_nsec -= 1000000000;
                    // }
                    pthread_mutex_lock(&w->lock);
                    if (w->worker_queue.empty())
                        pthread_cond_timedwait(&w->cond, &w->lock, &ts);
                    pthread_mutex_unlock(&w->lock);
                    ts.tv_nsec += 5000000;
                    if (ts.tv_nsec >= 1000000000) {
                        ts.tv_sec++;
                        ts.tv_nsec -= 1000000000;
                    }
                }

                pthread_mutex_lock(&w->lock);
                flag = w->worker_queue.empty();
                pthread_mutex_unlock(&w->lock);
            }
            // pthread_mutex_unlock(&w->lock);
        }
    }
    // graceful shutdown, this code runs when w->stop is set to 1 by pool
    // std::cerr << "Worker " << w->worker_id << "exiting outer loop" << std::endl;
    while (true) {
        while (!w->worker_queue.empty()) {
            pthread_mutex_lock(&w->lock); // do we really need mutex when we are exiting?? also benchamrk this
            task *t = w->worker_queue.front();
            w->worker_queue.pop_front();
            pthread_mutex_unlock(&w->lock);
            if (t != nullptr) {
                t->f(t->arg);
                delete t;
            }
        }
        task *t = w->steal();
        if (t == nullptr)
            break;
        else {
            t->f(t->arg);
            delete t;
        }
    }
    // std::cerr<<"Worker "<<w->worker_id<<"dying"<<std::endl;
    return nullptr;
}

// void *worker::worker_loop(void *arg) {
//     worker *w = (worker *)(arg);
//     while (true) {
//         task *t=nullptr;
//         pthread_mutex_lock(&w->lock);
//         while (w->worker_queue.empty() && !w->stop) {
//                 pthread_cond_wait(&w->cond, &w->lock);
//         }
//         if (w->stop && w->worker_queue.empty()) {
//             pthread_mutex_unlock(&w->lock);
//             break;
//         }
//         t = w->worker_queue.pop_front();// Previosuly task *t = w->worker_queue.pop();
//         pthread_mutex_unlock(&w->lock);
//         if (t) {
//             t->f(t->arg);
//             delete t;
//         }
//     }
//     return nullptr;
// }

// Where is worker loop called?

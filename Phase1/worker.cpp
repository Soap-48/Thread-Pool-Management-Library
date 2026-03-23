#include "worker.h"
#include <iostream> //debug
worker::worker(int id, thread_pool *pool_ptr) : worker_id(id),
                                                stop(false),
                                                pool(pool_ptr)

{
    if (pthread_mutex_init(&lock, nullptr) != 0) {
        throw std::runtime_error("Mutex init failed.\n");
    }
    if (pthread_cond_init(&cond, nullptr) != 0) {
        pthread_mutex_destroy(&lock);
        throw std::runtime_error("Cond init failed.\n");
    }
    // tempchange
    pthread_create(&thread, nullptr, worker_loop, this);
}

worker::~worker() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}

void *worker::worker_loop(void *arg) {
    worker *w = (worker *)(arg);

    //Debug
    //std::cerr<<"Worker "<<((w)->worker_id)<<" is allive\n";
    //Debug

    while (true) {
        pthread_mutex_lock(&w->lock);
        while (w->worker_queue.empty() && !w->stop) {
            pthread_cond_wait(&w->cond, &w->lock);
        }
        if (w->stop && w->worker_queue.empty()) {
            pthread_mutex_unlock(&w->lock);
            break;
        }
        task *t = w->worker_queue.pop(w);// Previosuly task *t = w->worker_queue.pop();
        pthread_mutex_unlock(&w->lock);
        if (t) {
            t->f(t->arg);
            delete t;
        }
    }
    return nullptr;
}

//Where is worker loop called?
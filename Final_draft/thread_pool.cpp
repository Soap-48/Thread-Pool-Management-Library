#include "thread_pool.h"
#include "task.h"
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <iostream>

thread_pool::thread_pool(int n=sysconf(_SC_NPROCESSORS_ONLN)) : stop(false) {
     if(n<=0||n>(static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN))))
        n=(static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN)));
    num_workers=n;

    workers.reserve(num_workers);

    for(int i = 0; i < num_workers; i += 1){
        workers.push_back(new worker(i, this));
    }

    for(int i = 0; i < num_workers; i += 1){
        workers[i]->start();
    }
}

thread_pool::~thread_pool(){
    // Graceful shutdown
    for(int i = 0; i < num_workers; i += 1){
        pthread_mutex_lock(&workers[i]->lock);
        workers[i]->stop = 1;
        pthread_cond_signal(&workers[i]->cond);
        pthread_mutex_unlock(&workers[i]->lock);
    }

    for(int i = 0; i < num_workers; i++){
        pthread_join(workers[i]->thread, nullptr);
        delete workers[i]; // FIX: You allocated with 'new', so you must 'delete' to prevent a memory leak
    }
    workers.clear();
}

void thread_pool::submit(std::function<void()> task_func){
    task *t = new task; 
    t->f = std::move(task_func); // std::move is more efficient for std::function
    t->prev=t->next = nullptr;
    // t->arg = arg; --> Removed! std::function carries its own state.

    // Load balancing (round robin)
    static std::atomic<int> w{0};
    int target = w.fetch_add(1, std::memory_order_relaxed) % num_workers;
    
    pthread_mutex_lock(&workers[target]->lock);
    workers[target]->worker_queue.push_back(t);
    pthread_cond_signal(&workers[target]->cond);
    pthread_mutex_unlock(&workers[target]->lock);
}

// purely testing function
void thread_pool::submit_n(std::function<void()> task_func, int id){
    task *t = new task;
    t->f = std::move(task_func); 
    t->prev=t->next = nullptr;

    pthread_mutex_lock(&workers[id]->lock); 
    workers.at(id)->worker_queue.push_back(t);
    pthread_cond_signal(&workers[id]->cond);
    pthread_mutex_unlock(&workers[id]->lock);
}
#include "thread_pool.h"
#include "IO_Pool.h"

IO_Pool::IO_Pool():num_threads(num_threads),pool_shutdown(false), Compute_pool(Compute_pool)
{
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        throw std::runtime_error("IOPool: Failed to initialize queue mutex.");
    }
    if (pthread_cond_init(&cond, nullptr) != 0) {
        pthread_mutex_destroy(&mutex);
        throw std::runtime_error("IOPool: Failed to initialize queue condition variable.");
    }
    
    workers.resize(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&workers[i], nullptr, worker_routine, this) != 0) {
            throw std::runtime_error("IOPool: Failed to create worker thread.");
        }
    }
}
IO_Pool::~IO_Pool()
{
    pthread_mutex_lock(&mutex);
    pool_shutdown = true;
    pthread_mutex_unlock(&mutex);
    pthread_cond_broadcast(&cond);//To tackle Thundering herd!

    for (pthread_t& thread : workers) {
        pthread_join(thread, nullptr);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}
void IO_Pool::worker_loop() {
    while (true) {
        IO_Request* req=nullptr;

        pthread_mutex_lock(&mutex);

        while (IO_tasks.empty() && !pool_shutdown) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (pool_shutdown && IO_tasks.empty()) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        req = IO_tasks.pop_front();
        pthread_mutex_unlock(&mutex);
        if(!req)
        continue;
        if (req->is_shutdown_flag) {//Implements dynamic resizing
            delete req;
            req=nullptr;
            break; 
        }
        if (req->buffer && req->fd >= 0) {
            req->buffer->resize(req->bytes_to_read);
            ssize_t bytes_read = read(req->fd, req->buffer->data(), req->bytes_to_read);
            
            if (bytes_read > 0) {
                req->buffer->resize(bytes_read); 
            } 
            else {
                req->buffer->clear(); 
            }
        }
        if (req->callback) {
            auto user_callback=std::move(req->callback);//Critical
            auto safe_buffer=req->buffer;
            Compute_pool->submit([user_callback,safe_buffer](){
                user_callback(safe_buffer);
            });
        }
        delete req;
        req=nullptr;
    }
}
#include "thread_pool.h"
#include "IO_Pool.h"

IO_Pool::IO_Pool(int n, thread_pool *p):num_threads(n),pool_shutdown(false), Compute_pool(p)
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
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

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
            req->buffer->resize(req->bytes_to_io);
            ssize_t bytes;
            if(!(req->io_type))
            bytes = read(req->fd, req->buffer->data(), req->bytes_to_io);
            else
            bytes = write(req->fd, req->buffer->data(), req->bytes_to_io);
            if (bytes > 0) {
                req->buffer->resize(bytes); 
            } 
            else {
                req->buffer->clear();//
                //
                std::cerr<<"Failed Read/Write\n";
            }
        }
        if (req->callback) {
            std::cerr<<"IO_POOL starting callback"<<std::endl;
            auto user_callback=std::move(req->callback);//Critical
            auto safe_buffer=req->buffer;
            Compute_pool->submit([user_callback,safe_buffer](){
                user_callback(safe_buffer);
            });
            std::cerr<<"IO_POOL callback returned"<<std::endl;
        }
        delete req;
        req=nullptr;
    }
}
void IO_Pool::submit(int fd, size_t bytes_to_io, bool io_type,
    std::function<void(std::shared_ptr<std::vector<char>>)> callback,
    bool is_shutdown_flag,
    std::shared_ptr<std::vector<char>> buffer)
{
    IO_Request* io_task=new IO_Request;
    io_task->fd=fd;
    io_task->bytes_to_io=bytes_to_io;
    io_task->io_type=io_type;
    io_task->callback=callback;
    io_task->is_shutdown_flag=is_shutdown_flag;
    io_task->buffer=buffer;
    pthread_mutex_lock(&mutex);
    IO_tasks.push_back(io_task);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
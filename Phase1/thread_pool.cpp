#include "thread_pool.h"
#include "task.h"
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <iostream> //for debug

thread_pool::thread_pool(int n=sysconf(_SC_NPROCESSORS_ONLN)):stop(false){
    n=std::min(static_cast<long>(n),sysconf(_SC_NPROCESSORS_ONLN));
    num_workers=n;
    workers.reserve(n);
    for(int i=0;i<n;i+=1){
        workers.push_back(new worker(i,this));
    }
}

thread_pool::~thread_pool(){
    //write graceful shutdown
    for(int i=0;i<num_workers;i+=1){
        pthread_mutex_lock(&workers[i]->lock);
        workers[i]->stop=1;
        pthread_cond_signal(&workers[i]->cond);
        pthread_mutex_unlock(&workers[i]->lock);
    }

    for(int i=0;i<num_workers;i++){
        pthread_join(workers[i]->thread,nullptr);
    }
    workers.clear();
}

void thread_pool::submit(void (*fptr)(void *),void *arg){
    task_t *t=new task_t;
    t->f=fptr;
    t->arg=arg;
    t->next=nullptr;
    //load balancing, using priority queue or heap or something
    //gives w which the id of worker to use
    //for now we use a round robin like thingy
    static int w=0;
    w=(w+1)%num_workers;
    //make something better
    pthread_mutex_lock(&workers[w]->lock); // mutex locks used because other workers can access this queue during worker stealing
    workers[w]->worker_queue.push(workers[w],t);
    pthread_cond_signal(&workers[w]->cond);
    pthread_mutex_unlock(&workers[w]->lock);

    //Debug
    //std::cerr<<"Task Pushed\n";
    //Debug

}
//task *t = w->worker_queue.pop(w); wtf this doing here??
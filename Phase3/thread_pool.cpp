#include "thread_pool.h"
#include "task.h"
#include <vector>
#include <unistd.h>
#include <pthread.h>
//#include <random>
#include <iostream>

thread_pool::thread_pool(int n=sysconf(_SC_NPROCESSORS_ONLN)):stop(false){
    if(n<=0||n>(static_cast<int>((_SC_NPROCESSORS_ONLN)))) //if n is not valid default to cores supported by device
        n=(static_cast<int>((_SC_NPROCESSORS_ONLN)));
    num_workers=n;

    //
    //std::cerr<<"Trying to create rnd gen"<<std::endl;
    //

    workers.reserve(num_workers);

    //std::cerr<<"Creating Workers"<<std::endl;

    for(int i=0;i<num_workers;i+=1){
        workers.push_back(new worker(i,this));
        //std::cerr<<"Cretaed Worker "<<i<<std::endl;
    }

    for(int i=0;i<num_workers;i+=1){
        workers[i]->start();
        //std::cerr<<"Started Worker "<<i<<std::endl;
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
    workers[w]->worker_queue.push_back(t);
    pthread_cond_signal(&workers[w]->cond);
    pthread_mutex_unlock(&workers[w]->lock);

}

//this is a purely testing function
void thread_pool::submit_n(void (*fptr)(void *),void *arg,int id){
    task_t *t=new task_t;
    t->f=fptr;
    t->arg=arg;
    t->next=nullptr;

    pthread_mutex_lock(&workers[id]->lock); // mutex locks used because other workers can access this queue during worker stealing
    workers[id]->worker_queue.push_back(t);
    pthread_cond_signal(&workers[id]->cond);
    pthread_mutex_unlock(&workers[id]->lock);

}

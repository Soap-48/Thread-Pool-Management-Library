#include "queue.h"
#include "worker.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0){;
}

queue::~queue() {;
}

//Removes handling of locks and cond to the functions calling push and pop instead of push and pop themselves
//to allow greater flexiblity in use by callee function

void queue::push(worker *w,task *t) {
    if (t != nullptr) {
        t->next = nullptr;
        // pthread_mutex_lock(&w->lock); // mutex locks used because other workers can access this queue during worker stealing
        if (tail)
            tail->next = t;
        else
            head = t;
        tail = t;
        task_count += 1;
        // pthread_cond_signal(&w->cond);
        // pthread_mutex_unlock(&w->lock);
    }
}

task *queue::pop(worker* w) {
    task *t=nullptr;
    //pthread_mutex_lock(&w->lock);
    if (task_count > 0) {
        t=head;
        head = head->next;
        if (head == nullptr)
            tail = nullptr;
        task_count -= 1;
    }
    //pthread_mutex_unlock(&w->lock);
    return t;
}

bool queue::empty()  {
    if(task_count==0)   return true;
    else    return false;
}
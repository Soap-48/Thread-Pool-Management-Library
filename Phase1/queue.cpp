#include "queue.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0), w(nullptr) {;
}

queue::~queue() {;
}

void queue::push(worker *w,task *t) {
    if (t != nullptr) {
        t->next = nullptr;
        pthread_mutex_lock(&w->q_lock); // mutex locks used because other workers can access this queue during worker stealing
        if (tail)
            tail->next = t;
        else
            head = t;
        tail = t;
        task_count += 1;
        pthread_cond_signal(&w->q_cond);
        pthread_mutex_unlock(&w->q_lock);
    }
}

task *queue::pop(worker* w) {
    task *t=nullptr;
    pthread_mutex_lock(&w->q_lock);
    if (task_count > 0) {
        t=head;
        head = head->next;
        if (head == nullptr)
            tail = nullptr;
        task_count -= 1;
    }
    pthread_mutex_unlock(&w->q_lock);
    return t;
}

bool queue:empty()  {
    if(task_count==0)   return false;
    else    return true;
}
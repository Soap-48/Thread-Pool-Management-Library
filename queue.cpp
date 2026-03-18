#include "queue.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0), w(nullptr) {
    pthread_mutex_init(&q_lock, NULL);
    pthread_cond_init(&q_cond, NULL);
}

queue::~queue() {
    pthread_mutex_destroy(&q_lock);
    pthread_cond_destroy(&q_cond);
}

void queue::push(task *t) {
    if (t != nullptr) {
        t->next = nullptr;
        pthread_mutex_lock(&q_lock); // mutex locks used because other workers can access this queue during worker stealing
        if (tail)
            tail->next = t;
        else
            head = t;
        tail = t;
        task_count += 1;
        pthread_cond_signal(&q_cond);
        pthread_mutex_unlock(&q_lock);
    }
    // cond var code
}

task *queue::pop() {
    task *t=nullptr;
    pthread_mutex_lock(&q_lock);
    if (task_count > 0) {
        t=head;
        head = head->next;
        if (head == nullptr)
            tail = nullptr;
        task_count -= 1;
    }
    pthread_mutex_unlock(&q_lock);
    return t;
}
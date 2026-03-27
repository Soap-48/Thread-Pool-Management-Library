#include "queue.h"
#include "worker.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0){;
}


//Removes handling of locks and cond to the functions calling push and pop instead of push and pop themselves
//to allow greater flexiblity in use by callee function

void queue::push(task *t) {
    if (t != nullptr) {
        t->next = nullptr;
        if (tail)
            tail->next = t;
        else
            head = t;
        tail = t;
        task_count += 1;
    }
}

task *queue::pop() {
    task *t=nullptr;
    if (task_count > 0) {
        t=head;
        head = head->next;
        if (head == nullptr)
            tail = nullptr;
        task_count -= 1;
    }
    return t;
}

bool queue::empty()  {
    if(task_count==0)   return true;
    else    return false;
}
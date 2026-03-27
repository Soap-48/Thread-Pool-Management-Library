#include "queue.h"
#include "worker.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0){;
}

//Removes handling of locks and cond to the functions calling push and pop instead of push and pop themselves
//to allow greater flexiblity in use by callee function

// void queue::push(task *t) {
//     if (t != nullptr) {
//         t->next = nullptr;
//         if (tail)
//             tail->next = t;
//         else
//             head = t;
//         tail = t;
//         task_count += 1;
//     }
// }

// task *queue::pop() {
//     task *t=nullptr;
//     if (task_count > 0) {
//         t=head;
//         head = head->next;
//         if (head == nullptr)
//             tail = nullptr;
//         task_count -= 1;
//     }
//     return t;
// }

void  queue::push_back(task *t){
    if (t==nullptr) return;
    t->next=nullptr;
    t->prev=tail;        // point back at current tail
    if(tail)
        tail->next=t;
    else
        head=t;          // queue was empty, head = t too
    tail=t;
    task_count++;
}
task* queue::pop_back(){
    if(task_count==0) return nullptr;
    task *t=tail;
    tail=tail->prev;
    if(tail)
        tail->next=nullptr;  // new tail has no successor
    else
        head=nullptr;        // queue is now empty
    task_count--;
    return t;
}
task* queue::pop_front(){
    if(task_count==0) return nullptr;
    task *t=head;
    head=head->next;
    if(head)
        head->prev=nullptr;  // new head has no predecessor
    else
        tail=nullptr;        // queue is now empty
    task_count--;
    return t;
}

bool queue::empty()  {
    if(task_count==0)   return true;
    else    return false;
}
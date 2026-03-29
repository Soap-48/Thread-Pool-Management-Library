#include "queue.h"
#include "worker.h"

queue::queue() : head(nullptr), tail(nullptr), task_count(0){;
}


void  queue::push_back(task *t){
    if (t==nullptr) return;
    t->next=nullptr;
    t->prev=tail;       
    if(tail)
        tail->next=t;
    else
        head=t;        
    tail=t;
    task_count++;
}
task* queue::pop_back(){
    if(task_count==0) return nullptr;
    task *t=tail;
    tail=tail->prev;
    if(tail)
        tail->next=nullptr; 
    else
        head=nullptr;      
    task_count--;
    return t;
}
task* queue::pop_front(){
    if(task_count==0) return nullptr;
    task *t=head;
    head=head->next;
    if(head)
        head->prev=nullptr;  
    else
        tail=nullptr;      
    task_count--;
    return t;
}

bool queue::empty()  {
    if(task_count==0)   return true;
    else    return false;
}
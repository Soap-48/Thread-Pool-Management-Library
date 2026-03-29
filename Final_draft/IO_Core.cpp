#include "IO_Core.h"
IO_queue::IO_queue() : head(nullptr), tail(nullptr), task_count(0){;
}

void  IO_queue::push_back(IO_Request* req){
    if (req==nullptr) return;
    req->next=nullptr;
    req->prev=tail;       
    if(tail)
        tail->next=req;
    else
        head=req;        
    tail=req;
    task_count++;
}
IO_Request* IO_queue::pop_front(){
    if(task_count==0) return nullptr;
    IO_Request* req=head;
    head=head->next;
    if(head)
        head->prev=nullptr;  
    else
        tail=nullptr;      
    task_count--;
    return req;
}
bool IO_queue::empty()  {
    if(task_count==0)   return true;
    else    return false;
}
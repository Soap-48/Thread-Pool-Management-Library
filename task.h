typedef void (*fn_ptr)(void* arg);

typedef struct task{
    fn_ptr fn;  //pointer to the submitted function
    void* arg;  //pointer to argumets
    struct task *next;  //pointer to next submitted task in linked list
} task_t;
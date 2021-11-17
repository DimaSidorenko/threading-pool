#ifndef TASKINFO_H
#define TASKINFO_H

#include <iostream>
#include <string>

enum TaskType{
    Fibonacci, Factorial, DoubleFactorial
};


enum TaskStatus {
    waiting, in_progress, done
};


struct TaskInfo{
    size_t task_id;
    TaskType task_type;
    int64_t param;

    TaskInfo(){}

    TaskInfo(size_t id, TaskType type, int64_t param){
        this->task_id = id;
        this->task_type = type;
        this->param = param;
    } 
};

#endif // TASKINFO_H

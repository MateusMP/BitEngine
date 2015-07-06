#pragma once


#include <memory>
#include <deque>

#include "Task.h"

namespace BitEngine{


class TaskManager{

    public:
        typedef std::deque<Task*> TaskList;

        void AddTask(Task *task);


    private:

        void Execute(Task *task);


};



}


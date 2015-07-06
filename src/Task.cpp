
#include "Task.h"

namespace BitEngine{



Task::Task(TASK_MODE flags)
    : m_flags(flags)
{

}

Task::~Task(){

}

Task::TASK_MODE Task::getFlags() const{
    return m_flags;
}



}

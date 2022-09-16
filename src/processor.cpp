#include "processor.h"

// Return the aggregate CPU utilization
float Processor::Utilization() { 
    auto idle = LinuxParser::IdleJiffies();
    auto active = LinuxParser::ActiveJiffies();
    
    auto total = idle + active;
    auto percent = ((float)(active-prevActive_)/(float)(total-(prevActive_+prevIdle_)));

    prevIdle_ = idle;
    prevActive_ = active;
    
    return percent; 
}
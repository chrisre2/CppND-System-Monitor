#include "processor.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    auto idle = LinuxParser::IdleJiffies();
    auto active = LinuxParser::ActiveJiffies();
    
    auto total = idle + active;
    auto percent = 1.0-((float)(idle-prevIdle_)/(float)(total-(prevActive_+prevIdle_)));

    prevIdle_ = idle;
    prevActive_ = active;
    
    return percent; 
}
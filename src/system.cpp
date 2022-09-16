#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

/*You need to complete the mentioned TODOs in order to satisfy the rubric criteria "The student will be able to extract and display basic data about the system."

You need to properly format the uptime. Refer to the comments mentioned in format. cpp for formatting the uptime.*/

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
    auto pids = LinuxParser::Pids();
    //add process to vector
    for(auto& pid : pids){
        Process process(pid);
        if((std::find(processes_.begin(),processes_.end(), process)==processes_.end()))
            processes_.emplace_back(process);
    }
    //remove process from vector
    processes_.erase(std::remove_if(processes_.begin(),processes_.end(),[](Process& p1){
        auto pids = LinuxParser::Pids();
        return (std::find(pids.begin(), pids.end(),p1.Pid())==pids.end())?true:false;
        }),processes_.end());
    //sort processes
    std::stable_sort(processes_.begin(),processes_.end());
    return processes_;
    }

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }

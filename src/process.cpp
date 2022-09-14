#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
    auto pids = LinuxParser::Pids();
    if(!(std::find(pids.begin(), pids.end(),Pid())!=pids.end()))
        return cpuUsage_;
    //auto totalTime = LinuxParser::ActiveJiffies(Pid());
    //auto seconds = LinuxParser::UpTime() - LinuxParser::UpTime(Pid());
    //cpuUsage_ = (float)(totalTime/sysconf(_SC_CLK_TCK))/seconds;
    auto totalProcess = LinuxParser::ActiveJiffies(Pid());
    auto totalSystem = LinuxParser::ActiveJiffies();
    auto totalPId = totalProcess;
    if(totalProcess != prevTotalProcess_)
        totalPId -= prevTotalProcess_;
    auto totalCPU = totalSystem;
    if(totalSystem != prevTotalSystem_)
        totalSystem -= prevTotalSystem_;
    prevTotalProcess_=totalProcess;
    prevTotalSystem_=totalSystem;
    cpuUsage_=(float)totalPId/(float)totalCPU;
    return cpuUsage_;
    }

// TODO: Return the command that generated this process
string Process::Command() {
    auto pids = LinuxParser::Pids();
    if((std::find(pids.begin(), pids.end(),Pid())!=pids.end()))
        command_= LinuxParser::Command(Pid());
    return command_;
     }

// TODO: Return this process's memory utilization
string Process::Ram() { 
    auto pids = LinuxParser::Pids();
    if((std::find(pids.begin(), pids.end(),Pid())!=pids.end()))
    ram_= LinuxParser::Ram(Pid());
    return ram_;
    }

// TODO: Return the user (name) that generated this process
string Process::User() {
    auto pids = LinuxParser::Pids();
    if((std::find(pids.begin(), pids.end(),pid_)!=pids.end()))
    user_= LinuxParser::User(Pid());
    return user_;
    }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() {
    auto pids = LinuxParser::Pids();
    if((std::find(pids.begin(), pids.end(),Pid())!=pids.end()))
    upTime_=LinuxParser::UpTime(Pid());
    return upTime_;
    }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const { return a.getCpuUsage()>getCpuUsage(); }
bool Process::operator==(Process const& a) const { return a.pid_==pid_; }
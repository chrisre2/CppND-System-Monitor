#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  auto pids = LinuxParser::Pids();
  if (!(std::find(pids.begin(), pids.end(), Pid()) != pids.end()))
    return cpuUsage_;
  float totalProcess = LinuxParser::ActiveJiffies(Pid());
  float totalSystem = LinuxParser::ActiveJiffies();

  float totalPId = totalProcess - prevTotalProcess_;
  float totalCPU = totalSystem - prevTotalSystem_;

  prevTotalProcess_ = totalProcess;
  prevTotalSystem_ = totalSystem;
  cpuUsage_ = (totalPId / totalCPU);
  return cpuUsage_;
}

// Return the command that generated this process
string Process::Command() {
  auto pids = LinuxParser::Pids();
  if ((std::find(pids.begin(), pids.end(), Pid()) != pids.end()))
    command_ = LinuxParser::Command(Pid());
  return command_;
}

// Return this process's memory utilization
string Process::Ram() {
  auto pids = LinuxParser::Pids();
  if ((std::find(pids.begin(), pids.end(), Pid()) != pids.end()))
    ram_ = LinuxParser::Ram(Pid());
  return ram_;
}

// Return the user (name) that generated this process
string Process::User() {
  auto pids = LinuxParser::Pids();
  if ((std::find(pids.begin(), pids.end(), pid_) != pids.end()))
    user_ = LinuxParser::User(Pid());
  return user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
  auto pids = LinuxParser::Pids();
  if ((std::find(pids.begin(), pids.end(), Pid()) != pids.end()))
    upTime_ = LinuxParser::UpTime(Pid());
  return upTime_;
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return a.cpuUsage_ < cpuUsage_;
}
bool Process::operator==(Process const& a) const { return a.pid_ == pid_; }
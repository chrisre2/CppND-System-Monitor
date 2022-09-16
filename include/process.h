#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid) : pid_(pid){};
  int Pid();                               // See src/process.cpp
  std::string User();                      // See src/process.cpp
  std::string Command();                   // See src/process.cpp
  float CpuUtilization();                  // See src/process.cpp
  std::string Ram();                       // See src/process.cpp
  long int UpTime();                       // See src/process.cpp
  bool operator<(Process const& a) const;  // See src/process.cpp
  bool operator==(Process const& a) const;

  // Declare any necessary private members
 private:
  int pid_;
  std::string user_;
  std::string command_;
  long prevTotalProcess_;
  long prevTotalSystem_;
  float cpuUsage_;
  long int upTime_;
  std::string ram_;
};

#endif
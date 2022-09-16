#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "linux_parser.h"
#include <asm/param.h>

using std::stoi;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
  //the macro is to determine the version of gcc where filesystem is declared
  //as stated in https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
  //must be 8 or higher as stated in https://gcc.gnu.org/gcc-8/changes.html
vector<int> LinuxParser::Pids() {
  vector<int> pids;
#if __GNUC__ > 7
  for(const auto& file : std::filesystem::directory_iterator(kProcDirectory)){
      if(file.is_directory()){
        string filename(file.path().filename());
        if(std::all_of(filename.begin(),filename.end(), isdigit)){
          int pid = stoi(filename);
          pids.emplace_back(pid);
        }
      }
  }
#else
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  #endif 
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  float totalMem = GetValue<float>(stream, "MemTotal");
  float freeMem = GetValue<float>(stream, "MemFree");
  return 1.f-(freeMem/totalMem);
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  string line, value;
  long uptime; 
  
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;    
  }
  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return UpTime()*sysconf(_SC_CLK_TCK); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatFilename);
  long utime= GetLongValue(stream, "14");
  long stime= GetLongValue(stream, "15");
  long cutime=GetLongValue(stream, "16");
  long cstime=GetLongValue(stream, "17");
  return utime+stime+cutime+cstime;
  }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  auto jiffies = CpuUtilization();
  vector<long>jiffiesLong;
  std::transform(jiffies.begin(), jiffies.end(), std::back_inserter(jiffiesLong),[&](std::string s) { return std::stol(s); });
  return jiffiesLong[0]+jiffiesLong[1]+jiffiesLong[2]+jiffiesLong[5]+jiffiesLong[6]+jiffiesLong[7];//user and nice had already the guest values
  }

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  auto jiffies = CpuUtilization();
  vector<long>jiffiesLong;
  std::transform(jiffies.begin(), jiffies.end(), std::back_inserter(jiffiesLong),[&](std::string s) { return std::stol(s); });
  return jiffiesLong[3]+jiffiesLong[4];
  }

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line, key;
  vector<string> jiffies{10};//0=user, 1=nice, 2=system, 3=idle, 4=iowait, 5=irq, 6=softirq, 7=steal, 8=guest, 9=guest_nice
  if (stream.is_open()) {    
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key >> jiffies[0]>> jiffies[1]>> jiffies[2]>> jiffies[3]>> jiffies[4]>> jiffies[5]>> jiffies[6]>> jiffies[7]>> jiffies[8]>> jiffies[9];    
  }
  return jiffies;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  int processes = GetValue<int>(stream, "processes");
  return processes; 
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  int processes=GetValue<int>(stream, "procs_running");
  return processes;
  }

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory+to_string(pid)+kCmdlineFilename);
  string line, cmd;
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cmd;
  }
  return cmd; }

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
   std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatusFilename);
  string line, key;
  float value = GetValue<float>(stream, "VmSize");
  value /=1000.f;
  std::stringstream format;
  format << std::fixed << std::setprecision(2) << value;
  return format.str();
  }

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatusFilename);
  string uid = GetValue<string>(stream, "Uid");
  return uid;
  }

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  std::ifstream stream(kPasswordPath);
  string  pUid = Uid(pid);
  string line, user, password, uid;
  if(stream.is_open()){
    while(uid!=pUid){
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':',' ');
      std::istringstream linestream(line);
      linestream  >> user >> password >> uid;
    }
  }
  return user;
  }

// Read and return the uptime of a process
//the time the process has been active in user mode + kernel mode
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatFilename);
  string line, value;
  auto utime=GetLongValue(stream, "14");
  auto stime=GetLongValue(stream, "15");
  auto upTime = utime+stime;
  return upTime/sysconf(_SC_CLK_TCK);
}

  template <typename T>
  T LinuxParser::GetValue(std::ifstream& stream, const std::string& key){
    T value {};
    if(stream.is_open()){
      string line, point;
      if(std::all_of(key.begin(),key.end(), isdigit)){
        std::getline(stream, line);
        int index = stoi(key);
        std::istringstream linestream(line);
        int position = 0;
        do{
          linestream >> value;
          position++;
        }while(position != index);
      }else
        while(std::getline(stream, line)){
          if(line.find(key)!=std::string::npos){
            std::replace(line.begin(), line.end(), ':',' ');
            std::istringstream linestream(line);
            linestream >> point >> value;
            break;
          }
        }
    }
    return value;
  }

  long LinuxParser::GetLongValue(std::ifstream& stream, const std::string& key){
    long value;
    if(stream.is_open()){
      string line, point;
      if(std::all_of(key.begin(),key.end(), isdigit)){
        std::getline(stream, line);
        int index = stoi(key);
        std::istringstream linestream(line);
        int position = 0;
        do{
          linestream >> point;
          position++;
        }while(position != index);
        value = std::stol(point);
        stream.clear();
        stream.seekg(0,std::ios::beg);
      }
    }
    return value;
  }

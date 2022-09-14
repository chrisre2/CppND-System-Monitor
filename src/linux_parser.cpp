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
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  //the following macro is to determine the version of gcc where filesystem is declared
  //as stated in https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
  //must be 8 or higher as stated in https://gcc.gnu.org/gcc-8/changes.html
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

// DONE: Read and return the system memory utilization
// TODO: check cached and buffers
float LinuxParser::MemoryUtilization() {
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  //string line, key;
  //float value;
  float totalMem = GetValue<float>(stream, "MemTotal");
  float freeMem = GetValue<float>(stream, "MemFree");
  return 1.f-(freeMem/totalMem);
  /*std::vector<float> memory(2);//0 is for total memory, 1 is for free memory
  
    if (stream.is_open()) {
      for(int i=0; i<2;i++)
      {
        std::getline(stream, line);
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);        
        linestream >> key >> value;
        memory[i]=value;
      }
    }  
  return 1 - (memory[1]/memory[0]);*/
}

// DONE: Read and return the system uptime
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

// TODO: Read and return the number of jiffies for the system
//(secconds * HZ)
long LinuxParser::Jiffies() { return UpTime()*sysconf(_SC_CLK_TCK); }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatFilename);
  string line, value;
  long utime, stime, cutime, cstime;
  //long utime= GetLongValue(stream, "14");
  //long stime= GetLongValue(stream, "15");
  //long cutime=GetLongValue(stream, "16");
  //long cstime=GetLongValue(stream, "17");
  if(stream.is_open()){    
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':',' ');
      std::istringstream linestream(line);
      //loop control for specific index of value to take values at 14 15 16 17 22
      int index=1;
      while(index <= 17)
      {
        switch(index)
        {
          case 14:
          linestream >> utime;
          break;
          case 15:
          linestream >> stime;
          break;
          case 16:
          linestream >> cutime;
          break;
          case 17:
          linestream >> cstime;
          break;
          default:
          linestream >> value;
        }
        index++;
      }    
  }
  return utime+stime+cutime+cstime;
  }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  auto jiffies = CpuUtilization();
  vector<long>jiffiesLong;
  std::transform(jiffies.begin(), jiffies.end(), std::back_inserter(jiffiesLong),[&](std::string s) { return std::stol(s); });
  return jiffiesLong[0]+jiffiesLong[1]+jiffiesLong[2]+jiffiesLong[5]+jiffiesLong[6]+jiffiesLong[7];//user and nice had already the guest values
  }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  auto jiffies = CpuUtilization();
  vector<long>jiffiesLong;
  std::transform(jiffies.begin(), jiffies.end(), std::back_inserter(jiffiesLong),[&](std::string s) { return std::stol(s); });
  return jiffiesLong[3]+jiffiesLong[4];
  }

// TODO: Read and return CPU utilization
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

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  //string line, key, value;
  int processes = GetValue<int>(stream, "processes");
  /*if (stream.is_open()) {
    while(key != "processes")
    {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> key >> value;    
    }
    processes = stoi(value);
  }*/
  return processes; 
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  //string line, key, value;
  int processes=GetValue<int>(stream, "procs_running");
  /*if (stream.is_open()) {
    while(key != "procs_running")
    {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> key >> value;    
    }
    processes = stoi(value);
  }*/
  return processes;
  }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory+to_string(pid)+kCmdlineFilename);
  string line, cmd;
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cmd;
  }
  return cmd; }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
   std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatusFilename);
  string line, key;
  float value = GetValue<float>(stream, "VmSize");
  /*if(stream.is_open()){
    while(std::getline(stream, line) && key!="VmSize"){
      //std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':',' ');
      std::istringstream linestream(line);
      linestream  >> key >> value;
    }
    value /=1000.f;
  }*/
  value /=1000.f;
  std::stringstream format;
  format << std::fixed << std::setprecision(2) << value;
  return format.str();
  }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatusFilename);
  //string line, key, uid;
  string uid = GetValue<string>(stream, "Uid");
  /*if(stream.is_open()){
    while(key!="Uid"){
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':',' ');
      std::istringstream linestream(line);
      linestream  >> key >> uid;
    }
  }*/
  return uid;
  }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
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

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kStatFilename);
  string line, value;
  //long utime, stime;
  long starttime;
  //auto utime=GetLongValue(stream, "14");
  //auto stime=GetLongValue(stream, "15");
  if(stream.is_open()){    
      std::getline(stream, line);
      std::replace(line.begin(), line.end(), ':',' ');
      std::istringstream linestream(line);
      int index=1;
      while(index < 23)
      {
        /*if(index == 14)
        linestream >> utime;
        else if(index == 15)
        linestream >> stime;*/
        if(index == 22)
        linestream >> starttime;
        else
        linestream >> value;
        index++;
      }
  }
  return /*(utime + stime)*/starttime/sysconf(_SC_CLK_TCK);
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
  //unused a failed experiment probably to delete at the end
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
          linestream >> value;
          position++;
        }while(position != index);
      }
    }
    return value;
  }

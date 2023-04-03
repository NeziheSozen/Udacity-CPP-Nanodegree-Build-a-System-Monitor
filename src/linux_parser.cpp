#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
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

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
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
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() 
{
  float total_memory = 0.0, free_memory = 0.0;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      string current_key;
      float current_value;
      if (linestream >> current_key >> current_value) {
        if (current_key == "MemTotal:") {
          total_memory = current_value;
        } else if (current_key == "MemFree:") {
          free_memory = current_value;
        }
      }
    }
  }
  return (total_memory > 0) ? (total_memory - free_memory) / total_memory : 0.0;
}

// Read and return the system uptime
long LinuxParser::UpTime() 
{
  long up_time = 0;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    if (stream >> up_time) {
      return up_time;
    }
  }
  return 0;
}


// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() 
{ 
  return 0;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) 
{ 
  return 0; 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() 
{ 
  return 0; 
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() 
{ 
  return 0; 
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() 
{ 
  return {}; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() 
{
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() 
{ 
  return 0; 
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) 
{ 
  return string(); 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) 
{ 
  return string(); 
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) 
{ 
  return string(); 
}

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) 
{ 
  return string(); 
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) 
{ 
  return 0; 
}

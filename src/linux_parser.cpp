#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream> 
#include <numeric>
#include <fstream> 

#include "linux_parser.h"


using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::istringstream;
using std::accumulate;
using std::ifstream;

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

//Helper method:
long getProcessActiveJiffies(int pid) {
  string fileName = LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename;
  ifstream stream(fileName);
  vector<string> values;

  if (stream.is_open()) {
    string line;
    getline(stream, line);

    istringstream linestream(line);
    string value;
    while (linestream >> value) {
      values.push_back(value);
    }
  }

  if (values.size() < 17) {
    // Handle error: not enough fields in values
    return -1;
  }

  long userTime = stol(values[13]);
  long systemTime = stol(values[14]);
  long childUserTime = stol(values[15]);
  long childSystemTime = stol(values[16]);

  long totalJiffies = userTime + systemTime + childUserTime + childSystemTime;

  return totalJiffies;
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
  return ActiveJiffies() + IdleJiffies(); 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) 
{ 
  long totalJiffies = LinuxParser::Jiffies();
  long activeJiffies = getProcessActiveJiffies(pid);

  if (totalJiffies <= 0) {
    // Handle error: totalJiffies is not positive
    return -1;
  }

  long activeJiffiesPercentage = (activeJiffies * 100) / totalJiffies;
  return activeJiffiesPercentage;
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

float LinuxParser::CpuUtilizationProcess(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (!stream.is_open()) {
    return 0.0f;
  }
  std::string line;
  std::getline(stream, line);
  std::istringstream linestream(line);
  std::vector<std::string> tokens(
      std::istream_iterator<std::string>{linestream},
      std::istream_iterator<std::string>{});

  float utime = stof(tokens[13]);
  float stime = stof(tokens[14]);
  float cutime = stof(tokens[15]);
  float cstime = stof(tokens[16]);
  float starttime = stof(tokens[21]);

  float total_time = utime + stime + cutime + cstime;
  float seconds = static_cast<float>(UpTime()) -
                  (starttime / static_cast<float>(sysconf(_SC_CLK_TCK)));
  float cpu_usage = total_time / (seconds * sysconf(_SC_CLK_TCK));

  return cpu_usage;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() 
{ 
  string key = "processes";
  string path = kProcDirectory + kStatFilename;
  return findValueByKey<int>(path,key);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() 
{ 
  string key = "procs_running";
  string path = kProcDirectory + kStatFilename;
  return findValueByKey<int>(path,key);
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) 
{ 
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::string command;
    std::getline(filestream, command);
    const std::string postfix = "...";
    constexpr int max_length = 25;
    if (command.size() > max_length) {
      command = command.substr(0, max_length) + postfix;
    }
    return command;
  }
  return "";
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) 
{ 
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    std::string line;
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      std::string key;
      if (linestream >> key) {
        if (key == "VmRSS:") {
          int ram_kb;
          if (linestream >> ram_kb) {
            // Convert from KB to MB
            constexpr int kb_per_mb = 1024;
            return std::to_string(ram_kb / kb_per_mb);
          }
        }
      }
    }
  }
  return "0"; 
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

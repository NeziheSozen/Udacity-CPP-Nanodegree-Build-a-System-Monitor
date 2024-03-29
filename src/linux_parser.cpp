#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream> 
#include <numeric>
#include <fstream> 
#include <iostream>

#include "linux_parser.h"


using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::istringstream;
using std::accumulate;
using std::ifstream;
using std::stringstream;

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

//helper method for splitting string
vector<string> split(const string& s, char delimiter) 
{
  vector<string> tokens;
  stringstream ss(s);
  string token;
  while (getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

//helper template method
template <typename T>
T findValueByKey(const std::string& pathName, const std::string& keyFilter) {
  std::ifstream filestream(pathName);
  if (!filestream.is_open()) {
    throw std::runtime_error("Cannot open file: " + pathName);
  }

  T value{};
  std::string line;
  while (std::getline(filestream, line)) {
    std::istringstream linestream(line);
    std::string key;
    if (linestream >> key && key == keyFilter) {
      if (!(linestream >> value)) {
        throw std::runtime_error("Failed to extract value for key " + keyFilter);
      }
      return value;
    }
  }

  throw std::runtime_error("Key " + keyFilter + " not found in file " + pathName);
}

// Helper template method
template <typename T> 
T getValueOfFile(std::string const& PathName)
{
  T value;

  std::ifstream filestream(PathName);
  if (filestream.is_open()) {
    std::istringstream linestream;
    linestream >> value;
  }

  return value;
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
  vector<string> cpu_utilization = CpuUtilization();
  vector<int> active_jiffies_indexes = {kUser_, kNice_, kSystem_, kIRQ_, kSoftIRQ_, kSteal_};
  long active_jiffies = 0;
  for (int index : active_jiffies_indexes) {
    active_jiffies += stol(cpu_utilization[index]);
  }
  return active_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() 
{ 
  vector<string> cpu_utilization = CpuUtilization();
  float idle_jiffies = stof(cpu_utilization[kIdle_]) + stof(cpu_utilization[kIOwait_]);
  return idle_jiffies;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() 
{ 
  vector<string> jiffies;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line;
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      string cpu_label;
      linestream >> cpu_label;
      if (cpu_label == "cpu") {
        std::copy(std::istream_iterator<string>(linestream),
                  std::istream_iterator<string>(),
                  std::back_inserter(jiffies));
      }
    }
  }
  return jiffies;
}

float LinuxParser::CpuUtilizationProcess(int pid) {
  std::string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);

  float utime = 0.0;
  float stime = 0.0;
  float cutime = 0.0;
  float cstime = 0.0;
  float starttime = 0.0;

  auto Hertz = sysconf(_SC_CLK_TCK);
  if (Hertz == 0) return 0;

  int pos = 1;

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<float> values(23);
    for (int i = 0; i < 23; i++) {
      linestream >> values[i];
    }

    utime = values[13];
    stime = values[14];
    cutime = values[15];
    cstime = values[16];
    starttime = values[21];
  }

  // To protect division by 0
  if (pos < 22) {
    return 0;
  }

  float total_time = utime + stime + cutime + cstime;
  float seconds = UpTime() - (starttime / (float)Hertz);

  // To protect division by 0
  if (seconds == 0) return 0;

  float cpu_usage = total_time / (float)Hertz / seconds;

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

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) 
{ 
  std::string path = kProcDirectory + std::to_string(pid) + kStatusFilename;
  return findValueByKey<std::string>(path, "Uid:");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) 
{ 
  string uid = Uid(pid);
  string user = "unknown";
  
  // Read /etc/passwd file and extract username by UID
  string passwd_file = "/etc/passwd";
  ifstream filestream(passwd_file);
  if (filestream.is_open()) {
    string line;
    while (getline(filestream, line)) {
      vector<string> tokens = split(line, ':');
      if (tokens.size() >= 3 && tokens[2] == uid) {
        user = tokens[0];
        break;
      }
    }
  }
  
  // Truncate username if it's too long
  const int kMaxUsernameLength = 10;
  if (user.length() > kMaxUsernameLength) {
    user = user.substr(0, kMaxUsernameLength - 2) + "..";
  }
  
  return user;
}

// Read and return the system uptime
long LinuxParser::UpTime() 
{
long up_time = 0;
  std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kUptimeFilename);
  if (stream.is_open())
  {
    if (stream >> up_time)
    {
      return up_time;
    }
  }
  return 0;
}

// Read and return the UpTime with a process
long LinuxParser::UpTime(int pid) 
{ 
  std::string line;
  std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) + LinuxParser::kStatFilename);
  std::vector<std::string> tokens;

  if (stream.is_open())
  {
    if (std::getline(stream, line))
    {
      std::istringstream linestream(line);
      std::string token;
      while (linestream >> token)
      {
        tokens.push_back(token);
      }
    }
  }

  try {
    if (!tokens.empty()) {
      long starttime = std::stol(tokens[21]); // get the starttime value

      // Check Linux kernel version to determine the units of starttime
      long uptime;
      auto k = stof(LinuxParser::Kernel());
      if (k< 2.6) {
        uptime = starttime;  // expressed in jiffies
      } else {
        uptime = starttime / sysconf(_SC_CLK_TCK); // expressed in clock ticks
      }

      long system_uptime = LinuxParser::UpTime(); // get the system uptime
      return system_uptime - uptime; // subtract process starttime from system uptime to get process uptime
    }
  } catch (const std::invalid_argument &error) {
    std::cerr << error.what() << std::endl;
  }

  return 0;
}

// Read and return the RAM
int LinuxParser::GetRam(int pid)
{
  std::string ram = LinuxParser::Ram(pid);
  return std::stoi(ram);
}

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

Process::Process(int pid) { 
    this->process_id_        = pid; 
    this->ram_         		 = LinuxParser::Ram(pid);
    this->command_     		 = LinuxParser::Command(pid);
    this->user_        		 = LinuxParser::User(pid);
    this->up_time_      	 = LinuxParser::UpTime(pid);
    this->cpu_utilization_   =  0.0;
    
}

// Return this process's ID
int Process::Pid() 
{ 
  return this->process_id_; 
}

// Return this process's CPU utilization
float Process::CpuUtilization() { 
    long ProcesUptime       = LinuxParser::UpTime(process_id_);
    long TotalTimeActive    = LinuxParser::ActiveJiffies(process_id_);

    this->cpu_utilization_ = float(TotalTimeActive)/float(ProcesUptime);

    return this->cpu_utilization_; 
}

// Return the command that generated this process
string Process::Command() 
{ 
  return this->command_; 
}

// Return this process's memory utilization
string Process::Ram() 
{ 
  return this->ram_; 
}

// Return the user (name) that generated this process
string Process::User() 
{
  return this->user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() 
{ 
  return this->up_time_; 
}

//Overload less than.
bool Process::operator<(Process const& a) const 
{ 
    return this->cpu_utilization_ < a.cpu_utilization_;
}

//Overload greater than.
bool Process::operator>(Process const& a) const 
{ 
    return this->cpu_utilization_ > a.cpu_utilization_;
}

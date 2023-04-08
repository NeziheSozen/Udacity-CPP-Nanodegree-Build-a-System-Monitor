#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"
#include "format.h"

using namespace std;


Processor& System::Cpu() 
{ 
    return cpu_; 
}

vector<Process>& System::Processes() { 
    processes_.clear(); 
    const vector<int> &pids = LinuxParser::Pids(); 

    for(const int &pid : pids) {
        int ram = LinuxParser::GetRam(pid); 
        if(ram > 0) {
            processes_.emplace_back(pid);
        }
    }

    std::sort(processes_.begin(), processes_.end(), std::greater<Process>()); // Sort the vector in descending order.

    return processes_; 
}

std::string System::Kernel() 
{ 
    return LinuxParser::Kernel(); 
}

float System::MemoryUtilization() 
{ 
    return LinuxParser::MemoryUtilization(); 
}

std::string System::OperatingSystem() 
{ 
    return LinuxParser::OperatingSystem(); 
}

int System::RunningProcesses() 
{ 
    return LinuxParser::RunningProcesses(); 
}

int System::TotalProcesses() 
{ 
    return LinuxParser::TotalProcesses(); 
}

long int System::UpTime() 
{ 
    return (long int)(LinuxParser::UpTime());
}
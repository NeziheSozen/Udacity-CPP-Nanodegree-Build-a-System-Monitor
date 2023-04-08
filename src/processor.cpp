#include "processor.h"
#include "linux_parser.h"

using namespace std;

Processor::Processor()
{
    this->idle_jiffies_     = LinuxParser::IdleJiffies();
    this->active_jiffies_   = LinuxParser::ActiveJiffies();
    this->jiffies_          = LinuxParser::Jiffies();
}

float Processor::Utilization() 
{ 
    float cpu_utilization;
    long const idle_jiffies       = LinuxParser::IdleJiffies();
    long const active_jiffies     = LinuxParser::ActiveJiffies();
    long const jiffies            = LinuxParser::Jiffies();

    long delta_idle               = idle_jiffies - this->idle_jiffies_ ;
    long delta_total              = jiffies - this->jiffies_;
    
    UpdateProcessor(idle_jiffies, active_jiffies, jiffies);
    
    if(delta_total == 0) {
        cpu_utilization = 0.0f;
    }
    else{
        cpu_utilization = (delta_total - delta_idle)*1.0/delta_total;
    }
    
    return cpu_utilization; 
}

void Processor::UpdateProcessor(long idle_jiffies, long active_jiffies, long jiffies)
{
    this->idle_jiffies_     = idle_jiffies;
    this->active_jiffies_   = active_jiffies;
    this->jiffies_          = jiffies;
}
#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  Processor();
  float Utilization();  
  void UpdateProcessor(long idle_jiffies, long active_jiffies, long jiffies);


 private:
  
  long idle_jiffies_;
  long active_jiffies_;
  long jiffies_;
};

#endif
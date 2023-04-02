#include <string>

#include "format.h"

using std::string;

void convertUnit(std::string &input, long unit)
{
  if (unit < 10){
    input = "0" + std::to_string(unit);
  }
  else {
    input = std::to_string(unit);
  }
}

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds)
{ 
  long second = seconds % 60;
  long minute = (seconds / 60) % 60;
  long hour = (seconds / (60 * 60)) % 24;

  std::string ss = "";
  std::string mm = "";
  std::string hh = "";

  convertUnit(ss, second);
  convertUnit(mm, minute);
  convertUnit(hh, hour);

  return string(hh + ":" + mm + ":" + ss);

}
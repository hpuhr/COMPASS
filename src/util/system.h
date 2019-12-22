#ifndef SYSTEM_H
#define SYSTEM_H

//#include <sys/sysinfo.h>
//#include <stdint.h>

#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include "logger.h"

const double megabyte = 1024 * 1024;
const double gigabyte = 1024 * 1024 * 1024;

namespace Utils
{

namespace System
{

float getFreeRAMinGB ()
{
//    struct sysinfo info;
//    sysinfo (&info);

//    return ((uint64_t) (info.freeram + info.bufferram) * info.mem_unit)/gigabyte;

    std::string token;
       std::ifstream file("/proc/meminfo");
       while(file >> token) {
           if(token == "MemAvailable:") {
               unsigned long mem;

               if(file >> mem) // returns in kB
               {
                   return mem / megabyte;
               }
               else
               {
                   return 0;
               }
           }
           // ignore rest of the line
           file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
       }
       return 0; // nothing found
}

}
}

#endif // SYSTEM_H

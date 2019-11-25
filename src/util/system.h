#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/sysinfo.h>
#include <stdint.h>

const double megabyte = 1024 * 1024;
const double gigabyte = 1024 * 1024 * 1024;

namespace Utils
{

namespace System
{

float getFreeRAMinGB ()
{
    struct sysinfo info;
    sysinfo (&info);

    return ((uint64_t) info.freeram * info.mem_unit)/gigabyte;
}

}
}

#endif // SYSTEM_H

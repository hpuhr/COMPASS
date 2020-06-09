#ifndef SYSTEM_H
#define SYSTEM_H

//#include <sys/sysinfo.h>
//#include <stdint.h>

#include <sstream>

namespace Utils
{
namespace System
{
extern float getFreeRAMinGB();

extern std::string exec(const std::string& cmd);

}  // namespace System
}  // namespace Utils

#endif  // SYSTEM_H

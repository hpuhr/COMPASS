#ifndef DATA_H
#define DATA_H

#include <memory>

class DBOVariableSet;
class Buffer;

namespace Utils
{

namespace Data
{

    void finalizeBuffer (DBOVariableSet &read_list, std::shared_ptr<Buffer> buffer);

}
}

#endif // DATA_H

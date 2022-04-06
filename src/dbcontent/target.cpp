#include "target.h"

namespace dbContent {


Target::Target(unsigned int utn, nlohmann::json info)
    : utn_(utn), info_(info)
{

}

}

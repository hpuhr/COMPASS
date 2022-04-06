#ifndef DBCONTENT_TARGET_H
#define DBCONTENT_TARGET_H

#include "json.hpp"

namespace dbContent {

class Target
{
public:
    Target(unsigned int utn, nlohmann::json info);

    unsigned int utn() { return utn_; }
    nlohmann::json info()  { return info_; }

protected:
    unsigned int utn_ {0};
    nlohmann::json info_;
};

}
#endif // DBCONTENT_TARGET_H

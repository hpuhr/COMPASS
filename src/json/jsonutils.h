#ifndef JSONUTILS_H
#define JSONUTILS_H

#include "json.hpp"

namespace Utils
{

namespace JSON
{

inline std::string toString(const nlohmann::json& j)
{
    if (j.type() == nlohmann::json::value_t::string) {
        return j.get<std::string>();
    }

    return j.dump();
}

}
}

#endif // JSONUTILS_H

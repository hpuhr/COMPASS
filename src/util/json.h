#ifndef JSON_H
#define JSON_H

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

        extern bool canFindKey (const nlohmann::json& j, const std::vector<std::string>& keys);
        extern const nlohmann::json& findKey (const nlohmann::json& j, const std::vector<std::string>& keys);
        extern const nlohmann::json& findParentKey (const nlohmann::json& j, const std::vector<std::string>& keys);

        extern void applyFunctionToValues (nlohmann::json& j, const std::vector<std::string>& keys,
                                    std::vector<std::string>::const_iterator current_key_it,
                                    std::function<void(nlohmann::json&)> function, bool required);
//        extern void applyFunctionToValues (nlohmann::json& j, const std::vector<std::string>& keys,
//                                    std::function<void(nlohmann::json&)> function, bool required);
    }
}

#endif // JSON_H

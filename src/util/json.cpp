#include "json.h"
#include "logger.h"

namespace Utils
{
    namespace JSON
    {

        bool canFindKey (const nlohmann::json& j, const std::vector<std::string>& keys)
        {
            if (!keys.size())
                return false;

            std::vector<std::string>::const_iterator last_key = keys.end()-1;

            const nlohmann::json* val_ptr = &j;

            for (auto sub_it=keys.cbegin(); sub_it != keys.cend(); ++sub_it)
            {
                if (val_ptr->contains (*sub_it))
                {
                    if (sub_it == last_key) // last found
                        return true;

                    if (val_ptr->at(*sub_it).is_object()) // not last, step in
                        val_ptr = &val_ptr->at(*sub_it);
                    else // not last key, and not object
                        return false;
                }
                else // not found
                    return false;
            }

            return false;
        }

        const nlohmann::json& findKey (const nlohmann::json& j, const std::vector<std::string>& keys)
        {
            assert (keys.size());
            std::vector<std::string>::const_iterator last_key = keys.end()-1;

            const nlohmann::json* val_ptr = &j;

            for (auto sub_it=keys.cbegin(); sub_it != keys.cend(); ++sub_it)
            {
                if (val_ptr->contains (*sub_it))
                {
                    if (sub_it == last_key) // last found
                    {
                        val_ptr = &val_ptr->at(*sub_it);
                        return *val_ptr;
                    }

                    if (val_ptr->at(*sub_it).is_object()) // not last, step in
                        val_ptr = &val_ptr->at(*sub_it);
                    else // not last key, and not object
                        throw std::runtime_error("Utils: JSON: findKey: key '"+*sub_it+"' not found");
                }
                else // not found
                    throw std::runtime_error("Utils: JSON: findKey: key '"+*sub_it+"' not found");
            }

            throw std::runtime_error("Utils: JSON: findKey: keys empty");
        }

        const nlohmann::json& findParentKey (const nlohmann::json& j, const std::vector<std::string>& keys)
        {
            const nlohmann::json* val_ptr = &j;

            assert (keys.size() > 1);
            std::vector<std::string>::const_iterator second_to_last_key = keys.end()-2;

            for (auto sub_it=keys.begin(); sub_it != keys.end(); ++sub_it)
            {
                if (val_ptr->contains (*sub_it))
                {
                    if (sub_it == second_to_last_key) // second to last found
                    {
                        val_ptr = &val_ptr->at(*sub_it);
                        break;
                    }

                    if (val_ptr->at(*sub_it).is_object()) // not second to last, step in
                        val_ptr = &val_ptr->at(*sub_it);
                    else // not last key, and not object
                        throw std::runtime_error("Utils: JSON: findParentKey: key '"+*sub_it+"' not found");
                }
                else // not found
                    throw std::runtime_error("Utils: JSON: findParentKey: key '"+*sub_it+"' not found");
            }

            assert (val_ptr);
            return *val_ptr;
        }

        void applyFunctionToValues (nlohmann::json& j, const std::vector<std::string>& keys,
                                    std::vector<std::string>::const_iterator current_key_it,
                                    std::function<void(nlohmann::json&)> function, bool required)
        {
//            assert (keys.size());

//            loginf << "Utils: JSON: applyFunctionToValues: current_key '" << *current_key_it << "' data '"
//                   << j.dump(4) << "'";

            if (current_key_it == keys.end()) // no more keys
            {
                //loginf << "Utils: JSON: applyFunctionToValues: applying to '" << j.dump(4) << "'";
                function(j);
                return;
            }

            // not last key

            if (j.contains(*current_key_it))
            {
                //loginf << "Utils: JSON: applyFunctionToValues: contains";
                nlohmann::json& value = j.at(*current_key_it);

                if (value.is_object())
                {
                    //loginf << "Utils: JSON: applyFunctionToValues: recursing into object";
                    applyFunctionToValues (value, keys, current_key_it+1, function, required);
                }
                else if (value.is_array())
                {
                    //loginf << "Utils: JSON: applyFunctionToValues: recursing into array";

                    for (auto& value_it : value)
                        applyFunctionToValues (value_it, keys, current_key_it+1, function, required);
                }
                else
                {
                    if (required)
                        throw std::runtime_error("Utils: String: applyFunctionToValues: key '"+*current_key_it
                                                 +"' stopped at unsupported value type");
                    else
                        return;
                }
            }
            else
            {
                //loginf << "Utils: JSON: applyFunctionToValues: not contained";

                if (required)
                    throw std::runtime_error("Utils: String: applyFunctionToValues: key '"+*current_key_it
                                             +"' not found");
                else
                    return;
            }
        }

//        void applyFunctionToValues (nlohmann::json& j, const std::vector<std::string>& keys,
//                                    std::function<void(nlohmann::json&)> function, bool required)
//        {
//            applyFunctionToValues(j, keys, keys.begin(), function, required);
//        }
    }
}

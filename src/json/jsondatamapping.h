#ifndef JSONDATAMAPPING_H
#define JSONDATAMAPPING_H

#include "json.hpp"
#include "logger.h"
#include "format.h"
#include "nullablevector.h"
#include "jsonutils.h"

class DBOVariable;

class JSONDataMapping
{
public:
    JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory);

    JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                Format json_value_format);

    JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                const std::string& dimension, const std::string& unit);

    JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                Format json_value_format, const std::string& dimension, const std::string& unit);

//    bool hasKey (const nlohmann::json& j)
//    {
//        if (has_sub_keys_)
//        {
//            const nlohmann::json* k = &j;
//            std::string sub_key;

//            for (unsigned int cnt=0; cnt < num_sub_keys_; ++cnt)
//            {
//                sub_key = sub_keys_.at(cnt);

//                if (k->find (sub_key) != k->end())
//                {
//                    if (cnt == num_sub_keys_-1)
//                        break;

//                    if (k->at(sub_key).is_object())
//                        k = &k->at(sub_key);
//                    else
//                        return false;
//                }
//                else
//                    return false;
//            }
//            return true;
//        }
//        else
//            return j.find (json_key_) != j.end();
//    }

//    bool isNull (const nlohmann::json& j)
//    {
//        if (has_sub_keys_)
//        {
//            const nlohmann::json* k = &j;
//            std::string sub_key;

//            for (unsigned int cnt=0; cnt < num_sub_keys_; ++cnt)
//            {
//                sub_key = sub_keys_.at(cnt);

//                if (k->find (sub_key) != k->end())
//                {
//                    if (cnt == num_sub_keys_-1)
//                        return k->at(sub_key).is_null();

//                    if (k->at(sub_key).is_object())
//                        k = &k->at(sub_key);
//                    else
//                        return true;
//                }
//                else
//                    return true;
//            }
//            return true;
//        }
//        else
//        {
//            if (j.find (json_key_) != j.end())
//                return j.at(json_key_) == nullptr;
//            else
//                return true;
//        }
//    }

//    inline const nlohmann::json& getValue (const nlohmann::json& j, unsigned int index=0)
//    {
//        if (has_sub_keys_)
//        {
//            std::string& sub_key = sub_keys_.at(index);

//            if (j.find (sub_key) != j.end())
//            {
//                if (index == num_sub_keys_-1)
//                    return j.at(sub_key);

//                if (j.at(sub_key).is_object())
//                    return getValue(j.at(sub_key), index+1);
//                else
//                    return nullptr_;
//            }
//        }
//        else
//        {
//            if (j.find (json_key_) != j.end())
//                return j.at(json_key_);
//            else
//                return nullptr_;
//        }
//        return nullptr_;
//    }

//    template<typename T>
//    void setValue(NullableVector<T>& array_list, unsigned int row_cnt, const nlohmann::json& j)
//    {
//        if (j == nullptr)
//            array_list.setNull(row_cnt);
//        else
//        {
//            try
//            {
//                if (json_value_format_ == "")
//                    array_list.set(row_cnt, j);
//                else
//                    array_list.setFromFormat(row_cnt, json_value_format_,
//                                             toString(j));

//                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << j
//                       << " buffer " << array_list.get(row_cnt);
//            }
//            catch (nlohmann::json::exception& e)
//            {
//                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
//                array_list.setNull(row_cnt);
//            }
//        }
//    }

//    void setValue(NullableVector<char>& array_list, unsigned int row_cnt, const nlohmann::json& j)
//    {
//        if (j == nullptr)
//            array_list.setNull(row_cnt);
//        else
//        {
//            try
//            {
//                if (json_value_format_ == "")
//                    array_list.set(row_cnt, static_cast<int> (j));
//                else
//                    array_list.setFromFormat(row_cnt, json_value_format_,
//                                             toString(j));

//                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << j
//                       << " buffer " << array_list.get(row_cnt);
//            }
//            catch (nlohmann::json::exception& e)
//            {
//                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
//                array_list.setNull(row_cnt);
//            }
//        }
//    }

    // return bool mandatory missing
    template<typename T>
    bool findAndSetValue(const nlohmann::json& j, NullableVector<T>& array_list, unsigned int row_cnt) const
    {
        const nlohmann::json* val_ptr = &j;

        if (has_sub_keys_)
        {
            for (const std::string& sub_key : sub_keys_)
            {
                if (val_ptr->find (sub_key) != val_ptr->end())
                {
                    if (sub_key == sub_keys_.back()) // last found
                    {
                        val_ptr = &val_ptr->at(sub_key);
                        break;
                    }

                    if (val_ptr->at(sub_key).is_object()) // not last, step in
                        val_ptr = &val_ptr->at(sub_key);
                    else // not last key, and not object
                    {
                        val_ptr = nullptr;
                        break;
                    }
                }
                else // not found
                {
                    val_ptr = nullptr;
                    break;
                }
            }
        }
        else
        {
            if (val_ptr->find (json_key_) != val_ptr->end())
                val_ptr = &val_ptr->at(json_key_);
            else
                val_ptr = nullptr;
        }

        if (val_ptr == nullptr || *val_ptr == nullptr)
        {
            if (mandatory_)
                return true;

            //array_list.setNull(row_cnt);
            return false;
        }
        else
        {
            try
            {
                T tmp = *val_ptr;
                if (json_value_format_ == "")
                    array_list.set(row_cnt, tmp);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(tmp));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << *val_ptr
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNull(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool findAndSetValue(const nlohmann::json& j, NullableVector<char>& array_list, unsigned int row_cnt) const
    {
        const nlohmann::json* val_ptr = &j;

        if (has_sub_keys_)
        {
            for (const std::string& sub_key : sub_keys_)
            {
                if (val_ptr->find (sub_key) != val_ptr->end())
                {
                    if (sub_key == sub_keys_.back()) // last found
                    {
                        val_ptr = &val_ptr->at(sub_key);
                        break;
                    }

                    if (val_ptr->at(sub_key).is_object()) // not last, step in
                        val_ptr = &val_ptr->at(sub_key);
                    else // not last key, and not object
                    {
                        val_ptr = nullptr;
                        break;
                    }
                }
                else // not found
                {
                    val_ptr = nullptr;
                    break;
                }
            }
        }
        else
        {
            if (val_ptr->find (json_key_) != val_ptr->end())
                val_ptr = &val_ptr->at(json_key_);
            else
                val_ptr = nullptr;
        }

        if (val_ptr == nullptr || *val_ptr == nullptr)
        {
            if (mandatory_)
                return true;

            //array_list.setNull(row_cnt);
            return false;
        }
        else
        {
            try
            {
                char tmp = static_cast<int> (*val_ptr);
                if (json_value_format_ == "")
                    array_list.set(row_cnt, tmp);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(tmp));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << *val_ptr
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNull(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool hasDimension () const { return dimension_.size() > 0; }
    /// @brief Returns dimension contained in the column
    const std::string &dimension () const { return dimension_; }
    /// @brief Returns unit
    const std::string &unit () const { return unit_; }

private:
    std::string json_key_;
    DBOVariable& variable_;
    bool mandatory_;
    Format json_value_format_;

    /// Unit dimension
    std::string dimension_;
    /// Unit
    std::string unit_;

    bool has_sub_keys_ {false};
    std::vector<std::string> sub_keys_;
    unsigned int num_sub_keys_;

    nlohmann::json nullptr_ = nullptr;

public:
    std::string jsonKey() const;
    void jsonKey(const std::string &json_key);

    DBOVariable& variable() const;

    bool mandatory() const;
    void mandatory(bool mandatory);

    Format jsonValueFormat() const;
    void jsonValueFormat(const Format &json_value_format);
};

#endif // JSONDATAMAPPING_H

/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSONDATAMAPPING_H
#define JSONDATAMAPPING_H

#include "json.hpp"
#include "logger.h"
#include "format.h"
#include "nullablevector.h"
#include "jsonutils.h"
#include "configurable.h"
#include "jsondatamappingwidget.h"

#include <memory>

class DBOVariable;
class JSONObjectParser;

class JSONDataMapping : public Configurable
{
public:
    JSONDataMapping (const std::string& class_id, const std::string& instance_id, JSONObjectParser& parent);
    JSONDataMapping() = default;
    JSONDataMapping(JSONDataMapping&& other) { *this = std::move(other); }

    virtual ~JSONDataMapping();

    /// @brief Move constructor
    JSONDataMapping& operator=(JSONDataMapping&& other);

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
                logdbg << "JSONDataMapping: findAndSetValue: key " << json_key_ << " json " << val_ptr->type_name()
                       << " '" << val_ptr->dump() << "' format '" << json_value_format_ << "'";

                //T tmp = val_ptr->get<T>();
                if (json_value_format_ == "")
                    array_list.set(row_cnt, *val_ptr);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

                logdbg << "JSONDataMapping: findAndSetValue: key " << json_key_ << " json " << *val_ptr
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JSONDataMapping: findAndSetValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNull(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool findAndSetValue(const nlohmann::json& j, NullableVector<bool>& array_list, unsigned int row_cnt) const
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
                bool tmp_bool;

                if (val_ptr->is_number())
                {
                    unsigned int tmp = *val_ptr;
                    assert (tmp == 0 || tmp == 1);
                    tmp_bool = static_cast<bool> (tmp);
                }
                else
                {
                    tmp_bool = *val_ptr; // works for bool, throws for rest
                }

                if (json_value_format_ == "")
                    array_list.set(row_cnt, tmp_bool);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             Utils::JSON::toString(tmp_bool));

                logdbg << "JSONDataMapping: findAndSetValue(bool): json " << tmp_bool << " buffer "
                       << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JSONDataMapping: findAndSetValue(bool): key " << json_key_ << " json exception "
                         << e.what();
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
                if (json_value_format_ == "")
                    array_list.set(row_cnt, static_cast<int> (*val_ptr));
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             Utils::JSON::toString(static_cast<int> (*val_ptr)));

                logdbg << "JSONDataMapping: findAndSetValue(char): json " << static_cast<int> (*val_ptr) << " buffer "
                       << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JSONDataMapping: findAndSetValue(char): key " << json_key_ << " json exception "
                         << e.what();
                array_list.setNull(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool findAndSetValue(const nlohmann::json& j, NullableVector<std::string>& array_list, unsigned int row_cnt) const
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
                if (json_value_format_ == "")
                    array_list.set(row_cnt, Utils::JSON::toString(*val_ptr));
                else
                    array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

                logdbg << "JSONDataMapping: findAndSetValue(string): json " << Utils::JSON::toString(*val_ptr)
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JSONDataMapping: findAndSetValue(string): key " << json_key_ << " json exception "
                         << e.what();
                array_list.setNull(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool hasDimension () const { return dimension_.size() > 0; }
    /// @brief Returns dimension contained in the column
    std::string& dimensionRef () { return dimension_; }
    const std::string& dimension () const { return dimension_; }
    /// @brief Returns unit
    std::string& unitRef () { return unit_; }
    const std::string& unit () const { return unit_; }

    const std::string& jsonKey() const;
    void jsonKey(const std::string &json_key);

    bool active() const;
    void active(bool active);

    bool hasVariable () { return variable_ != nullptr; }
    DBOVariable& variable() const;

    bool mandatory() const;
    void mandatory(bool mandatory);

    Format jsonValueFormat() const;
    Format& jsonValueFormatRef();
    //void jsonValueFormat(const Format &json_value_format);

    std::string dbObjectName() const;

    void dboVariableName(const std::string& name);
    std::string dboVariableName() const;

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id) {}

    //JSONDataMappingWidget* widget ();

    void initializeIfRequired ();

    std::string& formatDataTypeRef();

    bool initialized() const;

    std::string comment() const;
    void comment(const std::string &comment);

private:
    bool initialized_ {false};

    bool active_ {false};
    std::string json_key_;

    std::string db_object_name_;
    std::string dbovariable_name_;
    DBOVariable* variable_ {nullptr};

    bool mandatory_ {false};

    std::string comment_;

    std::string format_data_type_;
    Format json_value_format_;
    //std::unique_ptr<Format> json_value_format_;

    /// Unit dimension
    std::string dimension_;
    /// Unit
    std::string unit_;

    bool has_sub_keys_ {false};
    std::vector<std::string> sub_keys_;
    unsigned int num_sub_keys_;

    std::unique_ptr<JSONDataMappingWidget> widget_;

    void initialize ();

protected:
    virtual void checkSubConfigurables () {}
};

Q_DECLARE_METATYPE(JSONDataMapping*)

#endif // JSONDATAMAPPING_H

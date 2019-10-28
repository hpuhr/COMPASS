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

#ifndef ASTERIXCATEGORYCONFIG_H
#define ASTERIXCATEGORYCONFIG_H

#include <string>

#include "configurable.h"

class ASTERIXCategoryConfig : public Configurable
{
public:
    ASTERIXCategoryConfig (unsigned int category, const std::string& class_id, const std::string& instance_id,
                           Configurable* parent)
        : Configurable (class_id, instance_id,parent), category_(category)
    {
        registerParameter("category", &category_, 0);
        registerParameter("decode", &decode_, false);
        registerParameter("edition", &edition_, "");
        registerParameter("ref", &ref_, "");
        registerParameter("spf", &spf_, "");
    }

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id) {}

    unsigned int category() const
    {
        return category_;
    }

    bool decode() const
    {
        return decode_;
    }
    void decode(bool decode)
    {
        decode_ = decode;
    }

    std::string edition() const
    {
        return edition_;
    }

    void edition(const std::string& edition)
    {
        edition_ = edition;
    }

    std::string ref() const
    {
        return ref_;
    }

    void ref(const std::string& ref)
    {
        ref_ = ref;
    }

    std::string spf() const
    {
        return spf_;
    }

    void spf(const std::string& spf)
    {
        spf_ = spf;
    }

private:
    unsigned int category_;
    bool decode_ {false};
    std::string edition_;
    std::string ref_;
    std::string spf_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // ASTERIXCATEGORYCONFIG_H



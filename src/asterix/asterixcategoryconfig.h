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
        : Configurable (class_id, instance_id,parent)
    {
        registerParameter("category", &category_, 0);
        registerParameter("decode", &decode_, false);
        registerParameter("edition", &edition_, "");
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

    void edition(const std::string &edition)
    {
        edition_ = edition;
    }

private:
    unsigned int category_;
    bool decode_ {false};
    std::string edition_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // ASTERIXCATEGORYCONFIG_H

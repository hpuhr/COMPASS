/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "result/report/sectionid.h"

#include "stringconv.h"

#include <QString>

namespace ResultReport
{

const std::string SectionID::Sep = ":";

/**
*/
std::vector<std::string> SectionID::subSections(const std::string& section_id) 
{
    return Utils::String::split(section_id, Sep[ 0 ]);
}

/**
*/
std::string SectionID::sectionID(const std::vector<std::string>& sub_sections)
{
    return Utils::String::compress(sub_sections, Sep[ 0 ]);
}

/**
*/
std::string SectionID::sectionID(const std::string& section0, const std::string& section1)
{
    assert(!section0.empty() && !section1.empty());

    return (section0 + Sep + section1);
}

/**
*/
std::string SectionID::sectionID2Path(const std::string& section_id)
{
    std::string p = section_id;

    boost::replace_all(p, ":", "/");
    boost::replace_all(p, " ", "_");

    return (p + "/");
}

}

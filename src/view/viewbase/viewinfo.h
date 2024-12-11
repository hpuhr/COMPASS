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

#pragma once

#include <string>
#include <vector>

/**
 */
class ViewInfos
{
public:
    struct ViewInfo
    {
        enum class Style
        {
            None = 0,
            Bold,
            Italic
        };

        ViewInfo(const std::string& vi_id,
                 const std::string& n, 
                 const std::string& v,
                 bool section,
                 Style n_style = Style::None,
                 Style v_style = Style::None)
        :   id(vi_id), name(n), value(v), is_section(section), name_style(n_style), value_style(v_style) {}

        bool isInfo() const { return !is_section && !isEmpty(); }
        bool isSection() const { return is_section && !isEmpty(); }
        bool isEmpty() const { return name.empty() && value.empty(); }

        std::string id;
        std::string name;
        std::string value; 
        bool        is_section  = false;
        Style       name_style  = Style::None;
        Style       value_style = Style::None;
    };

    typedef ViewInfo::Style Style;

    ViewInfos() = default;
    virtual ~ViewInfos() = default;

    void clear();

    ViewInfos& addSection(const std::string& name);
    ViewInfos& addInfo(const std::string& id,
                       const std::string& name, 
                       const std::string& value,
                       bool value_italic = false);
    ViewInfos& addSpace();
    ViewInfos& addInfos(const ViewInfos& other);

    const std::vector<ViewInfo>& infos() const;

    size_t numSections() const;
    size_t numInfos() const;

private:
    std::vector<ViewInfo> infos_;

    size_t num_infos_    = 0;
    size_t num_sections_ = 0;
};

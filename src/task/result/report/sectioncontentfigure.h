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

#include "task/result/report/sectioncontent.h"

#include <QObject>

#include "json.hpp"

namespace ResultReport
{

/**
 */
class SectionContentFigure : public QObject, public SectionContent
{
    Q_OBJECT

public slots:
    void viewSlot();

public:
SectionContentFigure(const std::string& name, 
                     const std::string& caption,
                     std::function<std::shared_ptr<nlohmann::json::object_t>(void)> viewable_fnc,
                     Section* parent_section, 
                     TaskManager& eval_man,
                     int render_delay_msec = 0);

    virtual void addToLayout (QVBoxLayout* layout) override;
    virtual void accept(LatexVisitor& v) override;

    void view () const;
    std::string getSubPath() const;

protected:
    std::string caption_;
    int         render_delay_msec_ = 0;

    std::function<std::shared_ptr<nlohmann::json::object_t>(void)> viewable_fnc_;
};

}

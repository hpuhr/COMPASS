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

#include "task/result/report/report.h"
#include "task/result/report/section.h"
#include "task/result/report/sectionid.h"

#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace ResultReport
{

/**
 */
Report::Report(TaskManager& task_man)
:   TreeItem("Report", nullptr)
,   task_man_(task_man)
{
    root_section_ = std::make_shared<Section>("Results", "", this, task_man_);
}

/**
 */
Report::~Report()
{
    logdbg << "Report: destructor";
}

/**
 */
TreeItem* Report::child(int row)
{
    if (row < 0 || row > 0)
        return nullptr;

    return root_section_.get();
}

/**
 */
int Report::childCount() const
{
    return 1;
}

/**
 */
int Report::columnCount() const
{
    return 1;
}

/**
 */
QVariant Report::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

/**
 */
int Report::row() const
{
    return 0;
}

/**
 */
std::shared_ptr<Section> Report::rootSection()
{
    return root_section_;
}

/**
 */
std::vector<std::shared_ptr<Section>> Report::sections() const
{
    return std::vector<std::shared_ptr<Section>>();
}

/**
 */
std::vector<std::shared_ptr<SectionContentFigure>> Report::figures() const
{
    return std::vector<std::shared_ptr<SectionContentFigure>>();
}

/**
 */
Section& Report::getSection (const std::string& id)
{
    logdbg << "Report: getSection: id '" << id << "'";

    assert (id.size());

    std::vector<std::string> parts = SectionID::subSections(id);
    assert (parts.size());

    Section* tmp = nullptr;

    for (unsigned int cnt=0; cnt < parts.size(); ++cnt)
    {
        std::string& heading = parts.at(cnt);

        if (cnt == 0) // first
        {
            if (!root_section_->hasSubSection(heading))
                root_section_->addSubSection(heading);

            tmp = &root_section_->getSubSection(heading);
        }
        else // previous section
        {
            assert (tmp);

            if (!tmp->hasSubSection(heading))
                tmp->addSubSection(heading);

            tmp = &tmp->getSubSection(heading);
        }
    }

    assert (tmp);
    return *tmp;
}

/**
 */
nlohmann::json Report::toJSON() const
{
    nlohmann::json root;

    return root;
}

/**
 */
bool Report::fromJSON(const nlohmann::json& j)
{
    return true;
}

}

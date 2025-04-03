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

#include "task/result/report/treeitem.h"

#include <memory>

#include <QObject>

#include "json.hpp"

class TaskManager;

namespace ResultReport
{

class Section;
class SectionContent;


/**
 */
class Report : public QObject, public TreeItem
{
    Q_OBJECT
public:
    Report(TaskManager& task_man);
    virtual ~Report();

    void clear();

    virtual TreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

    std::shared_ptr<Section> rootSection();
    std::vector<std::shared_ptr<Section>> reportSections() const;
    std::vector<std::shared_ptr<SectionContent>> reportContents() const;

    Section& getSection (const std::string& id); // bla:bla2

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    TaskManager& taskManager() { return task_man_; }
    const TaskManager& taskManager() const { return task_man_; }

    void setCurrentViewable(const nlohmann::json::object_t& data);
    void setCurrentSection(const std::string& section_name);
    std::shared_ptr<ResultReport::SectionContent> loadContent(ResultReport::Section* section, 
                                                              unsigned int content_id) const;

    static const std::string FieldRootSection;

protected:
    TaskManager& task_man_;

    std::shared_ptr<Section> root_section_;
};

}

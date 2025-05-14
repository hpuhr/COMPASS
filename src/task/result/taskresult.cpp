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

#include "taskresult.h"
#include "taskmanager.h"

#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"

#include "timeconv.h"
#include "logger.h"

#include <QMenu>

const std::string  TaskResult::DBTableName         = "task_results";
const Property     TaskResult::DBColumnID          = Property("result_id"   , PropertyDataType::UINT  );
const Property     TaskResult::DBColumnName        = Property("name"        , PropertyDataType::STRING);
const Property     TaskResult::DBColumnJSONContent = Property("json_content", PropertyDataType::JSON  );
const Property     TaskResult::DBColumnResultType  = Property("type"        , PropertyDataType::INT   );
const PropertyList TaskResult::DBPropertyList      = PropertyList({ TaskResult::DBColumnID,
                                                                    TaskResult::DBColumnName,
                                                                    TaskResult::DBColumnJSONContent,
                                                                    TaskResult::DBColumnResultType });
const std::string TaskResult::FieldID       = "id";
const std::string TaskResult::FieldName     = "name";
const std::string TaskResult::FieldType     = "type";
const std::string TaskResult::FieldCreated  = "created";
const std::string TaskResult::FieldComments = "comments";
const std::string TaskResult::FieldReport   = "report";
const std::string TaskResult::FieldConfig   = "config";

/**
 */
TaskResult::TaskResult(unsigned int id, TaskManager& task_man)
:   task_manager_(task_man)
,   id_          (id)
{
    report_ = std::make_shared<ResultReport::Report> (this);
}

/**
 */
TaskResult::~TaskResult() {}

/**
 */
unsigned int TaskResult::id() const
{
    return id_;
}

/**
 */
void TaskResult::id(unsigned int id)
{
    id_ = id;
}

/**
 */
std::string TaskResult::name() const
{
    return name_;
}

/**
 */
void TaskResult::name(const std::string& name)
{
    name_ = name;
}

/**
 */
const std::shared_ptr<ResultReport::Report>& TaskResult::report() const
{
    assert (report_);
    return report_;
}

/**
 */
std::shared_ptr<ResultReport::Report>& TaskResult::report()
{
    assert (report_);
    return report_;
}

/**
 */
void TaskResult::setConfiguration(const nlohmann::json& config)
{
    config_ = config;
}

/**
 */
bool TaskResult::hasConfiguration() const
{
    return config_.is_null();
}

/**
 */
const nlohmann::json& TaskResult::configuration() const
{
    return config_;
}

/**
 */
Result TaskResult::canRecompute() const
{
    //we generally assume a config is needed to recompute the result
    if (!config_.is_object())
        return Result::failed("No configuration available");

    //derived custom check
    return canRecompute_impl();
}

/**
 */
bool TaskResult::recomputeNeeded() const
{
    //outdated => recompute needed
    if (isOutdated())
        return true;

    //derived custom check
    return recomputeNeeded_impl();
}

/**
 */
Result TaskResult::recompute(bool restore_section)
{
    //check if we can recompute
    auto r = canRecompute();
    if (!r.ok())
        return r;

    r = recompute_impl();
    if (!r.ok())
        return r;

    //recompute succeeded => unset outdated flag
    outdated_ = false;

    if (restore_section)
        task_manager_.restoreBackupSection();

    return Result::succeeded();
}

/**
 */
Result TaskResult::recomputeIfNeeded(bool restore_section)
{
    //recompute needed? => recompute
    if (recomputeNeeded())
        return recompute(restore_section);

    //still ok
    return Result::succeeded();
}

/**
 */
void TaskResult::setOutdated()
{
    outdated_ = true;
}

/**
 */
bool TaskResult::isOutdated() const
{
    return outdated_;
}

/**
 */
bool TaskResult::loadOnDemandContent(ResultReport::SectionContent* content) const
{
    if (!content)
        return false;

    if (content->type() == ResultReport::SectionContent::Type::Figure)
    {
        auto c = dynamic_cast<ResultReport::SectionContentFigure*>(content);
        assert(c);

        return loadOnDemandFigure_impl(c);
    }
    else if (content->type() == ResultReport::SectionContent::Type::Table)
    {
        auto c = dynamic_cast<ResultReport::SectionContentTable*>(content);
        assert(c);

        return loadOnDemandTable_impl(c);
    }

    return false;
}

/**
 */
bool TaskResult::loadOnDemandViewable(const ResultReport::SectionContent& content,
                                      ResultReport::SectionContentViewable& viewable, 
                                      const QVariant& index) const
{
    if (!loadOnDemandViewable_impl(content, viewable, index))
        return false;
    
    return viewable.hasCallback();
}

/**
 */
bool TaskResult::customContextMenu(QMenu& menu, 
                                   ResultReport::SectionContentTable* table, 
                                   unsigned int row) const
{
    assert (table);

    bool ok = customContextMenu_impl(menu, table, row);

    return ok && menu.actions().size() > 0;
}

/**
 */
void TaskResult::postprocessTable(ResultReport::SectionContentTable* table) const
{
    assert (table);
    postprocessTable_impl(table);
}

/**
 */
nlohmann::json TaskResult::toJSON() const
{
    nlohmann::json root = nlohmann::json::object();

    root[ FieldID       ] = id_;
    root[ FieldName     ] = name_;
    root[ FieldType     ] = type();
    root[ FieldCreated  ] = Utils::Time::toString(created_);
    root[ FieldComments ] = comments_;

    root[ FieldReport   ] = report_->toJSON();
    root[ FieldConfig   ] = config_;

    //derived content
    toJSON_impl(root);

    return root;
}

/**
 */
bool TaskResult::fromJSON(const nlohmann::json& j)
{
    //loginf << j.dump(4);

    if (!j.is_object()             || 
        !j.contains(FieldID)       ||
        !j.contains(FieldName)     ||
        !j.contains(FieldType)     ||
        !j.contains(FieldCreated)  ||
        !j.contains(FieldComments) ||
        !j.contains(FieldReport)   ||
        !j.contains(FieldConfig))
        return false;

    task::TaskResultType stored_type = j[ FieldType ];
    if (stored_type != type())
    {
        logerr << "TaskResult: fromJSON: Stored type " << stored_type
               << " does not match result type " << type();
        return false;
    }

    id_       = j[ FieldID ];
    name_     = j[ FieldName ];
    comments_ = j[ FieldComments ];

    std::string ts = j[ FieldCreated ];
    created_ = Utils::Time::fromString(ts);

    if (!report_->fromJSON(j[ FieldReport ]))
        return false;

    config_ = j[ FieldConfig ];

    //derived content
    if (!fromJSON_impl(j))
        return false;

    return true;
}

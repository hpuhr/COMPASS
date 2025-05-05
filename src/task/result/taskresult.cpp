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

#include "timeconv.h"
#include "logger.h"

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
:   id_(id)
{
    report_ = std::make_shared<ResultReport::Report> (task_man);
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
TaskResult::ContentPtr TaskResult::createOnDemandContent(const std::string& section_id,
                                                         const std::string& content_id) const
{
    return ContentPtr();
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

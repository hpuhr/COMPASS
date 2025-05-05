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

#include "task/result/report/report.h"
#include "task/taskdefs.h"

#include "json.hpp"
#include "property.h"
#include "propertylist.h"

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

class ResultManager;

namespace ResultReport
{
    class SectionContent;
}

/**
 */
class TaskResult
{
    friend class TaskManager; // to change id if required

public:
    typedef std::shared_ptr<ResultReport::SectionContent> ContentPtr;

    TaskResult(unsigned int id, 
               TaskManager& task_man);
    virtual ~TaskResult();

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    const std::shared_ptr<ResultReport::Report>& report() const;
    std::shared_ptr<ResultReport::Report>& report();

    void setConfiguration(const nlohmann::json& config);
    bool hasConfiguration() const;
    const nlohmann::json& configuration() const;

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    virtual task::TaskResultType type() const { return task::TaskResultType::Generic; }

    static const std::string  DBTableName;
    static const Property     DBColumnID;
    static const Property     DBColumnName;
    static const Property     DBColumnJSONContent;
    static const Property     DBColumnResultType;
    static const PropertyList DBPropertyList;

    static const std::string FieldID;
    static const std::string FieldName;
    static const std::string FieldType;
    static const std::string FieldCreated;
    static const std::string FieldComments;
    static const std::string FieldReport;
    static const std::string FieldConfig;

protected:
    void id(unsigned int id);

    virtual ContentPtr createOnDemandContent(const std::string& section_id,
                                             const std::string& content_id) const;

    //serialization of derived content
    virtual void toJSON_impl(nlohmann::json& root_node) const {};
    virtual bool fromJSON_impl(const nlohmann::json& j) { return true; };

    unsigned int             id_{0};
    std::string              name_;
    boost::posix_time::ptime created_;
    std::string              comments_;

    std::shared_ptr<ResultReport::Report> report_;
    nlohmann::json                        config_;
};

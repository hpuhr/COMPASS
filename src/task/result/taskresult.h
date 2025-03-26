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

#include "json.hpp"
#include "property.h"
#include "task/result/report/report.h"

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

class ResultManager;

/**
 */
class TaskResult
{
    friend class TaskManager; // to change id if required

public:
    enum TaskResultType
    {
        Generic=0,
        Evaluation
    };

    TaskResult(unsigned int id, TaskManager& task_man);

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    TaskResultType type() const;
    void type(TaskResultType type);

    const std::shared_ptr<ResultReport::Report> report() const;
    std::shared_ptr<ResultReport::Report> report();

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    static const std::string DBTableName;
    static const Property    DBColumnID;
    static const Property    DBColumnName;
    static const Property    DBColumnJSONContent;
    static const Property    DBColumnResultType;

    static const std::string FieldID;
    static const std::string FieldName;
    static const std::string FieldType;
    static const std::string FieldCreated;
    static const std::string FieldComments;
    static const std::string FieldReport;

protected:
    void id(unsigned int id);

    //serialization of derived content
    virtual void toJSON_impl(nlohmann::json& root_node) const {};
    virtual bool fromJSON_impl(const nlohmann::json& j) { return true; };

    unsigned int             id_{0};
    std::string              name_;
    TaskResultType           type_;
    boost::posix_time::ptime created_;
    std::string              comments_;

    std::shared_ptr<ResultReport::Report> report_;
};

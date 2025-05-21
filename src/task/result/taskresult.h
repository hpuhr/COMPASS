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

#include "result.h"

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

class ResultManager;

namespace ResultReport
{
    class SectionContent;
    class SectionContentFigure;
    class SectionContentTable;
    
    struct SectionContentViewable;
}

class QMenu;

/**
 */
class TaskResult
{
    friend class TaskManager; // to change id if required

public:
    enum UpdateEvent
    {
        NoUpdate = 0, // no update needed
        Content,      // specific contents need update (e.g. tables)
        Partial,      // partial update needed
        Complete      // full update needed
    };

    typedef std::shared_ptr<ResultReport::SectionContent> ContentPtr;
    typedef std::pair<std::string, std::string>           ContentID;

    TaskResult(unsigned int id, 
               TaskManager& task_man);
    virtual ~TaskResult();

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    const TaskManager& taskManager() const { return task_manager_; }
    TaskManager& taskManager() { return task_manager_; }

    const std::shared_ptr<ResultReport::Report>& report() const;
    std::shared_ptr<ResultReport::Report>& report();

    void setConfiguration(const nlohmann::json& config);
    bool hasConfiguration() const;
    const nlohmann::json& configuration() const;

    bool updateNeeded() const;
    UpdateEvent neededUpdate() const;
    void informUpdate(UpdateEvent evt, 
                      const ContentID& cid = ContentID());
    Result canUpdate() const;
    Result update(bool restore_section = false);

    Result initResult();
    Result finalizeResult();

    bool loadOnDemandContent(ResultReport::SectionContent* content) const;
    bool loadOnDemandViewable(const ResultReport::SectionContent& content,
                              ResultReport::SectionContentViewable& viewable, 
                              const QVariant& index,
                              unsigned int row) const;
    bool customContextMenu(QMenu& menu, 
                           ResultReport::SectionContentTable* table, 
                           unsigned int row);
    bool customContextMenu(QMenu& menu,
                           ResultReport::SectionContent* content);
    void postprocessTable(ResultReport::SectionContentTable* table);

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    virtual task::TaskResultType type() const { return task::TaskResultType::Generic; }

    virtual std::string startSection() const { return ""; }

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
    static const std::string FieldUpdateEvent;
    static const std::string FieldUpdateContents;

protected:
    void id(unsigned int id);
    void clearPendingUpdates();

    //reimplement for custom result clearing
    virtual void clear_impl() {}

    //reimplement for recomputation mechanics
    virtual Result update_impl(UpdateEvent evt) { return false; }
    virtual Result canUpdate_impl(UpdateEvent evt) const { return false; }
    virtual Result updateContents_impl(const std::vector<ContentID>& contents);

    Result updateContents(const std::vector<ContentID>& contents);

    //reimplement for custom initialization/finalization of a result
    virtual Result initResult_impl() { return Result::succeeded(); }
    virtual Result finalizeResult_impl() { return Result::succeeded(); }

    //reimplement for on-demand generation of contents
    virtual bool loadOnDemandFigure_impl(ResultReport::SectionContentFigure* figure) const { return false; }
    virtual bool loadOnDemandTable_impl(ResultReport::SectionContentTable* table) const { return false; }
    virtual bool loadOnDemandViewable_impl(const ResultReport::SectionContent& content,
                                           ResultReport::SectionContentViewable& viewable, 
                                           const QVariant& index,
                                           unsigned int row) const { return false; }
    
    //reimplement for serialization of derived content
    virtual void toJSON_impl(nlohmann::json& root_node) const {};
    virtual bool fromJSON_impl(const nlohmann::json& j) { return true; };

    //reimplement for custom table behavior
    virtual bool customContextMenu_impl(QMenu& menu, 
                                        ResultReport::SectionContentTable* table, 
                                        unsigned int row) { return false; }
    virtual bool customContextMenu_impl(QMenu& menu, 
                                        ResultReport::SectionContent* content) { return false; }
    virtual void postprocessTable_impl(ResultReport::SectionContentTable* table) {}

    TaskManager& task_manager_;

    unsigned int             id_{0};
    std::string              name_;
    boost::posix_time::ptime created_;
    std::string              comments_;

    bool finalized_ = false;

    UpdateEvent            update_evt_ = UpdateEvent::NoUpdate;
    std::vector<ContentID> update_contents_;

    std::shared_ptr<ResultReport::Report> report_;
    nlohmann::json                        config_;
};

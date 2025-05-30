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

#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/reportexport.h"

#include "compass.h"
#include "dbcontentmanager.h"
#include "viewmanager.h"
#include "view.h"

#include "timeconv.h"
#include "logger.h"
#include "files.h"

#include <boost/filesystem.hpp>

#include <QMenu>
#include <QApplication>

const std::string  TaskResult::DBTableName          = "task_results";
const Property     TaskResult::DBColumnID           = Property("result_id"    , PropertyDataType::UINT  );
const Property     TaskResult::DBColumnName         = Property("name"         , PropertyDataType::STRING);
const Property     TaskResult::DBColumnJSONHeader   = Property("json_header"  , PropertyDataType::JSON  );
const Property     TaskResult::DBColumnJSONContent  = Property("json_content" , PropertyDataType::JSON  );
const Property     TaskResult::DBColumnResultType   = Property("type"         , PropertyDataType::INT   );
const PropertyList TaskResult::DBPropertyList       = PropertyList({ TaskResult::DBColumnID,
                                                                     TaskResult::DBColumnName,
                                                                     TaskResult::DBColumnJSONHeader,
                                                                     TaskResult::DBColumnJSONContent,
                                                                     TaskResult::DBColumnResultType });
const std::string TaskResult::FieldID                   = "id";
const std::string TaskResult::FieldName                 = "name";
const std::string TaskResult::FieldType                 = "type";
const std::string TaskResult::FieldMetaDataCreated      = "ts_created";
const std::string TaskResult::FieldMetaDataRefreshed    = "ts_refreshed";
const std::string TaskResult::FieldMetaDataUser         = "user";
const std::string TaskResult::FieldMetaDataComments     = "comments";
const std::string TaskResult::FieldMetaData             = "metadata";
const std::string TaskResult::FieldHeaderUpdateState    = "update_state";
const std::string TaskResult::FieldHeaderUpdateContents = "update_contents";
const std::string TaskResult::FieldReport               = "report";
const std::string TaskResult::FieldConfig               = "config";

/************************************************************************************************
 * TaskResultMetaData
 ************************************************************************************************/

/**
 */
nlohmann::json TaskResultMetaData::toJSON() const
{
    nlohmann::json j;

    j[ TaskResult::FieldMetaDataCreated   ] = Utils::Time::toString(ts_created);
    j[ TaskResult::FieldMetaDataRefreshed ] = Utils::Time::toString(ts_refreshed);
    j[ TaskResult::FieldMetaDataUser      ] = user;
    j[ TaskResult::FieldMetaDataComments  ] = comments;

    return j;
}

/**
 */
bool TaskResultMetaData::fromJSON(const nlohmann::json& j)
{
    if (!j.is_object() ||
        !j.contains(TaskResult::FieldMetaDataCreated)   ||
        !j.contains(TaskResult::FieldMetaDataRefreshed) ||
        !j.contains(TaskResult::FieldMetaDataUser)      ||
        !j.contains(TaskResult::FieldMetaDataComments))
        return false;

    std::string ts_created_str = j[ TaskResult::FieldMetaDataCreated ];
    ts_created = Utils::Time::fromString(ts_created_str);

    std::string ts_refreshed_str = j[ TaskResult::FieldMetaDataRefreshed ];
    ts_refreshed = Utils::Time::fromString(ts_refreshed_str);

    user     = j[ TaskResult::FieldMetaDataUser     ];
    comments = j[ TaskResult::FieldMetaDataComments ];

    return true;
}

/************************************************************************************************
 * TaskResultHeader
 ************************************************************************************************/

/**
 */
nlohmann::json TaskResultHeader::toJSON() const
{
    nlohmann::json j;

    j[ TaskResult::FieldMetaData             ] = metadata.toJSON();
    j[ TaskResult::FieldHeaderUpdateState    ] = update_state;
    j[ TaskResult::FieldHeaderUpdateContents ] = update_contents;

    return j;
}

/**
 */
bool TaskResultHeader::fromJSON(const nlohmann::json& j)
{
    if (!j.is_object() ||
        !j.contains(TaskResult::FieldMetaData)             ||
        !j.contains(TaskResult::FieldHeaderUpdateState)    ||
        !j.contains(TaskResult::FieldHeaderUpdateContents))
        return false;

    if (!metadata.fromJSON(j[ TaskResult::FieldMetaData ]))
        return false;

    update_state    = j[ TaskResult::FieldHeaderUpdateState    ];
    update_contents = j[ TaskResult::FieldHeaderUpdateContents ].get<std::vector<std::pair<std::string, std::string>>>();

    return true;
}

/************************************************************************************************
 * TaskResult
 ************************************************************************************************/

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
const TaskResultMetaData& TaskResult::metadata() const
{
    return metadata_;
}

/**
 */
TaskResultHeader TaskResult::header() const
{
    TaskResultHeader header;
    header.metadata        = metadata_;
    header.update_state    = update_state_;
    header.update_contents = update_contents_;

    return header;
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
void TaskResult::configure(const TaskResultHeader& header)
{
    //apply update state stored in header
    if (header.update_state == UpdateState::ContentUpdateNeeded)
    {
        for (const auto& c : header.update_contents)
            informUpdate(header.update_state, c, false);
    }
    else
    {
        informUpdate(header.update_state, ContentID(), false);
    }
}

/**
 */
void TaskResult::setJSONConfiguration(const nlohmann::json& config)
{
    config_ = config;
}

/**
 */
bool TaskResult::hasJSONConfiguration() const
{
    return config_.is_null();
}

/**
 */
const nlohmann::json& TaskResult::jsonConfiguration() const
{
    return config_;
}

/**
 */
void TaskResult::informUpdate(UpdateState state, 
                              const ContentID& cid,
                              bool inform_manager)
{
    //"biggest" update wins
    bool state_changed = false;
    if (state > update_state_)
    {
        update_state_ = state;
        state_changed = true;
    }

    if (update_state_ == UpdateState::ContentUpdateNeeded)
    {
        //add content info
        assert(!cid.first.empty());
        assert(!cid.second.empty());

        update_contents_.push_back(cid);
    }
    else
    {
        //content info not needed any more
        update_contents_.clear();
    }

    //inform task manager?
    if (state_changed && inform_manager)
    {
        if (update_state_ == UpdateState::Locked)
        {
            //result is now locked => update contents of report accordingly
            report_->updateContents();
        }

        task_manager_.resultHeaderChanged(*this);
    }
}

/**
 */
bool TaskResult::isLocked() const
{
    return update_state_ == UpdateState::Locked;
}

/**
 */
bool TaskResult::updateNeeded() const
{
    return update_state_ != UpdateState::UpToDate;
}

/**
 */
TaskResult::UpdateState TaskResult::updateState() const
{
    return update_state_;
}

/**
 */
Result TaskResult::canUpdate() const
{
    //we generally assume a config is needed to recompute the result
    if (!config_.is_object())
        return Result::failed("No configuration available");

    if (update_state_ == UpdateState::UpToDate)
        return Result::succeeded();

    //derived custom check
    return canUpdate_impl(update_state_);
}

/**
 */
Result TaskResult::update(bool restore_section,
                          bool inform_manager)
{
    if (!updateNeeded())
        return Result::succeeded();

    //check if we can recompute
    auto r = canUpdate();
    if (!r.ok())
        return r;

    r = Result::succeeded();
    bool restore_needed = false;

    if (update_state_ == UpdateState::ContentUpdateNeeded)
    {
        loginf << "TaskResult: update: running content update";

        for (const auto& c : update_contents_)
            loginf << "   " << c.first << " " << c.second;

        //update specific contents
        r = updateContents(update_contents_);
    }
    else if (update_state_ != UpdateState::UpToDate)
    {
        loginf << "TaskResult: update: running " 
               << (update_state_ == UpdateState::PartialUpdateNeeded ? "partial" : "full") << " update";

        //partial and full update
        r = update_impl(update_state_);

        // these bigger updates might trigger report regeneration
        restore_needed = true;
    }

    //update failed?
    if (!r.ok())
    {
        logerr << "TaskResult: update: update failed: " << r.error();
        return r;
    }

    //recompute succeeded => reset updates
    clearPendingUpdates();

    //restore current section in widgets if desired
    if (restore_needed && restore_section)
        task_manager_.restoreBackupSection();

    //inform task manager?
    if (inform_manager)
        task_manager_.resultHeaderChanged(*this);

    return Result::succeeded();
}

/**
 */
Result TaskResult::updateContents(const std::vector<ContentID>& contents)
{
    if (contents.empty())
        return Result::succeeded();

    return updateContents_impl(contents);
}

/**
 */
Result TaskResult::updateContents_impl(const std::vector<ContentID>& contents)
{
    Result r = Result::succeeded();
    
    for (const auto& c : contents)
    {
        if (!report_->hasSection(c.first))
        {
            r = Result::failed("Unknown section '" + c.first + "'");
            continue;
        }

        auto& section = report_->getSection(c.first);
        auto  flags   = section.contentInfo(c.second);

        //content must be available in section
        if ((flags & ResultReport::Section::ContentInfoFlag::ContentAvailable) == 0)
        {
            r = Result::failed("Unknown content '" + c.second + "' in section '" + c.first + "'");
            continue;
        }

        //not yet loaded from db? => no problem, nothing to update
        if ((flags & ResultReport::Section::ContentInfoFlag::ContentLoaded) == 0)
            continue;

        //only on-demand contents allowed
        if ((flags & ResultReport::Section::ContentInfoFlag::ContentOnDemand) == 0)
        {
            r = Result::failed("Static content '" + c.second + "' in section '" + c.first + "'");
            continue;
        }

        //on-demand content not yet complete? => no problem, nothing to update
        if ((flags & ResultReport::Section::ContentInfoFlag::ContentOnDemandComplete) == 0)
            continue;

        //update content
        auto content_id = section.contentID(c.second);
        auto content    = section.retrieveContent(content_id);
        assert(content);
        assert(content->isOnDemand());
        assert(content->isComplete());

        if (!content->forceReload())
            r = Result::failed("Could not update content '" + c.second + "' in section '" + c.first + "'");
    }

    return r;
}

/**
 */
void TaskResult::clearPendingUpdates()
{
    update_state_ = UpdateState::UpToDate;
    update_contents_.clear();
}

/**
 */
Result TaskResult::initResult()
{
    //clear report
    report()->clear();

    //custom clear
    clear_impl();

    //reset any pending updates
    clearPendingUpdates();

    //custom init
    return initResult_impl();
}

/**
 */
Result TaskResult::finalizeResult()
{
    if (finalized_)
        return Result::succeeded();

    auto r = finalizeResult_impl();
    if (!r.ok())
        return r;

    finalized_ = true;

    return Result::succeeded();
}

/**
 */
bool TaskResult::loadOnDemandContent(ResultReport::SectionContent* content) const
{
    if (!content)
        return false;

    if (content->isLocked())
        return false;

    if (content->contentType() == ResultReport::SectionContent::ContentType::Figure)
    {
        auto c = dynamic_cast<ResultReport::SectionContentFigure*>(content);
        assert(c);

        return loadOnDemandFigure_impl(c);
    }
    else if (content->contentType() == ResultReport::SectionContent::ContentType::Table)
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
                                      const QVariant& index,
                                      unsigned int row) const
{
    //on demand viewables cannot be loaded if the result is locked
    if (isLocked())
        return false;

    if (!loadOnDemandViewable_impl(content, viewable, index, row))
        return false;
    
    return viewable.hasCallback();
}

/**
 */
bool TaskResult::customContextMenu(QMenu& menu, 
                                   ResultReport::SectionContentTable* table, 
                                   unsigned int row)
{
    assert (table);

    if (table->isLocked())
        return false;

    bool ok = customContextMenu_impl(menu, table, row);

    return ok && menu.actions().size() > 0;
}

/**
 */
bool TaskResult::customMenu(QMenu& menu,
                            ResultReport::SectionContent* content)
{
    assert (content);

    if (content->isLocked())
        return false;

    return customMenu_impl(menu, content);
}

/**
 */
void TaskResult::postprocessTable(ResultReport::SectionContentTable* table)
{
    //no need to postprocess locked tables
    if (table->isLocked())
        return;

    assert (table);
    postprocessTable_impl(table);
}

/**
 */
bool TaskResult::hasCustomTooltip(const ResultReport::SectionContentTable* table, 
                                  unsigned int row,
                                  unsigned int col) const
{
    assert (table);

    if (table->isLocked())
        return false;

    return hasCustomTooltip_impl(table, row, col);
}

/**
 */
std::string TaskResult::customTooltip(const ResultReport::SectionContentTable* table, 
                                      unsigned int row,
                                      unsigned int col) const
{
    assert (table);
    assert (!table->isLocked());
    return customTooltip_impl(table, row, col);
}

/**
 */
nlohmann::json TaskResult::toJSON() const
{
    nlohmann::json j = nlohmann::json::object();

    j[ FieldType           ] = type();
    j[ FieldID             ] = id_;
    j[ FieldName           ] = name_;
    j[ FieldMetaData       ] = metadata_.toJSON();
    j[ FieldReport         ] = report_->toJSON();
    j[ FieldConfig         ] = config_;

    //derived content
    toJSON_impl(j);

    return j;
}

/**
 */
bool TaskResult::fromJSON(const nlohmann::json& j)
{
    //loginf << j.dump(4);

    if (!j.is_object()                    || 
        !j.contains(FieldType)            ||
        !j.contains(FieldID)              ||
        !j.contains(FieldName)            ||
        !j.contains(FieldMetaData)        ||
        !j.contains(FieldReport)          ||
        !j.contains(FieldConfig))
        return false;

    task::TaskResultType stored_type = j[ FieldType ];
    if (stored_type != type())
    {
        logerr << "TaskResult: fromJSON: Stored type " << stored_type
               << " does not match result type " << type();
        return false;
    }

    id_       = j[ FieldID   ];
    name_     = j[ FieldName ];

    if (!metadata_.fromJSON(j[ FieldMetaData ]))
        return false;

    if (!report_->fromJSON(j[ FieldReport ]))
        return false;

    config_ = j[ FieldConfig ];

    //derived content
    if (!fromJSON_impl(j))
        return false;

    //finalize after reading in data
    auto f_res = finalizeResult();
    if (!f_res.ok())
    {
        logerr << "TaskResult: fromJSON: Finalizing result failed: " << f_res.error();
        return false;
    }

    return true;
}

/**
 */
ResultT<nlohmann::json> TaskResult::exportResult(const std::string& fn,
                                                 ResultReport::ReportExportMode mode)
{
    auto dir      = boost::filesystem::path(fn).parent_path().string();
    auto filename = boost::filesystem::path(fn).filename().string();

    std::string temp_dir = dir + "/" + "report_" + name() + "_" + filename;

    ResultReport::ReportExport r_export;
    return r_export.exportReport(*this, mode, fn, temp_dir);
}

/**
 */
std::vector<std::pair<QImage, std::string>> TaskResult::renderFigure(const ResultReport::SectionContentFigure& figure) const
{
    //do net render locked figures
    if (figure.isLocked())
        return std::vector<std::pair<QImage, std::string>>();

    std::vector<std::pair<QImage, std::string>> renderings;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    ViewManager&      view_man   = COMPASS::instance().viewManager();

    //while (QCoreApplication::hasPendingEvents())
    QCoreApplication::processEvents();

    figure.view();

    while (dbcont_man.loadInProgress())
        QCoreApplication::processEvents();

    QCoreApplication::processEvents();

    figure.executeRenderDelay();

    for (auto& view_it : view_man.getViews())
    {
        //skip table views
        if (view_it.second->classId() == "TableView")
            continue;
        
        //skip views which show no data
        if (view_it.second->classId() != "GeographicView" && !view_it.second->showsData())
            continue;

        //render view and collect
        auto img = view_it.second->renderData();
        renderings.emplace_back(img, view_it.second->instanceId());
    }

    return renderings;
}

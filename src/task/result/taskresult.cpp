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

const std::string  TaskResult::DBTableName         = "task_results";
const Property     TaskResult::DBColumnID          = Property("result_id"   , PropertyDataType::UINT  );
const Property     TaskResult::DBColumnName        = Property("name"        , PropertyDataType::STRING);
const Property     TaskResult::DBColumnJSONContent = Property("json_content", PropertyDataType::JSON  );
const Property     TaskResult::DBColumnResultType  = Property("type"        , PropertyDataType::INT   );
const PropertyList TaskResult::DBPropertyList      = PropertyList({ TaskResult::DBColumnID,
                                                                    TaskResult::DBColumnName,
                                                                    TaskResult::DBColumnJSONContent,
                                                                    TaskResult::DBColumnResultType });
const std::string TaskResult::FieldID             = "id";
const std::string TaskResult::FieldName           = "name";
const std::string TaskResult::FieldType           = "type";
const std::string TaskResult::FieldCreated        = "created";
const std::string TaskResult::FieldComments       = "comments";
const std::string TaskResult::FieldReport         = "report";
const std::string TaskResult::FieldConfig         = "config";
const std::string TaskResult::FieldUpdateEvent    = "update_event";
const std::string TaskResult::FieldUpdateContents = "update_contents";

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
void TaskResult::informUpdate(UpdateEvent evt, 
                              const ContentID& cid,
                              bool inform_manager)
{
    //"biggest" update wins
    if (evt > update_evt_)
        update_evt_ = evt;

    if (update_evt_ == UpdateEvent::Content)
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
    if (inform_manager)
        task_manager_.resultChanged(*this);
}

/**
 */
bool TaskResult::updateNeeded() const
{
    return update_evt_ != UpdateEvent::NoUpdate;
}

/**
 */
TaskResult::UpdateEvent TaskResult::neededUpdate() const
{
    return update_evt_;
}

/**
 */
Result TaskResult::canUpdate() const
{
    //we generally assume a config is needed to recompute the result
    if (!config_.is_object())
        return Result::failed("No configuration available");

    if (update_evt_ == UpdateEvent::NoUpdate)
        return Result::succeeded();

    //derived custom check
    return canUpdate_impl(update_evt_);
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

    if (update_evt_ == UpdateEvent::Content)
    {
        loginf << "TaskResult: update: running content update";

        for (const auto& c : update_contents_)
            loginf << "   " << c.first << " " << c.second;

        //update specific contents
        r = updateContents(update_contents_);
    }
    else if (update_evt_ != UpdateEvent::NoUpdate)
    {
        loginf << "TaskResult: update: running " << (update_evt_ == UpdateEvent::Partial ? "partial" : "full") << " update";

        //partial and full update
        r = update_impl(update_evt_);

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
        task_manager_.resultChanged(*this);

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
    update_evt_ = UpdateEvent::NoUpdate;
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

    bool ok = customContextMenu_impl(menu, table, row);

    return ok && menu.actions().size() > 0;
}

/**
 */
bool TaskResult::customContextMenu(QMenu& menu,
                                   ResultReport::SectionContent* content)
{
    assert (content);

    return customContextMenu_impl(menu, content);
}

/**
 */
void TaskResult::postprocessTable(ResultReport::SectionContentTable* table)
{
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
    return hasCustomTooltip_impl(table, row, col);
}

/**
 */
std::string TaskResult::customTooltip(const ResultReport::SectionContentTable* table, 
                                      unsigned int row,
                                      unsigned int col) const
{
    assert (table);
    return customTooltip_impl(table, row, col);
}

/**
 */
nlohmann::json TaskResult::toJSON() const
{
    nlohmann::json root = nlohmann::json::object();

    root[ FieldID             ] = id_;
    root[ FieldName           ] = name_;
    root[ FieldType           ] = type();
    root[ FieldCreated        ] = Utils::Time::toString(created_);
    root[ FieldComments       ] = comments_;

    root[ FieldUpdateEvent    ] = update_evt_;
    root[ FieldUpdateContents ] = update_contents_;

    root[ FieldReport         ] = report_->toJSON();
    root[ FieldConfig         ] = config_;

    //derived content
    toJSON_impl(root);

    return root;
}

/**
 */
bool TaskResult::fromJSON(const nlohmann::json& j)
{
    //loginf << j.dump(4);

    if (!j.is_object()                    || 
        !j.contains(FieldID)              ||
        !j.contains(FieldName)            ||
        !j.contains(FieldType)            ||
        !j.contains(FieldCreated)         ||
        !j.contains(FieldComments)        ||
        !j.contains(FieldUpdateEvent)     ||
        !j.contains(FieldUpdateContents)  ||
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

    id_       = j[ FieldID       ];
    name_     = j[ FieldName     ];
    comments_ = j[ FieldComments ];

    update_evt_      = j[ FieldUpdateEvent    ];
    update_contents_ = j[ FieldUpdateContents ].get<std::vector<std::pair<std::string, std::string>>>();

    std::string ts = j[ FieldCreated ];
    created_ = Utils::Time::fromString(ts);

    if (!report_->fromJSON(j[ FieldReport ]))
        return false;

    config_ = j[ FieldConfig ];

    //derived content
    if (!fromJSON_impl(j))
        return false;

    //finalize
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

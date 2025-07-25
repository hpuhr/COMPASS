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

#include "taskmanager.h"

#include "compass.h"
#include "createartasassociationstask.h"
#include "jsonimporttask.h"
#include "managesectorstask.h"
#include "radarplotpositioncalculatortask.h"
#include "viewpointsimporttask.h"
#include "gpstrailimporttask.h"
//#include "gpsimportcsvtask.h"
#include "reconstructortask.h"
#include "mainwindow.h"
#include "viewabledataconfig.h"
#include "viewmanager.h"
#include "dbinterface.h"
#include "asynctask.h"

#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"

#include "taskresult.h"
#include "taskresultswidget.h"

#include "reportexport.h"
#include "reportexportdialog.h"

#include "evaluationtaskresult.h"

#include <cassert>

#include <QCoreApplication>
#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <QProgressDialog>
#include <QMessageBox>

//#include "boost/date_time/posix_time/posix_time.hpp"

using namespace Utils;

const bool TaskManager::CleanupDBIfNeeded = true;

/**
 */
TaskManager::TaskManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "task.json")
{
    createSubConfigurables();

    setObjectName("TaskManager");
}

/**
 */
TaskManager::~TaskManager() {}

/**
 */
void TaskManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    if (class_id == "ASTERIXImportTask")
    {
        assert(!asterix_importer_task_);
        asterix_importer_task_.reset(new ASTERIXImportTask(class_id, instance_id, *this));
        assert(asterix_importer_task_);
        addTask(class_id, asterix_importer_task_.get());
    }
    else if (class_id == "ViewPointsImportTask")
    {
        assert(!view_points_import_task_);
        view_points_import_task_.reset(new ViewPointsImportTask(class_id, instance_id, *this));
        assert(view_points_import_task_);
        addTask(class_id, view_points_import_task_.get());
    }
    else if (class_id == "JSONImportTask")
    {
        assert(!json_import_task_);
        json_import_task_.reset(new JSONImportTask(class_id, instance_id, *this));
        assert(json_import_task_);
        addTask(class_id, json_import_task_.get());
    }
    else if (class_id == "GPSTrailImportTask")
    {
        assert(!gps_trail_import_task_);
        gps_trail_import_task_.reset(new GPSTrailImportTask(class_id, instance_id, *this));
        assert(gps_trail_import_task_);
        addTask(class_id, gps_trail_import_task_.get());
    }
    // else if (class_id == "GPSImportCSVTask")
    // {
    //     assert(!gps_import_csv_task_);
    //     gps_import_csv_task_.reset(new GPSImportCSVTask(class_id, instance_id, *this));
    //     assert(gps_import_csv_task_);
    //     addTask(class_id, gps_import_csv_task_.get());
    // }
    else if (class_id == "ManageSectorsTask")
    {
        assert(!manage_sectors_task_);
        manage_sectors_task_.reset(new ManageSectorsTask(class_id, instance_id, *this));
        assert(manage_sectors_task_);
        addTask(class_id, manage_sectors_task_.get());
    }
    else if (class_id == "RadarPlotPositionCalculatorTask")
    {
        assert(!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_.reset(new RadarPlotPositionCalculatorTask(class_id, instance_id, *this));
        assert(radar_plot_position_calculator_task_);
        addTask(class_id, radar_plot_position_calculator_task_.get());

        connect(radar_plot_position_calculator_task_.get(), &RadarPlotPositionCalculatorTask::doneSignal,
                this, &TaskManager::taskRadarPlotPositionsDoneSignal);
    }
    else if (class_id == "CreateARTASAssociationsTask")
    {
        assert(!create_artas_associations_task_);
        create_artas_associations_task_.reset(
                    new CreateARTASAssociationsTask(class_id, instance_id, *this));
        assert(create_artas_associations_task_);
        addTask(class_id, create_artas_associations_task_.get());
    }
    else if (class_id == "ReconstructorTask")
    {
        assert(!reconstruct_references_task_);
        reconstruct_references_task_.reset(new ReconstructorTask(class_id, instance_id, *this));
        assert(reconstruct_references_task_);
        addTask(class_id, reconstruct_references_task_.get());
    }
    else if (class_id == "ReportExport")
    {
        assert(!report_export_);
        report_export_.reset(new ResultReport::ReportExport(class_id, instance_id, this));
        assert(report_export_);
    }
    else
    {
        throw std::runtime_error("TaskManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
    }
}

/**
 */
void TaskManager::addTask(const std::string& class_id, Task* task)
{
    assert(task);
    assert(!tasks_.count(class_id));
    tasks_[class_id] = task;
}

/**
 */
void TaskManager::checkSubConfigurables()
{
    if (!asterix_importer_task_)
    {
        generateSubConfigurable("ASTERIXImportTask", "ASTERIXImportTask0");
        assert(asterix_importer_task_);
    }

    if (!view_points_import_task_)
    {
        generateSubConfigurable("ViewPointsImportTask", "ViewPointsImportTask0");
        assert(view_points_import_task_);
    }

    if (!json_import_task_)
    {
        generateSubConfigurable("JSONImportTask", "JSONImportTask0");
        assert(json_import_task_);
    }

    if (!gps_trail_import_task_)
    {
        generateSubConfigurable("GPSTrailImportTask", "GPSTrailImportTask0");
        assert(gps_trail_import_task_);
    }

    // if (!gps_import_csv_task_)
    // {
    //     generateSubConfigurable("GPSImportCSVTask", "GPSImportCSVTask0");
    //     assert(gps_import_csv_task_);
    // }

    if (!manage_sectors_task_)
    {
        generateSubConfigurable("ManageSectorsTask", "ManageSectorsTask0");
        assert(manage_sectors_task_);
    }

    if (!radar_plot_position_calculator_task_)
    {
        generateSubConfigurable("RadarPlotPositionCalculatorTask",
                                "RadarPlotPositionCalculatorTask0");
        assert(radar_plot_position_calculator_task_);
    }

    if (!create_artas_associations_task_)
    {
        generateSubConfigurable("CreateARTASAssociationsTask", "CreateARTASAssociationsTask0");
        assert(create_artas_associations_task_);
    }

    if (!reconstruct_references_task_)
    {
        generateSubConfigurable("ReconstructorTask", "ReconstructorTask0");
        assert(reconstruct_references_task_);
    }

    if (!report_export_)
    {
        generateSubConfigurable("ReportExport", "ReportExport0");
        assert(report_export_);
    }
}

/**
 */
std::map<std::string, Task*> TaskManager::tasks() const 
{ 
    return tasks_; 
}

/**
 */
void TaskManager::init()
{
    //init all tasks
    for (const auto& t : tasks_)
        if (t.second)
            t.second->initTask();

    //update features
    updateFeatures();
}

/**
 */
void TaskManager::shutdown()
{
    loginf << "TaskManager: shutdown";

    asterix_importer_task_->stop(); // stops if active
    asterix_importer_task_ = nullptr;

    view_points_import_task_ = nullptr;
    json_import_task_ = nullptr;
    gps_trail_import_task_ = nullptr;
    //gps_import_csv_task_ = nullptr;
    manage_sectors_task_ = nullptr;
    radar_plot_position_calculator_task_ = nullptr;
    create_artas_associations_task_ = nullptr;
    reconstruct_references_task_ = nullptr;
}

/**
 */
void TaskManager::runTask(const std::string& task_name)
{
    loginf << "TaskManager: runTask: name " << task_name;

    assert(tasks_.count(task_name));
    assert(tasks_.at(task_name)->canRun());

    tasks_.at(task_name)->run();
}

/**
 */
ManageSectorsTask& TaskManager::manageSectorsTask() const
{
    assert(manage_sectors_task_);
    return *manage_sectors_task_;
}

/**
 */
ASTERIXImportTask& TaskManager::asterixImporterTask() const
{
    assert(asterix_importer_task_);
    return *asterix_importer_task_;
}

/**
 */
ViewPointsImportTask& TaskManager::viewPointsImportTask() const
{
    assert(view_points_import_task_);
    return *view_points_import_task_;
}

/**
 */
JSONImportTask& TaskManager::jsonImporterTask() const
{
    assert(json_import_task_);
    return *json_import_task_;
}

/**
 */
GPSTrailImportTask& TaskManager::gpsTrailImportTask() const
{
    assert(gps_trail_import_task_);
    return *gps_trail_import_task_;
}

/**
 */
// GPSImportCSVTask& TaskManager::gpsImportCSVTask() const
// {
//     assert(gps_import_csv_task_);
//     return *gps_import_csv_task_;
// }

/**
 */
RadarPlotPositionCalculatorTask& TaskManager::radarPlotPositionCalculatorTask() const
{
    assert(radar_plot_position_calculator_task_);
    return *radar_plot_position_calculator_task_;
}

/**
 */
CreateARTASAssociationsTask& TaskManager::createArtasAssociationsTask() const
{
    assert(create_artas_associations_task_);
    return *create_artas_associations_task_;
}

/**
 */
ReconstructorTask& TaskManager::reconstructReferencesTask() const
{
    assert(reconstruct_references_task_);
    return *reconstruct_references_task_;
}

/**
 */
TaskResultsWidget* TaskManager::widget()
{
    if (!widget_)
        widget_.reset(new TaskResultsWidget(*this));

    assert(widget_);
    return widget_.get();
}

/**
 */
std::shared_ptr<TaskResult> TaskManager::createResult(unsigned int id, 
                                                      task::TaskResultType type)
{
    std::shared_ptr<TaskResult> result;

    //generate result depending on stored type (@TODO: factory?)
    if (type == task::TaskResultType::Generic)
    {
        result.reset(new TaskResult(id, *this));
    }
    else if (type == task::TaskResultType::Evaluation)
    {
        result.reset(new EvaluationTaskResult(id, *this));
    }
    
    return result;
}

/**
 */
void TaskManager::beginTaskResultWriting(const std::string& name,
                                         task::TaskResultType type)
{
    if (widget_)
        widget_->setDisabled(true);

    if (current_result_)
        logerr << "TaskManager: beginTaskResultWriting: result id " << current_result_->id()
               << " name " << current_result_->name() << " already present";

    assert (!current_result_);
    current_result_ = getOrCreateResult(name, type);

    //prepare result for new content
    auto res = current_result_->prepareResult();
    if (!res.ok())
        logerr << "TaskManager: beginTaskResultWriting: result could not be initialized: " << res.error();

    loginf << "TaskManager: beginTaskResultWriting: begining result id " << current_result_->id()
           << " name " << current_result_->name();
    
    assert(res.ok());
}

/**
 */
std::shared_ptr<TaskResult>& TaskManager::currentResult()
{
    assert (current_result_);
    return current_result_;
}

std::shared_ptr<ResultReport::Report>& TaskManager::currentReport()
{
    assert (current_result_);
    return current_result_->report();
}

/**
 */
void TaskManager::endTaskResultWriting(bool store_result, bool show_dialog)
{
    loginf << "TaskManager: endTaskResultWriting: store_result " << store_result;

    if (widget_)
        widget_->setDisabled(false);

    assert (current_result_);

    //finalize result after adding content
    auto res = current_result_->finalizeResult();
    if (!res.ok())
        logerr << "TaskManager: endTaskResultWriting: Result could not be finalized: " << res.error();

    assert(res.ok());

    //store result?
    if (store_result)
    {
        loginf << "TaskManager: endTaskResultWriting: Storing result...";

        auto result_ptr = current_result_.get();
        bool cleanup_db = CleanupDBIfNeeded;

        auto cb = [ result_ptr, cleanup_db ] (const AsyncTaskState& s, AsyncTaskProgressWrapper& p)
        {
            return COMPASS::instance().dbInterface().saveResult(*result_ptr, cleanup_db);
        };

        AsyncFuncTask task(cb, "Save Result", "Saving result", false);
        bool ok = show_dialog ? task.runAsyncDialog() : task.runAsync();

        //@TODO
        if (!ok)
            logerr << "TaskManager: endTaskResultWriting: Storing result failed: " << task.taskState().error;
        
        assert(ok);
    }

    assert (current_result_);

    loginf << "TaskManager: endTaskResultWriting: ending result id " << current_result_->id()
           << " name " << current_result_->name();

    current_result_ = nullptr;

    emit taskResultsChangedSignal();
}

/**
 */
void TaskManager::resultHeaderChanged(const TaskResult& result)
{
    //update result header upon change
    auto res = COMPASS::instance().dbInterface().updateResultHeader(result);
    assert(res.ok());

    emit taskResultHeaderChangedSignal(QString::fromStdString(result.name()));
}

/**
 */
MainWindow* TaskManager::getMainWindow()
{
    for(QWidget* pWidget : QApplication::topLevelWidgets())
    {
        QMainWindow* qt_main_window = qobject_cast<QMainWindow*>(pWidget);

        if (qt_main_window)
        {
            MainWindow* main_window = dynamic_cast<MainWindow*>(qt_main_window);
            assert (main_window);
            return main_window;
        }
    }
    return nullptr;
}

/**
 */
void TaskManager::updateFeatures()
{
    for (auto& t : tasks_)
        if (t.second)
            t.second->updateFeatures();
}

/**
 */
const std::map<unsigned int, std::shared_ptr<TaskResult>>& TaskManager::results() const
{
    return results_;
}

/**
 */
std::shared_ptr<TaskResult> TaskManager::result(unsigned int id) const // get existing result
{
    assert (results_.count(id));
    return results_.at(id);
}

/**
 */
std::shared_ptr<TaskResult> TaskManager::result(const std::string& name) const // get existing result
{
    auto id = findResult(name);
    return id.has_value() ? results_.at(id.value()) : std::shared_ptr<TaskResult>();
}

/**
 */
std::shared_ptr<TaskResult> TaskManager::getOrCreateResult(const std::string& name,
                                                           task::TaskResultType type)
{
    auto id = findResult(name);

    if (id.has_value())
    {
        return results_.at(id.value());
    }
    else // create
    {
        unsigned int new_id{0};

        if (results_.size())
            new_id = results_.rbegin()->first + 1;

        auto r = createResult(new_id, type);

        results_[new_id] = r;
        results_.at(new_id)->name(name);

        return results_.at(new_id);
    }
}

/**
 */
boost::optional<unsigned int> TaskManager::findResult(const std::string& name) const
{
    auto it = std::find_if(results_.begin(), results_.end(),
                           [&name](const std::pair<const unsigned int, std::shared_ptr<TaskResult>>& pair) {
                               return pair.second && pair.second->name() == name; });
    if (it == results_.end())
        return boost::optional<unsigned int>();

    return it->first;
}

/**
 */
bool TaskManager::hasResult (const std::string& name) const
{
    return findResult(name).has_value();
}

/**
 */
bool TaskManager::removeResult(const std::string& name,
                               bool inform_changes)
{
    auto id = findResult(name);
    if (!id.has_value())
        return true;

    const auto& result = results_.at(id.value());
    assert(result);

    auto res = COMPASS::instance().dbInterface().deleteResult(*result, CleanupDBIfNeeded);
    if (!res.ok())
        return false;

    results_.erase(id.value());

    if (inform_changes)
        emit taskResultsChangedSignal();

    return true;
}

/**
 */
ResultT<nlohmann::json> TaskManager::exportResult(const std::string& name, 
                                                  ResultReport::ReportExportMode mode,
                                                  bool no_interaction_mode,
                                                  const boost::optional<std::string>& export_dir,
                                                  const std::string& section)
{
    assert(report_export_);
    assert(hasResult(name));

    auto r = result(name);
    assert(r);

    ResultReport::ReportExportDialog dlg(*r, 
                                         *report_export_, 
                                         mode,
                                         no_interaction_mode,
                                         export_dir,
                                         section);
    dlg.exec();

    return dlg.result();
}

/**
 */
void TaskManager::databaseOpenedSlot()
{
    loadResults();
}

/**
 */
void TaskManager::databaseClosedSlot()
{
    clearResults();
}

/**
 */
void TaskManager::setViewableDataConfig(const nlohmann::json::object_t& data)
{
    viewable_data_cfg_.reset(new ViewableDataConfig(data));

    COMPASS::instance().viewManager().setCurrentViewPoint(viewable_data_cfg_.get());
}

/**
 */
void TaskManager::unsetViewableDataConfig()
{
    COMPASS::instance().viewManager().unsetCurrentViewPoint();
    viewable_data_cfg_.reset();
}

/**
 */
std::shared_ptr<ResultReport::SectionContent> TaskManager::loadContent(ResultReport::Section* section, 
                                                                       unsigned int content_id,
                                                                       bool show_dialog) const
{
    ResultT<TaskResult::ContentPtr> result;

    if (show_dialog)
    {
        //run as async task with dialog
        auto result_ptr = &result;

        auto cb = [ this, result_ptr, section, content_id ] (const AsyncTaskState&, AsyncTaskProgressWrapper&) 
        { 
            *result_ptr = COMPASS::instance().dbInterface().loadContent(section, content_id);
            return true;
        };

        AsyncFuncTask task(cb, "Loading", "Loading section content", false);
        task.runAsyncDialog();
    }
    else
    {
        //directly run
        result = COMPASS::instance().dbInterface().loadContent(section, content_id);
    }

    if (!result.ok())
    {
        logerr << "TaskManager: loadResults: Could not load stored content: " << result.error();
        return std::shared_ptr<ResultReport::SectionContent>();
    }

    return result.result();
}

/**
 */
void TaskManager::loadResults()
{
    assert (!current_result_);

    results_.clear();
    
    auto res = COMPASS::instance().dbInterface().loadResults();
    if (!res.ok())
    {
        logerr << "TaskManager: loadResults: Could not load stored results: " << res.error();
        return;
    }

    for (const auto& r : res.result())
        results_[ r->id() ] = r;

    loginf << "TaskManager: loadResults: Loaded " << results_.size() << " result(s)";

    emit taskResultsChangedSignal();
}

/**
 */
void TaskManager::clearResults()
{
    assert (!current_result_);

    results_.clear();
    
    emit taskResultsChangedSignal();
}

/**
 */
void TaskManager::restoreBackupSection()
{
    if (widget_)
        widget_->restoreBackupSection();
}

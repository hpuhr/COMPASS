#include "managesectorstask.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "managesectorstaskwidget.h"
#include "taskmanager.h"
#include "savedfile.h"
#include "files.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

ManageSectorsTask::ManageSectorsTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
      : Task("ManageSectorsTask", "Manage Sectors", true, true, task_manager),
        Configurable(class_id, instance_id, &task_manager, "task_manage_sectors.json")
{
    registerParameter("current_filename", &current_filename_, "");

    createSubConfigurables();

    tooltip_ =
        "Allows management of sectors stored in the database. "
        "This task can not be run, but is performed using the GUI elements.";
}

TaskWidget* ManageSectorsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ManageSectorsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ManageSectorsTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void ManageSectorsTask::deleteWidget() { widget_.reset(nullptr); }

void ManageSectorsTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "SectorsFile")
    {
        SavedFile* file = new SavedFile(class_id, instance_id, this);
        assert(file_list_.count(file->name()) == 0);
        file_list_.insert(std::pair<std::string, SavedFile*>(file->name(), file));
    }
    else
        throw std::runtime_error("ManageSectorsTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool ManageSectorsTask::checkPrerequisites() { return ATSDB::instance().interface().ready(); }

void ManageSectorsTask::addFile(const std::string& filename)
{
    loginf << "ManageSectorsTask: addFile: filename '" << filename << "'";

    if (file_list_.count(filename) != 0)
        throw std::invalid_argument("ManageSectorsTask: addFile: name '" + filename +
                                    "' already in use");

    std::string instancename = filename;
    instancename.erase(std::remove(instancename.begin(), instancename.end(), '/'),
                       instancename.end());

    Configuration& config = addNewSubConfiguration("SectorsFile", "SectorsFile" + instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable("SectorsFile", "SectorsFile" + instancename);

    current_filename_ = filename;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::removeCurrentFilename()
{
    loginf << "ManageSectorsTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert(current_filename_.size());
    assert(hasFile(current_filename_));

    if (file_list_.count(current_filename_) != 1)
        throw std::invalid_argument("ManageSectorsTask: removeCurrentFilename: name '" +
                                    current_filename_ + "' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::removeAllFiles ()
{
    loginf << "ManageSectorsTask: removeAllFiles";

    while (file_list_.size())
    {
        delete file_list_.begin()->second;
        file_list_.erase(file_list_.begin());
    }

    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::currentFilename(const std::string& filename)
{
    loginf << "ManageSectorsTask: currentFilename: filename '" << filename << "'";

    bool had_filename = canImportFile();

    current_filename_ = filename;

    if (!had_filename)  // not on re-select
        emit statusChangedSignal(name_);
}

bool ManageSectorsTask::canImportFile()
{
    if (!current_filename_.size())
        return false;

    if (!Files::fileExists(current_filename_))
    {
        loginf << "ManageSectorsTask: canImportFile: not possible since file '"
               << current_filename_ << "'does not exist";
        return false;
    }

    return true;
}

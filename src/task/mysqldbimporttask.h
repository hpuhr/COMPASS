#ifndef MYSQLDBIMPORTTASK_H
#define MYSQLDBIMPORTTASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class SavedFile;
class MySQLDBImportTaskWidget;

class MySQLDBImportTask : public Task, public Configurable
{
public:
    MySQLDBImportTask(const std::string& class_id, const std::string& instance_id, TaskManager& task_manager);
    virtual ~MySQLDBImportTask();

    virtual QWidget* widget ();
    virtual void deleteWidget ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    bool canImportFile ();
    virtual bool canRun();
    virtual void run ();

    const std::map <std::string, SavedFile*> &fileList () { return file_list_; }
    bool hasFile (const std::string& filename) { return file_list_.count (filename) > 0; }
    void addFile (const std::string& filename);
    void removeCurrentFilename ();
    void currentFilename (const std::string& filename);
    const std::string &currentFilename () { return current_filename_; }

    virtual bool checkPrerequisites ();
    virtual bool isRecommended ();
    virtual bool isRequired ();

protected:
    std::map <std::string, SavedFile*> file_list_;
    std::string current_filename_;

    std::unique_ptr<MySQLDBImportTaskWidget> widget_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    virtual void checkSubConfigurables ();
};

#endif // MYSQLDBIMPORTTASK_H

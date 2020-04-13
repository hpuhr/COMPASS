/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYSQLDBIMPORTTASK_H
#define MYSQLDBIMPORTTASK_H

#include <QObject>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "configurable.h"
#include "task.h"

class TaskManager;
class SavedFile;
class MySQLDBImportTaskWidget;
class MySQLDBImportJob;

class QMessageBox;

class MySQLDBImportTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void importDoneSlot();
    void importObsoleteSlot();
    void importStatusSlot(std::string message);

  public:
    MySQLDBImportTask(const std::string& class_id, const std::string& instance_id,
                      TaskManager& task_manager);
    virtual ~MySQLDBImportTask();

    virtual TaskWidget* widget();
    virtual void deleteWidget();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool canImportFile();
    virtual bool canRun();
    virtual void run();

    const std::map<std::string, SavedFile*>& fileList() { return file_list_; }
    bool hasFile(const std::string& filename) { return file_list_.count(filename) > 0; }
    void addFile(const std::string& filename);
    void removeCurrentFilename();
    void currentFilename(const std::string& filename);
    const std::string& currentFilename() { return current_filename_; }

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

  protected:
    std::map<std::string, SavedFile*> file_list_;
    std::string current_filename_;

    std::unique_ptr<MySQLDBImportTaskWidget> widget_;
    std::shared_ptr<MySQLDBImportJob> import_job_;

    QMessageBox* msg_box_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    virtual void checkSubConfigurables();
};

#endif  // MYSQLDBIMPORTTASK_H

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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "singleton.h"
#include "configurable.h"
#include "global.h"
#include "task.h"

#include <QObject>

class ATSDB;
class DatabaseOpenTask;
class ManageSchemaTask;
class ManageDBObjectsTask;
class MySQLDBImportTask;
class CreateARTASAssociationsTask;
class JSONImporterTask;
class RadarPlotPositionCalculatorTask;
class PostProcessTask;
class TaskManagerWidget;

#if USE_JASTERIX
class ASTERIXImporterTask;
#endif

class TaskManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void startInspectionSignal ();

public slots:
    void taskStatusChangesSlot (std::string task_name);
    void taskDoneSlot (std::string task_name);

    void dbObjectsChangedSlot ();
    void schemaChangedSlot ();

public:
    TaskManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb);

    virtual ~TaskManager();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    //void deleteWidgets ();
    void shutdown ();

    TaskManagerWidget* widget(); // owned here

    std::vector<std::string> taskList() const;
    std::map<std::string, Task *> tasks() const;

    bool expertMode() const;
    void expertMode(bool value);

    void appendSuccess(const std::string& text);
    void appendInfo(const std::string& text);
    void appendWarning(const std::string& text);
    void appendError(const std::string& text);

    void runTask (const std::string& task_name);

protected:
    bool expert_mode_ {false};

    std::unique_ptr<DatabaseOpenTask> database_open_task_;
    std::unique_ptr<ManageSchemaTask> manage_schema_task_;
    std::unique_ptr<ManageDBObjectsTask> manage_dbobjects_task_;
#if USE_JASTERIX
    std::unique_ptr<ASTERIXImporterTask> asterix_importer_task_;
#endif
    std::unique_ptr<JSONImporterTask> json_importer_task_;
    std::unique_ptr<MySQLDBImportTask> mysqldb_import_task_;
    std::unique_ptr<RadarPlotPositionCalculatorTask> radar_plot_position_calculator_task_;
    std::unique_ptr<CreateARTASAssociationsTask> create_artas_associations_task_;
    std::unique_ptr<PostProcessTask> post_process_task_;

    std::unique_ptr<TaskManagerWidget> widget_;

    virtual void checkSubConfigurables ();

    std::vector <std::string> task_list_;
    std::map <std::string, Task*> tasks_;

    void addTask (const std::string& class_id, Task* task);
};

#endif // TASKMANAGER_H

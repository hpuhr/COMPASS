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

#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"
#include "json.hpp"
#include "jsonparsingschema.h"
#include "task.h"

#include <QObject>
#include <memory>
#include <set>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class JSONImportTaskWidget;
class SavedFile;
class JSONParseJob;
class JSONMappingJob;
class ReadJSONFileJob;
class DBObject;
class Buffer;
class DBOVariableSet;

class QMessageBox;

class JSONImportTask : public Task, public Configurable
{
    Q_OBJECT

    using JSONParsingSchemaIterator = std::map<std::string, JSONParsingSchema>::iterator;

  public slots:
    void addReadJSONSlot();
    void readJSONFileDoneSlot();
    void readJSONFileObsoleteSlot();

    void parseJSONDoneSlot();
    void parseJSONObsoleteSlot();

    void mapJSONDoneSlot();
    void mapJSONObsoleteSlot();

    void insertProgressSlot(float percent);
    void insertDoneSlot(DBObject& object);

  public:
    JSONImportTask(const std::string& class_id, const std::string& instance_id,
                   TaskManager& task_manager);
    virtual ~JSONImportTask();

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
    void removeAllFiles ();
    void currentFilename(const std::string& filename);
    const std::string& currentFilename() { return current_filename_; }

    JSONParsingSchemaIterator begin() { return schemas_.begin(); }
    JSONParsingSchemaIterator end() { return schemas_.end(); }
    bool hasSchema(const std::string& name) { return schemas_.count(name) > 0; }
    bool hasCurrentSchema();
    JSONParsingSchema& currentSchema();
    void removeCurrentSchema();

    std::string currentSchemaName() const;
    void currentSchemaName(const std::string& currentSchema);

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

    void test(bool test);

    size_t objectsInserted() const;

  protected:
    std::map<std::string, SavedFile*> file_list_;
    std::string current_filename_;

    std::unique_ptr<JSONImportTaskWidget> widget_;

    std::string current_schema_;
    std::map<std::string, JSONParsingSchema> schemas_;

    size_t insert_active_{0};

    std::map<std::string, std::tuple<std::string, DBOVariableSet>> dbo_variable_sets_;
    std::set<int> added_data_sources_;

    std::shared_ptr<ReadJSONFileJob> read_json_job_;
    std::vector<std::shared_ptr<JSONParseJob>> json_parse_jobs_;
    std::vector<std::shared_ptr<JSONMappingJob>> json_map_jobs_;

    bool test_{false};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    size_t bytes_read_{0};
    size_t bytes_to_read_{0};
    float read_status_percent_{0.0};

    size_t objects_read_{0};
    size_t objects_parsed_{0};
    size_t objects_parse_errors_{0};

    size_t objects_mapped_{0};
    size_t objects_not_mapped_{0};

    size_t objects_created_{0};
    size_t objects_inserted_{0};
    bool all_done_{false};

    size_t statistics_calc_objects_inserted_{0};
    std::string object_rate_str_;
    std::string remaining_time_str_;

    std::unique_ptr<QMessageBox> msg_box_;

    void insertData(std::map<std::string, std::shared_ptr<Buffer>> job_buffers);

    void checkAllDone();

    void updateMsgBox();

    bool maxLoadReached();

    virtual void checkSubConfigurables() {}
};

#endif  // JSONIMPORTERTASK_H

#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"
#include "json.hpp"
#include "jsonmapping.h"
#include "readjsonfilepartjob.h"

#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class JSONImporterTaskWidget;
class SavedFile;
class QMessageBox;
class JSONParseJob;
class JSONMappingJob;

class JSONImporterTask : public QObject, public Configurable
{
        Q_OBJECT
public slots:
    void insertProgressSlot (float percent);
    void insertDoneSlot (DBObject& object);

    void readJSONFilePartDoneSlot ();
    void readJSONFilePartObsoleteSlot ();

    void parseJSONDoneSlot ();
    void parseJSONObsoleteSlot ();

    void mapJSONDoneSlot ();
    void mapJSONObsoleteSlot ();

public:
    JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager* task_manager);
    virtual ~JSONImporterTask();

    JSONImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    bool canImportFile (const std::string& filename);
    void importFile (const std::string& filename, bool test);
    void importFileArchive (const std::string& filename, bool test);

    const std::map <std::string, SavedFile*> &fileList () { return file_list_; }
    bool hasFile (const std::string &filename) { return file_list_.count (filename) > 0; }
    void addFile (const std::string &filename);
    void removeFile (const std::string &filename);

    const std::string &lastFilename () { return last_filename_; }

protected:
    std::map <std::string, SavedFile*> file_list_;
    std::string last_filename_;

    std::vector <JsonMapping> mappings_;
    size_t key_count_ {0};

    JSONImporterTaskWidget* widget_ {nullptr};

    unsigned int insert_active_ {0};
    unsigned int rec_num_cnt_ {0};
    unsigned int all_cnt_ {0};
    unsigned int skipped_cnt_ {0};
    unsigned int inserted_cnt_ {0};

    std::set <int> added_data_sources_;

    std::shared_ptr <ReadJSONFilePartJob> read_json_job_;
    std::vector<std::shared_ptr <JSONParseJob>> json_parse_jobs_;
    std::vector<std::shared_ptr <JSONMappingJob>> json_map_jobs_;

    std::string filename_;
    bool test_ {false};
    bool archive_ {false};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    size_t bytes_read_ {0};
    size_t bytes_to_read_ {0};
    float read_status_percent_ {0.0};
    unsigned int objects_read_ {0};
    unsigned int objects_parsed_ {0};
    unsigned int objects_skipped_ {0};
    unsigned int objects_mapped_ {0};
    unsigned int objects_inserted_ {0};
    bool all_done_ {false};

    size_t statistics_calc_objects_inserted_ {0};
    std::string object_rate_str_ {"0"};
    std::string remaining_time_str_ {"?"};

    std::map <std::string, std::shared_ptr<Buffer>> buffers_;

    QMessageBox* msg_box_ {nullptr};

    void createMappings ();
    void insertData ();
    //void clearData ();

    void updateMsgBox ();
};

#endif // JSONIMPORTERTASK_H

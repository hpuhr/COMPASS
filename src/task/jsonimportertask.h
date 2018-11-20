#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"
#include "json.hpp"
#include "jsonmapping.h"
#include "readjsonfilepartjob.h"

#include <QObject>

#include <memory>

class TaskManager;
class JSONImporterTaskWidget;
class SavedFile;

class JSONImporterTask : public QObject, public Configurable
{
        Q_OBJECT
public slots:
    void insertProgressSlot (float percent);
    void insertDoneSlot (DBObject& object);

    void readJSONFilePartDoneSlot ();
    void readJSONFilePartObsoleteSlot ();

public:
    JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager* task_manager);
    virtual ~JSONImporterTask();

    JSONImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

//    std::string dbObjectStr() const;
//    void dbObjectStr(const std::string& db_object_str);

    bool canImportFile (const std::string& filename);
    void importFile (const std::string& filename, bool test);
    void importFileArchive (const std::string& filename, bool test);

    const std::map <std::string, SavedFile*> &fileList () { return file_list_; }
    bool hasFile (const std::string &filename) { return file_list_.count (filename) > 0; }
    void addFile (const std::string &filename);
    void removeFile (const std::string &filename);

    const std::string &lastFilename () { return last_filename_; }

    bool useTimeFilter() const;
    void useTimeFilter(bool value);

    float timeFilterMin() const;
    void timeFilterMin(float value);

    float timeFilterMax() const;
    void timeFilterMax(float value);

    bool joinDataSources() const;
    void joinDataSources(bool value);

    bool separateMLATData() const;
    void separateMLATData(bool value);

    bool usePositionFilter() const;
    void usePositionFilter(bool value);

    float positionFilterLatitudeMin() const;
    void positionFilterLatitudeMin(float value);

    float positionFilterLatitudeMax() const;
    void positionFilterLatitudeMax(float value);

    float positionFilterLongitudeMin() const;
    void positionFilterLongitudeMin(float value);

    float positionFilterLongitudeMax() const;
    void positionFilterLongitudeMax(float value);

protected:
    std::map <std::string, SavedFile*> file_list_;
    std::string last_filename_;

    std::vector <JsonMapping> mappings_;

    JSONImporterTaskWidget* widget_ {nullptr};

    unsigned int insert_active_ {0};
    unsigned int rec_num_cnt_ {0};
    unsigned int all_cnt_ {0};
    unsigned int skipped_cnt_ {0};
    unsigned int inserted_cnt_ {0};

    bool use_time_filter_ {false};
    float time_filter_min_ {0};
    float time_filter_max_ {0};

    bool use_position_filter_ {false};
    float pos_filter_lat_min_ {0};
    float pos_filter_lat_max_ {0};
    float pos_filter_lon_min_ {0};
    float pos_filter_lon_max_ {0};

    bool join_data_sources_ {false};
    bool separate_mlat_data_ {false};

    std::set <int> added_data_sources_;

    std::shared_ptr <ReadJSONFilePartJob> read_json_job_;
    bool test_ {false};

    void parseJSON (nlohmann::json& j, bool test);
    void transformBuffers ();
    void insertData ();
    void clearData ();
};

#endif // JSONIMPORTERTASK_H

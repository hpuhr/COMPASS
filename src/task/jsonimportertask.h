#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"

#include <QObject>

class TaskManager;
class JSONImporterTaskWidget;
class SavedFile;
class DBObject;
class DBOVariable;

namespace Json
{
    class Value;
}

class JSONImporterTask : public QObject, public Configurable
{
        Q_OBJECT
public slots:
    void insertProgressSlot (float percent);
    void insertDoneSlot (DBObject& object);

public:
    JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager* task_manager);
    virtual ~JSONImporterTask();

    JSONImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::string dbObjectStr() const;
    void dbObjectStr(const std::string& db_object_str);

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

    std::string db_object_str_;
    DBObject* db_object_{nullptr};

    std::string key_var_str_;
    DBOVariable* key_var_{nullptr};

    std::string dsid_var_str_;
    DBOVariable* dsid_var_{nullptr};

    std::string target_addr_var_str_;
    DBOVariable* target_addr_var_{nullptr};

    std::string callsign_var_str_;
    DBOVariable* callsign_var_{nullptr};

    std::string altitude_baro_var_str_;
    DBOVariable* altitude_baro_var_{nullptr};

    std::string altitude_geo_var_str_;
    DBOVariable* altitude_geo_var_{nullptr};

    std::string latitude_var_str_;
    DBOVariable* latitude_var_{nullptr};

    std::string longitude_var_str_;
    DBOVariable* longitude_var_{nullptr};

    std::string tod_var_str_;
    DBOVariable* tod_var_{nullptr};

    JSONImporterTaskWidget* widget_ {nullptr};

    bool insert_active_ {false};
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

    std::map <int, std::string> datasources_existing_;

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);

    void parseJSON (Json::Value& object, bool test);
};

#endif // JSONIMPORTERTASK_H

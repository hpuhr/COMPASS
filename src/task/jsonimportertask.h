#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"

#include <QObject>

class TaskManager;
class JSONImporterTaskWidget;
class SavedFile;
class DBObject;
class DBOVariable;

class JSONImporterTask : public QObject, public Configurable
{
        Q_OBJECT
public:
    JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager* task_manager);
    virtual ~JSONImporterTask();

    JSONImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::string dbObjectStr() const;
    void dbObjectStr(const std::string& db_object_str);

    bool canImportFile (const std::string& filename);
    bool importFile (const std::string& filename, bool test);

    const std::map <std::string, SavedFile*> &fileList () { return file_list_; }
    bool hasFile (const std::string &filename) { return file_list_.count (filename) > 0; }
    void addFile (const std::string &filename);
    void removeFile (const std::string &filename);

    const std::string &lastFilename () { return last_filename_; }

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

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);
};

#endif // JSONIMPORTERTASK_H

#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"

#include <QObject>

class TaskManager;
class JSONImporterTaskWidget;
class SavedFile;
class DBObject;

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

    bool importFile(const std::string& filename, bool test);

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


    JSONImporterTaskWidget* widget_ {nullptr};
};

#endif // JSONIMPORTERTASK_H

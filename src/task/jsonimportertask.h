#ifndef JSONIMPORTERTASK_H
#define JSONIMPORTERTASK_H

#include "configurable.h"

#include <QObject>

class TaskManager;
class JSONImporterTaskWidget;
class DBObject;

class JSONImporterTask : public QObject, public Configurable
{
        Q_OBJECT
public:
    JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager* task_manager);
    virtual ~JSONImporterTask();

    JSONImporterTaskWidget* widget();

    std::string dbObjectStr() const;
    void dbObjectStr(const std::string& db_object_str);

protected:
    std::string db_object_str_;
    DBObject* db_object_{nullptr};

    JSONImporterTaskWidget* widget_ {nullptr};
};

#endif // JSONIMPORTERTASK_H

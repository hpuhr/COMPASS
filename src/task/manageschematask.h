#ifndef MANAGESCHEMATASK_H
#define MANAGESCHEMATASK_H

#include "configurable.h"
#include "task.h"
#include "manageschemataskwidget.h"

#include <QObject>

#include <memory>

class TaskManager;


class ManageSchemaTask: public QObject, public Configurable, public Task
{
public:
    ManageSchemaTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual QWidget* widget ();

protected:
    std::unique_ptr<ManageSchemaTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // MANAGESCHEMATASK_H

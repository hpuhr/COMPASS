#ifndef MANAGEDBOBJECTSTASK_H
#define MANAGEDBOBJECTSTASK_H

#include "configurable.h"
#include "task.h"
#include "managedbobjectstaskwidget.h"

#include <QObject>

#include <memory>

class TaskManager;

class ManageDBObjectsTask : public QObject, public Configurable, public Task
{
public:
    ManageDBObjectsTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual QWidget* widget ();

protected:
    std::unique_ptr<ManageDBObjectsTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // MANAGEDBOBJECTSTASK_H

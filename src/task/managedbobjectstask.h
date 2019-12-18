#ifndef MANAGEDBOBJECTSTASK_H
#define MANAGEDBOBJECTSTASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>

#include <memory>

class TaskManager;
class ManageDBObjectsTaskWidget;

class ManageDBObjectsTask : public Task, public Configurable
{
public:
    ManageDBObjectsTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual TaskWidget* widget ();
    virtual void deleteWidget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended () { return false; }
    virtual bool isRequired () { return false; }

protected:
    std::unique_ptr<ManageDBObjectsTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // MANAGEDBOBJECTSTASK_H

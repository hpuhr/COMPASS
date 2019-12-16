#ifndef MANAGESCHEMATASK_H
#define MANAGESCHEMATASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>

#include <memory>

class TaskManager;
class ManageSchemaTaskWidget;

class ManageSchemaTask: public Task, public Configurable
{
public:
    ManageSchemaTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual QWidget* widget ();
    virtual void deleteWidget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended () { return false; }
    virtual bool isRequired () { return false; }

protected:
    std::unique_ptr<ManageSchemaTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // MANAGESCHEMATASK_H

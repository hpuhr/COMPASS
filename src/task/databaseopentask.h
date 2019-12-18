#ifndef DATABASEOPENTASK_H
#define DATABASEOPENTASK_H

#include "configurable.h"
#include "task.h"

#include <QObject>

#include <memory>

class DatabaseOpenTaskWidget;
class TaskManager;

class DatabaseOpenTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void databaseOpenedSlot();

public:
    DatabaseOpenTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual TaskWidget* widget ();
    virtual void deleteWidget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended ();
    virtual bool isRequired ();

protected:
    std::unique_ptr<DatabaseOpenTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // DATABASEOPENTASK_H

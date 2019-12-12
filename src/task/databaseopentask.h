#ifndef DATABASEOPENTASK_H
#define DATABASEOPENTASK_H

#include "configurable.h"
#include "task.h"
#include "databaseopentaskwidget.h"

#include <QObject>

#include <memory>

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

    virtual QWidget* widget ();

    virtual bool checkPrerequisites ();

protected:
    std::unique_ptr<DatabaseOpenTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // DATABASEOPENTASK_H

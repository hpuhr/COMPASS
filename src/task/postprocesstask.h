#ifndef POSTPROCESSTASK_H
#define POSTPROCESSTASK_H

#include "configurable.h"
#include "task.h"
#include "postprocesstaskwidget.h"

#include <QObject>

#include <memory>

class TaskManager;


class PostProcessTask: public Task, public Configurable
{
    Q_OBJECT

public slots:
    void postProcessingDoneSlot ();

public:
    PostProcessTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual QWidget* widget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended () { return true; }
    virtual bool isRequired () { return true; }

    void run ();

protected:
    std::unique_ptr<PostProcessTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // POSTPROCESSTASK_H

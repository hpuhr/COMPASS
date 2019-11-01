#ifndef CREATEARTASASSOCIATIONSTASK_H
#define CREATEARTASASSOCIATIONSTASK_H

#include "configurable.h"
#include "createartasassociationsjob.h"

#include <QObject>

class TaskManager;
class CreateARTASAssociationsTaskWidget;

class CreateARTASAssociationsTask : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void createDoneSlot ();
    void createObsoleteSlot ();

public:
    CreateARTASAssociationsTask(const std::string& class_id, const std::string& instance_id,
                                TaskManager* task_manager);
    virtual ~CreateARTASAssociationsTask();

    CreateARTASAssociationsTaskWidget* widget();

    void run ();

    std::string currentDataSourceName() const;
    void currentDataSourceName(const std::string &currentDataSourceName);

protected:
    std::string current_data_source_name_;

    CreateARTASAssociationsTaskWidget* widget_ {nullptr};

    std::shared_ptr<CreateARTASAssociationsJob> create_job_;
};

#endif // CREATEARTASASSOCIATIONSTASK_H

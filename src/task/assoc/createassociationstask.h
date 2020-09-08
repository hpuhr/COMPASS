#ifndef CREATEASSOCIATIONSTASK_H
#define CREATEASSOCIATIONSTASK_H

#include "configurable.h"
#include "dbovariableset.h"
#include "task.h"

#include <QObject>
#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class CreateAssociationsTaskWidget;
class DBOVariable;
class MetaDBOVariable;
class DBObject;
class Buffer;
class CreateAssociationsJob;

class CreateAssociationsTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void createDoneSlot();
    void createObsoleteSlot();

    void newDataSlot(DBObject& object);
    void loadingDoneSlot(DBObject& object);

    void associationStatusSlot(QString status);

public:
    CreateAssociationsTask(const std::string& class_id, const std::string& instance_id,
                           TaskManager& task_manager);
    virtual ~CreateAssociationsTask();

    TaskWidget* widget();
    virtual void deleteWidget();

    std::string keyVarStr() const;
    void keyVarStr(const std::string& var_str);

    std::string todVarStr() const;
    void todVarStr(const std::string& var_str);

    std::string targetAddrVarStr() const;
    void targetAddrVarStr(const std::string& var_str);

    MetaDBOVariable* keyVar() const;
    MetaDBOVariable* todVar() const;
    MetaDBOVariable* targetAddrVar() const;

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired() { return false; }

    bool canRun();
    void run();

    static const std::string DONE_PROPERTY_NAME;

protected:
    std::string key_var_str_;
    MetaDBOVariable* key_var_{nullptr};

    std::string tod_var_str_;
    MetaDBOVariable* tod_var_{nullptr};

    std::string target_addr_var_str_;
    MetaDBOVariable* target_addr_var_{nullptr};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    std::unique_ptr<CreateAssociationsTaskWidget> widget_;

    std::map<std::string, bool> dbo_loading_done_flags_;
    bool dbo_loading_done_{false};

    std::shared_ptr<CreateAssociationsJob> create_job_;
    bool create_job_done_{false};

    void checkAndSetMetaVariable(std::string& name_str, MetaDBOVariable** var);

    DBOVariableSet getReadSetFor(const std::string& dbo_name);
};

#endif // CREATEASSOCIATIONSTASK_H

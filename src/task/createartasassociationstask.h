#ifndef CREATEARTASASSOCIATIONSTASK_H
#define CREATEARTASASSOCIATIONSTASK_H

#include "configurable.h"
#include "createartasassociationsjob.h"
#include "dbovariableset.h"

#include <QObject>
#include <memory>

class TaskManager;
class CreateARTASAssociationsTaskWidget;
class DBOVariable;
class MetaDBOVariable;
class QMessageBox;
class DBObject;
class Buffer;

class CreateARTASAssociationsTask : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void createDoneSlot ();
    void createObsoleteSlot ();

    void newDataSlot (DBObject& object);
    void loadingDoneSlot (DBObject& object);

public:
    CreateARTASAssociationsTask(const std::string& class_id, const std::string& instance_id,
                                TaskManager* task_manager);
    virtual ~CreateARTASAssociationsTask();

    CreateARTASAssociationsTaskWidget* widget();

    bool canRun ();
    void run ();

    std::string currentDataSourceName() const;
    void currentDataSourceName(const std::string &currentDataSourceName);

    std::string trackerTRIVarStr() const;
    void trackerTRIVarStr(const std::string &trackerTRIVarStr);

    std::string trackerTrackNumVarStr() const;
    void trackerTrackNumVarStr(const std::string &tracker_track_num_var_str);

    std::string trackerTrackBeginVarStr() const;
    void trackerTrackBeginVarStr(const std::string &tracker_track_begin_var_str);

    std::string trackerTrackEndVarStr() const;
    void trackerTrackEndVarStr(const std::string &tracker_track_end_var_str);

    std::string keyVarStr() const;
    void keyVarStr(const std::string &keyVarStr);

    std::string hashVarStr() const;
    void hashVarStr(const std::string &hashVarStr);

    std::string todVarStr() const;
    void todVarStr(const std::string &todVarStr);

protected:
    std::string current_data_source_name_;

    std::string tracker_tri_var_str_;
    DBOVariable* tracker_tri_var_ {nullptr};

    std::string tracker_track_num_var_str_;
    DBOVariable* tracker_track_num_var_ {nullptr};

    std::string tracker_track_begin_var_str_;
    DBOVariable* tracker_track_begin_var_ {nullptr};

    std::string tracker_track_end_var_str_;
    DBOVariable* tracker_track_end_var_ {nullptr};

    std::string key_var_str_;
    MetaDBOVariable* key_var_ {nullptr};

    std::string hash_var_str_;
    MetaDBOVariable* hash_var_ {nullptr};

    std::string tod_var_str_;
    MetaDBOVariable* tod_var_ {nullptr};

    CreateARTASAssociationsTaskWidget* widget_ {nullptr};

    QMessageBox* msg_box_ {nullptr};

    std::map<std::string, bool> dbo_loading_done_flags_;
    bool dbo_loading_done_ {false};

    //std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::shared_ptr<CreateARTASAssociationsJob> create_job_;
    bool create_job_done_ {false};

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);
    void checkAndSetMetaVariable (std::string &name_str, MetaDBOVariable** var);

    DBOVariableSet getReadSetFor (const std::string& dbo_name);

    void updateProgressSlot();
};

#endif // CREATEARTASASSOCIATIONSTASK_H

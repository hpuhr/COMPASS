#ifndef CREATEARTASASSOCIATIONSTASK_H
#define CREATEARTASASSOCIATIONSTASK_H

#include "configurable.h"
#include "createartasassociationsjob.h"
#include "dbovariableset.h"

#include <QObject>
#include <memory>

class TaskManager;
class CreateARTASAssociationsTaskWidget;
class CreateARTASAssociationsStatusDialog;
class DBOVariable;
class MetaDBOVariable;
//class QMessageBox;
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

    void associationStatusSlot (QString status);

    void closeStatusDialogSlot();

public:
    CreateARTASAssociationsTask(const std::string& class_id, const std::string& instance_id,
                                TaskManager* task_manager);
    virtual ~CreateARTASAssociationsTask();

    bool hasOpenWidget() { return widget_ != nullptr; }
    CreateARTASAssociationsTaskWidget* widget();

    bool canRun ();
    void run ();

    std::string currentDataSourceName() const;
    void currentDataSourceName(const std::string &currentDataSourceName);

    std::string trackerDsIdVarStr() const;
    void trackerDsIdVarStr(const std::string &tracker_ds_id_var_str);
    DBOVariable *trackerDsIdVar() const;

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

    MetaDBOVariable* keyVar() const;

    MetaDBOVariable* hashVar() const;

    MetaDBOVariable* todVar() const;


    float beginningTime() const;
    void beginningTime(float beginning_time);

    float dubiousTime() const;
    void dubiousTime(float dubious_time);

    float endTrackTime() const;
    void endTrackTime(float end_track_time);

    float futureTime() const;
    void futureTime(float future_time);

protected:
    std::string current_data_source_name_;

    std::string tracker_ds_id_var_str_;
    DBOVariable* tracker_ds_id_var_ {nullptr};

    std::string tracker_track_num_var_str_;
    DBOVariable* tracker_track_num_var_ {nullptr};

    std::string tracker_track_begin_var_str_;
    DBOVariable* tracker_track_begin_var_ {nullptr};

    std::string tracker_track_end_var_str_;
    DBOVariable* tracker_track_end_var_ {nullptr};

    std::string key_var_str_;
    MetaDBOVariable* key_var_ {nullptr};

    // contains artas md5 for target reports, tris for tracker
    std::string hash_var_str_;
    MetaDBOVariable* hash_var_ {nullptr};

    std::string tod_var_str_;
    MetaDBOVariable* tod_var_ {nullptr};

    float end_track_time_ {0};
    float beginning_time_ {0};
    float dubious_time_ {0};
    float future_time_ {0};

    CreateARTASAssociationsTaskWidget* widget_ {nullptr};

    //QMessageBox* msg_box_ {nullptr};
    std::unique_ptr<CreateARTASAssociationsStatusDialog> status_dialog_ {nullptr};

    std::map<std::string, bool> dbo_loading_done_flags_;
    bool dbo_loading_done_ {false};

    std::shared_ptr<CreateARTASAssociationsJob> create_job_;
    bool create_job_done_ {false};

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);
    void checkAndSetMetaVariable (std::string &name_str, MetaDBOVariable** var);

    DBOVariableSet getReadSetFor (const std::string& dbo_name);

    //void updateProgressSlot();
};

#endif // CREATEARTASASSOCIATIONSTASK_H

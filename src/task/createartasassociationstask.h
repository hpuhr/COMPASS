#ifndef CREATEARTASASSOCIATIONSTASK_H
#define CREATEARTASASSOCIATIONSTASK_H

#include "configurable.h"
#include "createartasassociationsjob.h"
#include "createartasassociationsstatusdialog.h"
#include "dbovariableset.h"
#include "task.h"

#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class CreateARTASAssociationsTaskWidget;
class DBOVariable;
class MetaDBOVariable;
class DBObject;
class Buffer;

class CreateARTASAssociationsTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void createDoneSlot ();
    void createObsoleteSlot ();

    void newDataSlot (DBObject& object);
    void loadingDoneSlot (DBObject& object);

    void associationStatusSlot (QString status);
    void saveAssociationsQuestionSlot (QString question_str);

    void closeStatusDialogSlot();

public:
    CreateARTASAssociationsTask(const std::string& class_id, const std::string& instance_id,
                                TaskManager& task_manager);
    virtual ~CreateARTASAssociationsTask();

    bool hasOpenWidget() { return widget_ != nullptr; }
    QWidget* widget();

    std::string currentDataSourceName() const;
    void currentDataSourceName(const std::string &currentDataSourceName);

    std::string trackerDsIdVarStr() const;
    void trackerDsIdVarStr(const std::string& var_str);
    DBOVariable *trackerDsIdVar() const;

    std::string trackerTrackNumVarStr() const;
    void trackerTrackNumVarStr(const std::string& var_str);

    std::string trackerTrackBeginVarStr() const;
    void trackerTrackBeginVarStr(const std::string& var_str);

    std::string trackerTrackEndVarStr() const;
    void trackerTrackEndVarStr(const std::string& var_str);

    std::string trackerTrackCoastingVarStr() const;
    void trackerTrackCoastingVarStr(const std::string& var_str);

    std::string keyVarStr() const;
    void keyVarStr(const std::string &keyVarStr);

    std::string hashVarStr() const;
    void hashVarStr(const std::string &hashVarStr);

    std::string todVarStr() const;
    void todVarStr(const std::string &todVarStr);

    MetaDBOVariable* keyVar() const;

    MetaDBOVariable* hashVar() const;

    MetaDBOVariable* todVar() const;

    float endTrackTime() const;
    void endTrackTime(float end_track_time);

    float associationTimePast() const;
    void associationTimePast(float association_time_past);

    float associationTimeFuture() const;
    void associationTimeFuture(float association_time_future);

    float missesAcceptableTime() const;
    void missesAcceptableTime(float misses_acceptable_time);

    float associationsDubiousDistantTime() const;
    void associationsDubiousDistantTime(float associations_dubious_distant_time);

    float associationDubiousCloseTimePast() const;
    void associationDubiousCloseTimePast(float association_dubious_close_time_past);

    float associationDubiousCloseTimeFuture() const;
    void associationDubiousCloseTimeFuture(float association_dubious_close_time_future);

    bool ignoreTrackEndAssociations() const;
    void ignoreTrackEndAssociations(bool value);

    bool markTrackEndAssociationsDubious() const;
    void markTrackEndAssociationsDubious(bool value);

    bool ignoreTrackCoastingAssociations() const;
    void ignoreTrackCoastingAssociations(bool value);

    bool markTrackCoastingAssociationsDubious() const;
    void markTrackCoastingAssociationsDubious(bool value);

    virtual bool checkPrerequisites ();
    virtual bool isRecommended ();
    virtual bool isRequired ()  { return false; }

    bool canRun ();
    void run ();

    static const std::string DONE_PROPERTY_NAME;

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

    std::string tracker_track_coasting_var_str_;
    DBOVariable* tracker_track_coasting_var_ {nullptr};

    std::string key_var_str_;
    MetaDBOVariable* key_var_ {nullptr};

    // contains artas md5 for target reports, tris for tracker
    std::string hash_var_str_;
    MetaDBOVariable* hash_var_ {nullptr};

    std::string tod_var_str_;
    MetaDBOVariable* tod_var_ {nullptr};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    float end_track_time_ {0}; // time-delta after which begin a new track

    float association_time_past_ {0}; // time_delta for which associations are considered into past time
    float association_time_future_ {0}; // time_delta for which associations are considered into future time

    float misses_acceptable_time_ {0}; // time delta at beginning/end of recording where misses are acceptable

    float associations_dubious_distant_time_ {0};
    // time delta of tou where association is dubious bc too distant in time
    float association_dubious_close_time_past_ {0};
    // time delta of tou where association is dubious when multible hashes exist
    float association_dubious_close_time_future_ {0};
    // time delta of tou where association is dubious when multible hashes exist

    bool ignore_track_end_associations_ {false};
    bool mark_track_end_associations_dubious_ {false};

    bool ignore_track_coasting_associations_ {false};
    bool mark_track_coasting_associations_dubious_ {false};

    CreateARTASAssociationsTaskWidget* widget_ {nullptr};

    bool save_associations_ {true};

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

#include "calculatereferencestask.h"
#include "calculatereferencestaskdialog.h"
#include "calculatereferencesstatusdialog.h"
#include "calculatereferencesjob.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "jobmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "viewabledataconfig.h"
#include "viewpointgenerator.h"
#include "buffer.h"

#include <fstream>

#include <QApplication>
#include <QMessageBox>
#include <QThread>

using namespace std;
using namespace Utils;
using namespace dbContent;
//using namespace nlohmann;

CalculateReferencesTask::CalculateReferencesTask(const std::string& class_id,
                                                 const std::string& instance_id,
                                                 TaskManager& task_manager)
    : Task("CalculateReferencesTask", "Associate Target Reports", task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_references.json")
{
    tooltip_ =
            "Allows calculation of references based on System Tracker and ADS-B data.";

    registerParameter("rec_type"     , (int*)&settings_.rec_type     , CalculateReferencesTaskSettings::Rec_UMKalman2D);
    registerParameter("rec_proj_mode", (int*)&settings_.map_proj_mode, CalculateReferencesTaskSettings::MapProjectionMode::MapProjectDynamic);

    registerParameter("rec_mm_stddev"        , &settings_.R_std     , 30.0  );
    registerParameter("rec_mm_stddev_high"   , &settings_.R_std_high, 1000.0);
    registerParameter("rec_process_stddev"   , &settings_.Q_std     , 30.0  );
    registerParameter("rec_state_stddev"     , &settings_.P_std     , 30.0  );
    registerParameter("rec_state_stddev_high", &settings_.P_std_high, 1000.0);

    registerParameter("rec_use_mm_stddev_cat021", &settings_.use_R_std_cat021, true);
    registerParameter("rec_mm_stddev_cat021_pos", &settings_.R_std_pos_cat021, 30.0);
    registerParameter("rec_mm_stddev_cat021_vel", &settings_.R_std_vel_cat021, 10.0);
    registerParameter("rec_mm_stddev_cat021_acc", &settings_.R_std_acc_cat021, 10.0);

    registerParameter("rec_use_mm_stddev_cat062", &settings_.use_R_std_cat062, true);
    registerParameter("rec_mm_stddev_cat062_pos", &settings_.R_std_pos_cat062, 30.0);
    registerParameter("rec_mm_stddev_cat062_vel", &settings_.R_std_vel_cat062, 20.0);
    registerParameter("rec_mm_stddev_cat062_acc", &settings_.R_std_acc_cat062, 20.0);

    registerParameter("rec_min_dt"        , &settings_.min_dt        ,  0.0);
    registerParameter("rec_max_dt"        , &settings_.max_dt        , 11.0);
    registerParameter("rec_min_chain_size", &settings_.min_chain_size,  2  );

    registerParameter("rec_use_velocity_mm", &settings_.use_vel_mm, true);
    registerParameter("rec_smooth_result"  , &settings_.smooth_rts, true);

    registerParameter("rec_resample_systracks"   , &settings_.resample_systracks   , true);
    registerParameter("rec_resample_systracks_dt", &settings_.resample_systracks_dt, 1.0 );
    registerParameter("rec_resample_systracks_max_dt", &settings_.resample_systracks_max_dt, 30.0);

    registerParameter("rec_resample_result"   , &settings_.resample_result   , true);
    registerParameter("rec_resample_result_dt", &settings_.resample_result_dt, 2.0 );
    registerParameter("rec_resample_result_qstd", &settings_.resample_result_Q_std, 10.0);

    registerParameter("use_tracker_data"  , &settings_.use_tracker_data, true);
    registerParameter("data_sources_tracker", &settings_.data_sources_tracker, nlohmann::json::object());

    registerParameter("use_adsb_data"  , &settings_.use_adsb_data, true);
    registerParameter("data_sources_adsb", &settings_.data_sources_adsb, nlohmann::json::object());
    
    // position usage
    registerParameter("filter_position_usage", &settings_.filter_position_usage, true);
    registerParameter("tracker_only_confirmed_positions", &settings_.tracker_only_confirmed_positions, true);
    registerParameter("tracker_only_noncoasting_positions", &settings_.tracker_only_noncoasting_positions, true);
    registerParameter("tracker_only_report_detection_positions",
                      &settings_.tracker_only_report_detection_positions, false);
    registerParameter("tracker_only_report_detection_nonpsronly_positions",
                      &settings_.tracker_only_report_detection_nonpsronly_positions, false);
    registerParameter("tracker_only_high_accuracy_postions", &settings_.tracker_only_high_accuracy_postions, true);
    registerParameter("tracker_minimum_accuracy", &settings_.tracker_minimum_accuracy, 30);

    registerParameter("adsb_only_v12_positions", &settings_.adsb_only_v12_positions, true);
    registerParameter("adsb_only_high_nucp_nic_positions", &settings_.adsb_only_high_nucp_nic_positions, false);
    registerParameter("adsb_minimum_nucp_nic", &settings_.adsb_minimum_nucp_nic, 4);
    registerParameter("adsb_only_high_nacp_positions", &settings_.adsb_only_high_nacp_positions, true);
    registerParameter("adsb_minimum_nacp", &settings_.adsb_minimum_nacp, 4);
    registerParameter("adsb_only_high_sil_positions_", &settings_.adsb_only_high_sil_positions, false);
    registerParameter("adsb_minimum_sil", &settings_.adsb_minimum_sil, 1);

    // output
    registerParameter("ds_name", &settings_.ds_name, "CalcRefTraj");
    registerParameter("ds_sac", &settings_.ds_sac, 0);
    registerParameter("ds_sic", &settings_.ds_sic, 1);
    registerParameter("ds_line", &settings_.ds_line, 0); // 0 ... 3

    assert (settings_.ds_line >= 0 && settings_.ds_line <= 3);

    //registerParameter("rec_verbose", &settings_.verbose, false);
    //registerParameter("rec_generate_viewpoints", &settings_.generate_viewpoints, false);

    //registerParameter("associate_non_mode_s", &associate_non_mode_s_, true);
}

CalculateReferencesTask::~CalculateReferencesTask() {}

CalculateReferencesTaskDialog* CalculateReferencesTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new CalculateReferencesTaskDialog(*this));

        connect(dialog_.get(), &CalculateReferencesTaskDialog::runSignal,
                this, &CalculateReferencesTask::dialogRunSlot);

        connect(dialog_.get(), &CalculateReferencesTaskDialog::cancelSignal,
                this, &CalculateReferencesTask::dialogCancelSlot);
    }
    else
        dialog_->updateSourcesWidgets();

    assert(dialog_);
    return dialog_.get();
}

bool CalculateReferencesTask::useTrackerData() const
{
    return settings_.use_tracker_data;
}

void CalculateReferencesTask::useTrackerData(bool value)
{
    loginf << "CalculateReferencesTask: useTrackerData: value " << value;

    settings_.use_tracker_data = value;
}


std::map<std::string, bool> CalculateReferencesTask::trackerDataSources()
{
    return settings_.data_sources_tracker;
}

void CalculateReferencesTask::trackerDataSources(std::map<std::string, bool> sources)
{
    loginf << "CalculateReferencesTask: trackerDataSources";

    settings_.data_sources_tracker = sources;
}

bool CalculateReferencesTask::useADSBData() const
{
    return settings_.use_adsb_data;
}

void CalculateReferencesTask::useADSBData(bool value)
{
    loginf << "CalculateReferencesTask: useADSBData: value " << value;

    settings_.use_adsb_data = value;
}

std::map<std::string, bool> CalculateReferencesTask::adsbDataSources()
{
    return settings_.data_sources_adsb;
}

void CalculateReferencesTask::adsbDataSources(std::map<std::string, bool> sources)
{
    loginf << "CalculateReferencesTask: adsbDataSources";

    settings_.data_sources_adsb = sources;
}

bool CalculateReferencesTask::anySourcesActive(const std::string& ds_type, const nlohmann::json& sources)
{
    loginf << "CalculateReferencesTask: anySourcesActive: ds_type " << ds_type;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    string ds_id_str;

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (ds_it->dsType() != ds_type)
            continue;

        ds_id_str = to_string(ds_it->id());

        if (sources.count(ds_id_str))
        {
            if (sources.at(ds_id_str))
                return true;
        }
        else
            return true; // not disabled
    }

    return false;
}

std::string CalculateReferencesTask::getActiveDataSources(const std::string& ds_type, const nlohmann::json& sources)
{
    loginf << "CalculateReferencesTask: getActiveDataSources: ds_type " << ds_type;

    string tmp;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    string ds_id_str;

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (ds_it->dsType() != ds_type)
            continue;

        ds_id_str = to_string(ds_it->id());

        if (sources.count(ds_id_str))
        {
            if (sources.at(ds_id_str))
                tmp += tmp.size() ? ", " + ds_id_str : ds_id_str;
        }
        else
            tmp += tmp.size() ? ", " + ds_id_str : ds_id_str;
    }

    loginf << "CalculateReferencesTask: getActiveDataSources: ds_type " << ds_type << " ids '" << tmp << "'";

    return tmp;
}

bool CalculateReferencesTask::canRun()
{
    if (!COMPASS::instance().dbOpened())
        return false;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (!dbcontent_man.hasData())
        return false;

    bool load_any_cat021 = dbcontent_man.dbContent("CAT021").hasData()
            && useADSBData() && anySourcesActive("ADSB", settings_.data_sources_adsb);

    bool load_any_cat062 = dbcontent_man.dbContent("CAT062").hasData()
            && useTrackerData() && anySourcesActive("Tracker", settings_.data_sources_tracker);

    if (!load_any_cat021 && !load_any_cat062)
        return false;

    return true;
}

bool CalculateReferencesTask::generateViewPoints() const
{
    return (settings_.generate_viewpoints || !utns_.empty());
}

bool CalculateReferencesTask::writeReferences() const
{
    return utns_.empty();
}

bool CalculateReferencesTask::closeDialogAfterFinishing() const
{
    return (!show_done_summary_ || !utns_.empty());
}

void CalculateReferencesTask::runUTN(unsigned int utn)
{
    utns_ = { utn };

    run();
}

void CalculateReferencesTask::run()
{
    assert(canRun());

    loginf << "CalculateReferencesTask: run: started";

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CalculateReferencesStatusDialog(*this));
    connect(status_dialog_.get(), &CalculateReferencesStatusDialog::closeSignal,
            this, &CalculateReferencesTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();
    status_dialog_->setStatusSlot("Initializing");
    status_dialog_->show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    //remove existing references if we want to write the new ones
    if (writeReferences())
    {
        status_dialog_->setStatusSlot("Deleting old References");

        if (dbcontent_man.dbContent("RefTraj").existsInDB()) // TODO rework to only delete define data source + line
            dbcontent_man.dbContent("RefTraj").deleteDBContentData();

        while (dbcontent_man.dbContent("RefTraj").isDeleting())
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(10);
        }
    }

    status_dialog_->setStatusSlot("Loading Data");

    COMPASS::instance().viewManager().disableDataDistribution(true);

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &CalculateReferencesTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &CalculateReferencesTask::loadingDoneSlot);

    bool loaded_any = false;

    for (auto& dbo_it : dbcontent_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        if (dbo_it.first != "CAT021" && dbo_it.first != "CAT062") // dbo_it.first != "CAT021" &&
            continue;

        if (dbo_it.first == "CAT021" && (!useADSBData() || !anySourcesActive("ADSB", settings_.data_sources_adsb)))
            continue;

        if (dbo_it.first == "CAT062" && (!useTrackerData() || !anySourcesActive("Tracker", settings_.data_sources_tracker)))
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        string custom_clause;

        dbContent::Variable& ds_id_var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_datasource_id_.name()).getFor(dbo_it.first);


        custom_clause += ds_id_var.dbColumnName() + " IN (";

        if (dbo_it.first == "CAT021")
            custom_clause += getActiveDataSources("ADSB", settings_.data_sources_adsb);
        else
            custom_clause += getActiveDataSources("Tracker", settings_.data_sources_tracker);

        custom_clause += ")";

        if (!utns_.empty())
        {
            //load for specified utns only
            dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                        DBContent::meta_var_utn_.name()).getFor(dbo_it.first);

            if (custom_clause.size())
                custom_clause += " AND ";
            custom_clause += var.dbColumnName() + " IN (" +  String::compress(utns_, ',') + ")";
        }

        dbo_it.second->load(read_set, false, false, custom_clause);

        loaded_any |= true;
    }

    assert (loaded_any);

    status_dialog_->show();
}

void CalculateReferencesTask::dialogRunSlot()
{
    loginf << "CalculateReferencesTask: dialogRunSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());
    run ();
}

void CalculateReferencesTask::dialogCancelSlot()
{
    loginf << "CalculateReferencesTask: dialogCancelSlot";

    assert (dialog_);
    dialog_->hide();
}

void CalculateReferencesTask::createDoneSlot()
{
    loginf << "CalculateReferencesTask: createDoneSlot";

    create_job_done_ = true;

    //handle viewpoints
    if (generateViewPoints())
    {
        auto j = create_job_->viewPointsJSON();

        if (settings_.generate_viewpoints)
        {
            //load all created view points
            status_dialog_->setStatusSlot("Adding View Points");

            COMPASS::instance().viewManager().loadViewPoints(j);
        }

        if (!utns_.empty())
        {
            //set first viewpoint as current viewable
            auto j_vp = ViewPointGenerator::viewPointJSON(j, 0, true);

            if (j_vp.has_value() && j_vp->is_object())
            {
                status_dialog_->setStatusSlot("Loading View Point");

                nlohmann::json::object_t obj = j_vp.value();

                viewable_.reset(new ViewableDataConfig(obj));

                COMPASS::instance().viewManager().setCurrentViewPoint(viewable_.get());
            }
        }
    }

    status_dialog_->setStatusSlot("Done");

    assert (create_job_);

    status_dialog_->setDone();

    if (closeDialogAfterFinishing())
        closeStatusDialogSlot();

    create_job_ = nullptr;

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    //    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");
    //    COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");
    //    COMPASS::instance().dbContentManager().resizeTargetListWidget();

    //    COMPASS::instance().interface().saveProperties();

    cache_ = nullptr;
    data_.clear();

    done_ = true;

    utns_ = {}; // all utns reconstructed

    QApplication::restoreOverrideCursor();

    emit doneSignal(name_);
}
void CalculateReferencesTask::createObsoleteSlot()
{
    create_job_ = nullptr;
    data_.clear();

}

void CalculateReferencesTask::loadedDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;

    assert (status_dialog_);

    status_dialog_->updateTime();
}

void CalculateReferencesTask::loadingDoneSlot()
{
    loginf << "CalculateReferencesTask: loadingDoneSlot";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (!cache_)
        cache_ = std::make_shared<dbContent::Cache> (dbcontent_man);

    cache_->add(data_);

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &CalculateReferencesTask::loadedDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &CalculateReferencesTask::loadingDoneSlot);

    assert(status_dialog_);
    status_dialog_->setStatusSlot("Loading done, starting calculation");
    status_dialog_->setLoadedCountsSlot(data_);

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    loginf << "CalculateReferencesTask: loadingDoneSlot: data loading done";

    assert(!create_job_);

    create_job_ = std::make_shared<CalculateReferencesJob>(*this, *status_dialog_, cache_);

    connect(create_job_.get(), &CalculateReferencesJob::doneSignal, this,
            &CalculateReferencesTask::createDoneSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CalculateReferencesJob::obsoleteSignal, this,
            &CalculateReferencesTask::createObsoleteSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(create_job_);
}

void CalculateReferencesTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
}

VariableSet CalculateReferencesTask::getReadSetFor(const std::string& dbcontent_name)
{
    VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_line_id_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_timestamp_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_ti_.name()).getFor(dbcontent_name));

    // flight level
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_mc_g_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_g_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_mc_v_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_v_.name()).getFor(dbcontent_name));

    // flight level trusted
    if (dbcontent_name == "CAT062")
    {
        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_baro_alt_.name()));
        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_fl_measured_.name()));

        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_vx_stddev_.name()));
        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_vy_stddev_.name()));
    }

    // m3a
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_m3a_g_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_g_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_m3a_v_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_v_.name()).getFor(dbcontent_name));

    // tn
    if (dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).getFor(dbcontent_name));

    // ground bit
    if (dbcontent_man.metaVariable(DBContent::meta_var_ground_bit_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_ground_bit_.name()).getFor(dbcontent_name));

    // speed & track angle

    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

    // adsb qis
    if (dbcontent_name == "CAT021")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_mops_version_));

        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_nacp_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_nacp_));

        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_nucp_nic_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_nucp_nic_));

        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_sil_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_sil_));

        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_nucv_nacv_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_nucv_nacv_));
    }

    if (dbcontent_name == "CAT010" || dbcontent_name == "CAT020"
            ||  dbcontent_name == "CAT062" || dbcontent_name == "RefTraj")
    {
        if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_confirmed_))
            read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_confirmed_));

        if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_coasting_))
            read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_coasting_));

        if (dbcontent_man.canGetVariable(dbcontent_name, DBContent::var_cat062_mono_sensor_))
            read_set.add(dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat062_mono_sensor_));

        if (dbcontent_man.canGetVariable(dbcontent_name, DBContent::var_cat062_type_lm_))
            read_set.add(dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat062_type_lm_));

        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_));
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_));
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_));
    }

    return read_set;
}

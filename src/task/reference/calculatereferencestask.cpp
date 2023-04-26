#include "calculatereferencestask.h"
#include "calculatereferencestaskdialog.h"
#include "calculatereferencesstatusdialog.h"
#include "calculatereferencesjob.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "jobmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "buffer.h"

#include <QApplication>
#include <QMessageBox>
#include <QThread>

using namespace std;
using namespace Utils;
using namespace dbContent;

CalculateReferencesTask::CalculateReferencesTask(const std::string& class_id,
                                                 const std::string& instance_id,
                                                 TaskManager& task_manager)
    : Task("CalculateReferencesTask", "Associate Target Reports", task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_references.json")
{
    tooltip_ =
            "Allows calculation of references based on System Tracker and ADS-B data.";

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

    assert(dialog_);
    return dialog_.get();
}

bool CalculateReferencesTask::canRun()
{
    if (!COMPASS::instance().dbOpened())
        return false;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (!dbcontent_man.hasData())
        return false;

    if (!dbcontent_man.dbContent("CAT021").hasData()
            && !dbcontent_man.dbContent("CAT062").hasData())
        return false;

    return true;
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
    status_dialog_->setStatus("Deleting old References");
    status_dialog_->show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    if (dbcontent_man.dbContent("RefTraj").existsInDB())
        dbcontent_man.dbContent("RefTraj").deleteDBContentData();

    while (dbcontent_man.dbContent("RefTraj").isDeleting())
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(10);
    }

    status_dialog_->setStatus("Loading Data");

    COMPASS::instance().viewManager().disableDataDistribution(true);

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &CalculateReferencesTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &CalculateReferencesTask::loadingDoneSlot);

    for (auto& dbo_it : dbcontent_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        if (dbo_it.first != "CAT021" && dbo_it.first != "CAT062") // dbo_it.first != "CAT021" &&
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, false);
    }

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

    status_dialog_->setStatus("Done");

    assert (create_job_);

    status_dialog_->setDone();

    if (!show_done_summary_)
        status_dialog_->close();

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
    status_dialog_->setStatus("Loading done, starting calculation");

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    loginf << "CalculateReferencesTask: loadingDoneSlot: data loading done";

    assert(!create_job_);

    create_job_ = std::make_shared<CalculateReferencesJob>(*this, cache_);

    connect(create_job_.get(), &CalculateReferencesJob::doneSignal, this,
            &CalculateReferencesTask::createDoneSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CalculateReferencesJob::obsoleteSignal, this,
            &CalculateReferencesTask::createObsoleteSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CalculateReferencesJob::statusSignal, this,
            &CalculateReferencesTask::calculationStatusSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(create_job_);
}

void CalculateReferencesTask::calculationStatusSlot(QString status)
{
    assert(status_dialog_);
    status_dialog_->setStatus(status.toStdString());
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
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_associations_.name()).getFor(dbcontent_name));
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
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_));
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_));
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_));
    }

    return read_set;
}

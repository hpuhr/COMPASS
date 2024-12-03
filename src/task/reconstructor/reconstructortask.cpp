#include "reconstructortask.h"

#include "compass.h"
#include "reconstructortaskdialog.h"
#include "datasourcemanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "buffer.h"
#include "simplereconstructor.h"
#include "simpleaccuracyestimator.h"
#include "timeconv.h"
#include "number.h"
#include "dbinterface.h"
#include "metavariable.h"
#include "util/async.h"
#include "evaluationmanager.h"
#include "viewpointgenerator.h"
#include "projectionmanager.h"
#include "projection.h"
#include "licensemanager.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "probimmreconstructor.h"
#include "complexaccuracyestimator.h"
#endif

#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>
#include <QPushButton>
#include <QLabel>

#include <malloc.h>

#include <future>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string ReconstructorTask::ScoringUMReconstructorName {"Scoring + UMKalman"};

#if USE_EXPERIMENTAL_SOURCE == true
const std::string ReconstructorTask::ProbImmReconstructorName {"Probabilistic + IMM"};
#endif

ReconstructorTask::ReconstructorTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task(task_manager),
    Configurable(class_id, instance_id, &task_manager, "task_reconstructor.json")
{
    tooltip_ = "Associate target reports and calculate reference trajectories based on all DB Content.";

    registerParameter("use_dstypes", &use_dstypes_, nlohmann::json::object());
    registerParameter("use_data_sources", &use_data_sources_, nlohmann::json::object());
    registerParameter("use_data_sources_lines", &use_data_sources_lines_, nlohmann::json::object());

    registerParameter("current_reconstructor_str", &current_reconstructor_str_, {});

    registerParameter("skip_reference_data_writing", &skip_reference_data_writing_, false);

    if (!current_reconstructor_str_.size()
        || (current_reconstructor_str_ != ScoringUMReconstructorName
#if USE_EXPERIMENTAL_SOURCE == true
            && current_reconstructor_str_ != ProbImmReconstructorName
#endif
            ))
        current_reconstructor_str_ = ScoringUMReconstructorName;

    registerParameter("debug", &debug_settings_.debug_, debug_settings_.debug_);
    registerParameter("debug_association", &debug_settings_.debug_association_,
                      debug_settings_.debug_association_);
    registerParameter("debug_outlier_detection", &debug_settings_.debug_outlier_detection_,
                      debug_settings_.debug_outlier_detection_);
    registerParameter("debug_accuracy_estimation", &debug_settings_.debug_accuracy_estimation_,
                      debug_settings_.debug_accuracy_estimation_);
    registerParameter("debug_bias_correction", &debug_settings_.debug_bias_correction_,
                      debug_settings_.debug_bias_correction_);
    registerParameter("debug_geo_altitude_correction", &debug_settings_.debug_geo_altitude_correction_,
                      debug_settings_.debug_geo_altitude_correction_);

    registerParameter("deep_debug_accuracy_estimation", &debug_settings_.deep_debug_accuracy_estimation_,
                      debug_settings_.deep_debug_accuracy_estimation_);
    registerParameter("deep_debug_accuracy_estimation_write_wp",
                      &debug_settings_.deep_debug_accuracy_estimation_write_wp_,
                      debug_settings_.deep_debug_accuracy_estimation_write_wp_);

    registerParameter("debug_reference_calculation", &debug_settings_.debug_reference_calculation_,
                      debug_settings_.debug_reference_calculation_);
    registerParameter("debug_kalman_chains", &debug_settings_.debug_kalman_chains_,
                      debug_settings_.debug_kalman_chains_);
    registerParameter("debug_write_reconstruction_viewpoints",
                      &debug_settings_.debug_write_reconstruction_viewpoints_,
                      debug_settings_.debug_write_reconstruction_viewpoints_);

    createSubConfigurables();
}

ReconstructorTask::~ReconstructorTask()
{
}

void ReconstructorTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "SimpleReconstructor")
    {
        assert(!simple_reconstructor_);

        std::unique_ptr<AccuracyEstimatorBase> acc_estimator;
        acc_estimator.reset(new SimpleAccuracyEstimator());

        simple_reconstructor_.reset(new SimpleReconstructor(class_id, instance_id, *this, std::move(acc_estimator)));
        assert(simple_reconstructor_);

        connect(simple_reconstructor_.get(), &ReconstructorBase::configChanged, this, &ReconstructorTask::configChanged);
    }
    else if (class_id == "ProbIMMReconstructor")
    {
#if USE_EXPERIMENTAL_SOURCE == true

        assert(!probimm_reconstructor_);

        std::unique_ptr<AccuracyEstimatorBase> acc_estimator;
        acc_estimator.reset(new ComplexAccuracyEstimator());

        probimm_reconstructor_.reset(new ProbIMMReconstructor(class_id, instance_id, *this, std::move(acc_estimator)));
        assert(probimm_reconstructor_);

        connect(probimm_reconstructor_.get(), &ReconstructorBase::configChanged, this, &ReconstructorTask::configChanged);
#endif
    }
    else
        throw std::runtime_error("ReconstructorTask: generateSubConfigurable: unknown class_id " + class_id);
}

void ReconstructorTask::initTask()
{
}

void ReconstructorTask::checkReconstructor()
{
    const auto& license_manager = COMPASS::instance().licenseManager();

    if (!license_manager.componentEnabled(license::License::ComponentProbIMMReconstructor) && currentReconstructorStr() == ProbImmReconstructorName)
        currentReconstructorStr(ScoringUMReconstructorName);
    else if (license_manager.componentEnabled(license::License::ComponentProbIMMReconstructor) && currentReconstructorStr() != ProbImmReconstructorName)
        currentReconstructorStr(ProbImmReconstructorName);
}

void ReconstructorTask::updateFeatures()
{
    checkReconstructor();
}

bool ReconstructorTask::canRun()
{
    assert (currentReconstructor());

    return COMPASS::instance().dbContentManager().hasData() && currentReconstructor()->hasNextTimeSlice();
}

void ReconstructorTask::updateProgressSlot(const QString& msg, bool add_slice_progress)
{
    logdbg << "ReconstructorTask: updateProgress: slice " << add_slice_progress
           << " dialog " << (progress_dialog_ != nullptr);

    if (!progress_dialog_)
        return;

    int ns   = currentReconstructor()->numSlices();
    int sidx = std::max(0, std::min((int)current_slice_idx_, ns - 1));

    if (done_)
        sidx = ns - 1;

    QString slice_p;
    if (ns >= 0)
        slice_p = " " + QString::number(sidx + 1) + "/" + QString::number(ns);

    QString pmsg = "<b>"+msg+"</b>";

    if (add_slice_progress)
        pmsg += "<b>"+slice_p+"</b>";

    double time_elapsed_s = Time::partialSeconds(boost::posix_time::microsec_clock::local_time() - run_start_time_);

    double progress = 0.0;

    if (ns >= 1)
    {
        if (done_)
            progress = 1.0;
        else
            progress = (double) sidx / (double) ns;
    }

    logdbg << "ReconstructorTask: updateProgress: slice " << add_slice_progress
           << " dialog " << (progress_dialog_ != nullptr) << " progress " << progress << " done " << done_ ;

    pmsg += ("<br><br><div align='left'>Elapsed: "
             + String::timeStringFromDouble(time_elapsed_s, false)+ " </div>").c_str();

    if (ns && sidx && !run_start_time_after_del_.is_not_a_date_time()) // do remaining time estimate if possible
    {
        double time_elapsed_after_del_s = Time::partialSeconds(boost::posix_time::microsec_clock::local_time()
                                                               - run_start_time_after_del_);

        double seconds_per_slice = time_elapsed_after_del_s / (float) (sidx + 1);

        int num_slices_remaining = ns - sidx; // not -1, since will display and async run
        double time_remaining_s = num_slices_remaining * seconds_per_slice;

        logdbg << "ReconstructorTask: updateProgress: current_slice_idx " << sidx
               << " ns " << ns << " num_slices_remaining " << num_slices_remaining
               << " time_remaining_s " << time_remaining_s;

        if (done_)
            pmsg += ("<div align='right'>Remaining: "
                     + String::timeStringFromDouble(0, false)+ " </div>").c_str();
        else
            pmsg += ("<div align='right'>Remaining: "
                     + String::timeStringFromDouble(time_remaining_s, false)+ " </div>").c_str();


        const auto& counts = currentReconstructor()->assocAounts();

        DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();
        DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

        if (counts.size())
        {
            // ds_id -> dbcont id -> cnt

            map<string, string> tmp_rows;

            for (auto& ds_it : counts)
            {
                for (auto& dbcont_it : ds_it.second)
                {
                    unsigned int assoc_cnt = dbcont_it.second.first;
                    unsigned int unassoc_cnt = dbcont_it.second.second;

                    std::string ds_name = ds_man.dbDataSource(ds_it.first).name();
                    std::string dbcont_name = dbcont_man.dbContentWithId(dbcont_it.first);

                    string row_str;

                    row_str += "<tr>";

                    row_str += ("<td>"+ds_name+"</td>").c_str();
                    row_str += ("<td>"+dbcont_name+"</td>").c_str();

                    row_str += ("<td align='right'>"+to_string(assoc_cnt)+"</td>").c_str();

                    std::string assoc_perc_str;

                    if (assoc_cnt + unassoc_cnt)
                        assoc_perc_str += String::percentToString(
                                              (100.0*assoc_cnt/(float)(assoc_cnt+unassoc_cnt)))+"%";
                    else
                        assoc_perc_str += String::percentToString(0)+"%";

                    row_str += ("<td align='right'>"+assoc_perc_str+"</td>").c_str();

                    row_str += "</tr>";

                    tmp_rows[ds_name] = row_str;
                }
            }

            pmsg += "<br><br><table width=\"100%\">"
                    "<tr> <td><b>Data Source</b></td> <td><b>DBContent</b></td>"
                    " <td align='right'><b>Associated</b></td> <td></td> </tr>";

            for (auto& row_it : tmp_rows)
                pmsg += row_it.second.c_str();

            pmsg += "</table>";
                //<table>
                //  <tr>
                //    <th>Company</th>
                //    <th>Contact</th>
                //    <th>Country</th>
                //  </tr>
                //  <tr>
                //    <td>Alfreds Futterkiste</td>
                //    <td>Maria Anders</td>
                //    <td>Germany</td>
                //  </tr>
                //</table>
        }

    }

    progress_dialog_->setLabelText("<html>"+pmsg+"</html>");

    if (done_)
        progress_dialog_->setValue(progress_dialog_->maximum());
    else
        progress_dialog_->setValue(progress_dialog_->maximum() * progress);
    
    Async::waitAndProcessEventsFor(50);
}

std::string ReconstructorTask::currentReconstructorStr() const
{
    return current_reconstructor_str_;
}

void ReconstructorTask::currentReconstructorStr(const std::string& value)
{

#if USE_EXPERIMENTAL_SOURCE == true
    assert (value == ReconstructorTask::ScoringUMReconstructorName
           || value == ReconstructorTask::ProbImmReconstructorName);
#else
    assert (value == ReconstructorTask::ScoringUMReconstructorName);
#endif

    current_reconstructor_str_ = value;
}

ReconstructorBase* ReconstructorTask::currentReconstructor() const
{
#if USE_EXPERIMENTAL_SOURCE == true
    assert (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName
           || current_reconstructor_str_ == ReconstructorTask::ProbImmReconstructorName);

    if (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName)
        return dynamic_cast<ReconstructorBase*> (simple_reconstructor_.get());
    else
        return dynamic_cast<ReconstructorBase*> (probimm_reconstructor_.get());
#else
    assert (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName);

    return dynamic_cast<ReconstructorBase*> (simple_reconstructor_.get());
#endif
}

SimpleReconstructor* ReconstructorTask::simpleReconstructor() const
{
    return simple_reconstructor_.get();
}

#if USE_EXPERIMENTAL_SOURCE == true

ProbIMMReconstructor* ReconstructorTask::probIMMReconstructor() const
{
    return probimm_reconstructor_.get();
}

#endif

std::set<unsigned int> ReconstructorTask::disabledDataSources() const
{
    std::set<unsigned int> disabled_ds;

    disabled_ds.insert(Number::dsIdFrom(currentReconstructor()->settings().ds_sac,
                                        currentReconstructor()->settings().ds_sic));

    if (current_reconstructor_str_ == ScoringUMReconstructorName)
    {
        for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
        {
            if (ds_it->name() == "CalcRef")
                disabled_ds.insert(ds_it->id());
        }
    }

    return disabled_ds;
}

void ReconstructorTask::run()
{
    assert(canRun());

    loading_slice_ = nullptr;
    loading_data_ = false;
    processing_slice_ = nullptr;
    processing_data_slice_ = false;
    writing_slice_ = nullptr;

    cancelled_ = false;
    done_ = false;

    current_slice_idx_ = 0;

    delcalcref_future_ = {};
    deltgts_future_ = {};
    delassocs_future_ = {};
    process_future_ = {};

    Projection& projection = ProjectionManager::instance().currentProjection();
    projection.clearCoordinateSystems();
    projection.addAllRadarCoordinateSystems();

    loginf << "ReconstructorTask: run: started";

    run_start_time_ = boost::posix_time::microsec_clock::local_time();
    run_start_time_after_del_ = {};

    QLabel* tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::RichText);

    progress_dialog_.reset(new QProgressDialog("Reconstructing...", "Cancel", 0, 100));
    progress_dialog_->setWindowTitle("Reconstructing References");
    progress_dialog_->setMinimumWidth(600);
    progress_dialog_->setLabel(tmp_label);
    progress_dialog_->setAutoClose(false);
    progress_dialog_->setAutoReset(false);
    progress_dialog_->setCancelButton(nullptr);
    progress_dialog_->setModal(true);

    progress_dialog_->show();

    if (cancelled_)
        return;

    updateProgressSlot("Deleting Previous References", false);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    COMPASS::instance().evaluationManager().clearLoadedDataAndResults(); // in case there are previous results

    delcalcref_future_ = std::async(std::launch::async, [&] {
        {
            try
            {
                deleteCalculatedReferences();

                if (cancelled_)
                    return;

                QMetaObject::invokeMethod(this, "deleteCalculatedReferencesDoneSlot", Qt::QueuedConnection);
            }
            catch (const std::exception& e)
            {
                loginf << "ReconstructorTask: run: delete calculated references threw exception '" << e.what() << "'";
                assert (false);
            }
        }});
}

void ReconstructorTask::deleteCalculatedReferencesDoneSlot()
{
    loginf << "ReconstructorTask: deleteCalculatedReferencesDoneSlot";

    updateProgressSlot("Deleting Previous Targets", false);

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    deltgts_future_ = std::async(std::launch::async, [&] {
        {
            try
            {
                cont_man.clearTargetsInfo();

                if (cancelled_)
                    return;

                QMetaObject::invokeMethod(this, "deleteTargetsDoneSlot", Qt::QueuedConnection);

            }
            catch (const std::exception& e)
            {
                loginf << "ReconstructorTask: run: delete targets threw exception '" << e.what() << "'";
                assert (false);
            }
        }});
}

void ReconstructorTask::deleteTargetsDoneSlot()
{
    loginf << "ReconstructorTask: deleteTargetsDoneSlot";

    updateProgressSlot("Deleting Previous Associations", false);

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    delassocs_future_ = std::async(std::launch::async, [&] {
        {
            try
            {
                for (auto& dbcont_it : cont_man)
                {
                    if (dbcont_it.second->existsInDB() && dbcont_it.second->hasVariable("UTN"))
                    {
                        QMetaObject::invokeMethod(this, "updateProgressSlot", Qt::QueuedConnection,
                                                  Q_ARG(const QString&, "Deleting Previous Associations"),
                                                  Q_ARG(bool, false));

                        COMPASS::instance().interface().clearAssociations(*dbcont_it.second);
                    }
                }

                if (cancelled_)
                    return;

                QMetaObject::invokeMethod(this, "deleteAssociationsDoneSlot", Qt::QueuedConnection);

            }
            catch (const std::exception& e)
            {
                loginf << "ReconstructorTask: run: delete associations threw exception '" << e.what() << "'";
                assert (false);
            }


        }});
}

void ReconstructorTask::deleteAssociationsDoneSlot()
{
    loginf << "ReconstructorTask: deleteAssociationsDoneSlot";

    // enable cancelling

    assert (progress_dialog_);
    progress_dialog_->setCancelButton(new QPushButton("Cancel"));

    connect(progress_dialog_.get(), &QProgressDialog::canceled,
            this, &ReconstructorTask::runCancelledSlot);

    updateProgressSlot("Initializing", false);

    run_start_time_after_del_ = boost::posix_time::microsec_clock::local_time();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    currentReconstructor()->reset();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &ReconstructorTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &ReconstructorTask::loadingDoneSlot);

    if (cancelled_)
        return;

    loadDataSlice();
}

void ReconstructorTask::loadDataSlice()
{
    assert (currentReconstructor());
    assert (currentReconstructor()->hasNextTimeSlice());
    assert (!loading_slice_);

    loginf << "ReconstructorTask: loadDataSlice";

    loading_slice_ = currentReconstructor()->getNextTimeSlice();

    assert (loading_slice_);
    loading_data_ = true;

    loginf << "ReconstructorTask: loadDataSlice: min " << Time::toString(loading_slice_->slice_begin_)
           << " max " << Time::toString(loading_slice_->next_slice_begin_)
           << " last " << loading_slice_->is_last_slice_;

    string timestamp_filter;

    timestamp_filter = "timestamp >= " + to_string(Time::toLong(loading_slice_->slice_begin_));

    if (loading_slice_->is_last_slice_)
        timestamp_filter += " AND timestamp <= " + to_string(Time::toLong(loading_slice_->next_slice_begin_));
    else
        timestamp_filter += " AND timestamp < " + to_string(Time::toLong(loading_slice_->next_slice_begin_));

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& dbcont_it : dbcontent_man)
    {
        logdbg << "ReconstructorTask: loadDataSlice: " << dbcont_it.first
               << " has data " << dbcont_it.second->hasData();

        if (!dbcont_it.second->hasData() || !dbcont_it.second->hasVariable("UTN"))
            continue;

        VariableSet read_set = currentReconstructor()->getReadSetFor(dbcont_it.first);

        dbcont_it.second->load(read_set, false, false, timestamp_filter);
    }
}

void ReconstructorTask::processDataSlice()
{
    loginf << "ReconstructorTask: processDataSlice";

    if (cancelled_)
        return;

    assert (loading_slice_);
    assert (!processing_slice_);

    processing_slice_ = std::move(loading_slice_);

    assert (!processing_data_slice_);

    if (!processing_slice_->data_.size())
    {
        loginf << "ReconstructorTask: processDataSlice: empty buffer at ("
               << Time::toString(loading_slice_->slice_begin_)<< ", no process";

        processing_slice_ = nullptr;

        return;
    }

    processing_data_slice_ = true;

    logdbg << "ReconstructorTask: processDataSlice: processing1 first slice "
           << !processing_slice_->first_slice_
           << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

    assert (processing_slice_);

    logdbg << "ReconstructorTask: processDataSlice: processing2 first slice "
           << !processing_slice_->first_slice_
           << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

    process_future_ = std::async(std::launch::async, [&] {
        try
        {
            logdbg << "ReconstructorTask: processDataSlice: async process";

            logdbg << "ReconstructorTask: processDataSlice: async processing first slice "
                   << !processing_slice_->first_slice_
                   << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

            assert (!currentReconstructor()->processing());
            currentReconstructor()->processSlice();

            // wait for previous writing done
            while (writing_slice_ && !cancelled_)
                QThread::msleep(1);

            logdbg << "ReconstructorTask: processDataSlice: done";

            QMetaObject::invokeMethod(this, "processingDoneSlot", Qt::QueuedConnection);
        }
        catch (std::exception& e)
        {
            loginf << "ReconstructorTask: run: processing slice threw exception '" << e.what() << "'";
            assert (false);
        }
    });
}


void ReconstructorTask::writeDataSlice()
{
    loginf << "ReconstructorTask: writeDataSlice";

    assert (writing_slice_);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : writing_slice_->assoc_data_)
    {
        logdbg << "ReconstructorTask: writeDataSlice: association dbcontent " << buf_it.first;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);

        dbcontent.updateData(
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(buf_it.first), buf_it.second);
    }

    if (writing_slice_->first_slice_)
        connect(&dbcontent_man, &DBContentManager::insertDoneSignal,
                this, &ReconstructorTask::writeDoneSlot);

    loginf << "ReconstructorTask: writeDataSlice: references dbcontent";
    dbcontent_man.insertData(writing_slice_->reftraj_data_);
}


void ReconstructorTask::loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    assert (loading_slice_);
}

void ReconstructorTask::loadingDoneSlot()
{
    loginf << "ReconstructorTask: loadingDoneSlot";

    assert (loading_data_);
    loading_data_ = false;

    assert (currentReconstructor());
    assert (loading_slice_);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (cancelled_)
    {
        loading_slice_ = nullptr;
        dbcontent_man.clearData(); // clear previous
        return;
    }

    bool last_slice = loading_slice_->is_last_slice_;

    loginf << "ReconstructorTask: loadingDoneSlot: is_last_slice " << last_slice
           << " current_slice_idx " << current_slice_idx_;

    loading_slice_->data_ = dbcontent_man.data();

    dbcontent_man.clearData(); // clear previous

    if (cancelled_)
        return;

    // check if not already processing
    while (currentReconstructor()->processing() || processing_data_slice_)
    {
        if (cancelled_)
            return;

        logdbg << "ReconstructorTask: loadingDoneSlot: waiting on reconst processing "
               << currentReconstructor()->processing()
               << " data slice proc " << processing_data_slice_;

        QCoreApplication::processEvents();
        QThread::msleep(1000);
    }

    if (cancelled_)
        return;

    loginf << "ReconstructorTask: loadingDoneSlot: calling process";

    updateProgressSlot("Processing Slice", true);
    ++current_slice_idx_;

    assert (!processing_data_slice_);

    loginf << "ReconstructorTask: loadingDoneSlot: processing first slice "
           << !loading_slice_->first_slice_
           << " remove ts " << Time::toString(loading_slice_->remove_before_time_);

    if (loading_slice_->data_.size())
    {
        processDataSlice();

        assert (!loading_slice_);
        assert (processing_data_slice_);
    }
    else // loading slice empty
    {
        loginf << "ReconstructorTask: loadingDoneSlot: empty buffer at ("
               << Time::toString(loading_slice_->slice_begin_)<< ", no process";

        loading_slice_ = nullptr;
        assert (!processing_data_slice_);
    }

    if (cancelled_)
        return;

    if (last_slice) // disconnect everything
    {
        disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
                   this, &ReconstructorTask::loadedDataSlot);
        disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
                   this, &ReconstructorTask::loadingDoneSlot);

        COMPASS::instance().viewManager().disableDataDistribution(false);
    }
    else // do next load
    {
        loginf << "ReconstructorTask: loadingDoneSlot: next slice";

        if (cancelled_)
            return;

        loadDataSlice();
    }
}

void ReconstructorTask::processingDoneSlot()
{
    loginf << "ReconstructorTask: processingDoneSlot";

    if (cancelled_)
    {
        processing_data_slice_ = false;
        processing_slice_ = nullptr;

        return;
    }

    // processing done
    assert(currentReconstructor()->currentSlice().processing_done_);
    assert (processing_data_slice_);
    assert (processing_slice_);
    assert (!writing_slice_);

    processing_data_slice_ = false;

    assert (!writing_slice_);
    writing_slice_ = std::move(processing_slice_);

    if (skip_reference_data_writing_)
    {
        writing_slice_ = nullptr;

        loginf << "ReconstructorTask: processingDoneSlot: skipping reference writing";
    }
    else
        writeDataSlice(); // starts the async jobs
}

void ReconstructorTask::writeDoneSlot()
{
    loginf << "ReconstructorTask: writeDoneSlot: last " << writing_slice_->is_last_slice_;

    assert (writing_slice_);
    writing_slice_->write_done_ = true;

    if (cancelled_)
    {
        writing_slice_ = nullptr;

        return;
    }

    if (writing_slice_->is_last_slice_)
    {
        DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

        disconnect(&dbcontent_man, &DBContentManager::insertDoneSignal,
                   this, &ReconstructorTask::writeDoneSlot);

        currentReconstructor()->saveTargets();

        COMPASS::instance().dataSourceManager().saveDBDataSources();
        emit COMPASS::instance().dataSourceManager().dataSourcesChangedSignal();
        COMPASS::instance().interface().saveProperties();

        done_ = true;

        assert (progress_dialog_);
        progress_dialog_->setCancelButtonText("OK");

        updateProgressSlot("Reference Calculation Done", true);

        if (debug_settings_.debug_)
        {
            //write some additional stuff before saving
            if (currentReconstructor())
                currentReconstructor()->createAdditionalAnnotations();
            
            saveDebugViewPoints();
        }

        currentReconstructor()->reset();

        double time_elapsed_s = Time::partialSeconds(
            boost::posix_time::microsec_clock::local_time() - run_start_time_);

        double time_elapsed_s_after_del = Time::partialSeconds(
            boost::posix_time::microsec_clock::local_time() - run_start_time_after_del_);

        loginf << "ReconstructorTask: writeDoneSlot: done after "
               << String::timeStringFromDouble(time_elapsed_s, false)
               << ", after deletion " << String::timeStringFromDouble(time_elapsed_s_after_del, false);

        COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");

        if (!allow_user_interactions_)
            progress_dialog_->close();
    }

    malloc_trim(0); // release unused memory

    writing_slice_ = nullptr;
}

void ReconstructorTask::runCancelledSlot()
{
    loginf << "ReconstructorTask: runCancelledSlot";

    assert (progress_dialog_);

    if (done_) // already done, cancel only to close
    {
        //close progress dialog
        progress_dialog_.reset();

        emit doneSignal();

        return;
    }

    progress_dialog_->setLabelText("Cancelling");
    progress_dialog_->setCancelButton(nullptr);
        //close progress dialog
    progress_dialog_.reset();

    currentReconstructor()->cancel();

    cancelled_ = true;

    QMessageBox* msg_box = new QMessageBox;

    msg_box->setWindowTitle("Cancelling Reconstruction");
    msg_box->setText("Please wait ...");
    msg_box->setStandardButtons(QMessageBox::NoButton);
    msg_box->setWindowModality(Qt::ApplicationModal);
    msg_box->show();

    Async::waitAndProcessEventsFor(50);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (dbcontent_man.loadInProgress())
        dbcontent_man.quitLoading();

    disconnect(&dbcontent_man, &DBContentManager::insertDoneSignal,
               this, &ReconstructorTask::writeDoneSlot);

    while (loading_data_ || dbcontent_man.loadInProgress()
           || processing_data_slice_ || currentReconstructor()->processing()
           || dbcontent_man.insertInProgress())
    {
        logdbg << "ReconstructorTask: runCancelledSlot: waiting, load "
               << (loading_data_ || dbcontent_man.loadInProgress())
               << " proc " << (processing_data_slice_ || currentReconstructor()->processing())
               << " insert " << dbcontent_man.insertInProgress();

        Async::waitAndProcessEventsFor(500);
    }

    loginf << "ReconstructorTask: runCancelledSlot: all done";

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &ReconstructorTask::loadedDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &ReconstructorTask::loadingDoneSlot);

    COMPASS::instance().viewManager().disableDataDistribution(false);

    if (debug_settings_.debug_)
        saveDebugViewPoints();

    currentReconstructor()->reset();

    loading_slice_ = nullptr;
    processing_slice_ = nullptr;
    writing_slice_ = nullptr;

    done_ = true;
    malloc_trim(0); // release unused memory

    msg_box->close();
    delete msg_box;

    emit doneSignal();

    loginf << "ReconstructorTask: runCancelledSlot: done";
}

bool ReconstructorTask::useDStype(const std::string& ds_type) const
{
    if (!use_dstypes_.contains(ds_type))
        return true;

    return use_dstypes_.at(ds_type);
}

void ReconstructorTask::useDSType(const std::string& ds_type, bool value)
{
    use_dstypes_[ds_type] = value;
}

bool ReconstructorTask::useDataSource(unsigned int ds_id) const
{
    string ds_id_str = to_string(ds_id);

    if (!use_data_sources_.contains(ds_id_str))
        return true;

    return use_data_sources_.at(ds_id_str);
}

void ReconstructorTask::useDataSource(unsigned int ds_id, bool value)
{
    use_data_sources_[to_string(ds_id)] = value;
}

bool ReconstructorTask::useDataSourceLine(unsigned int ds_id, unsigned int line_id) const
{
    string ds_id_str = to_string(ds_id);
    string line_id_str = to_string(line_id);

    if (!use_data_sources_lines_.contains(ds_id_str))
        return true;

    if (!use_data_sources_lines_.at(ds_id_str).contains(line_id_str))
        return true;

    return use_data_sources_lines_.at(ds_id_str).at(line_id_str);
}

void ReconstructorTask::useDataSourceLine(unsigned int ds_id, unsigned int line_id, bool value)
{
    use_data_sources_lines_[to_string(ds_id)][to_string(line_id)] = value;
}

std::set<unsigned int> ReconstructorTask::unusedDSIDs() const
{
    std::set<unsigned int> unused_ds = disabledDataSources();

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (unused_ds.count(ds_it->id()))
            continue;

        if (!useDStype(ds_it->dsType()) || !useDataSource(ds_it->id()))
            unused_ds.insert(ds_it->id());
    }

    return unused_ds;
}

std::map<unsigned int, std::set<unsigned int>> ReconstructorTask::unusedDSIDLines() const
{
    std::set<unsigned int> unused_ds = unusedDSIDs();

    std::map<unsigned int, std::set<unsigned int>> unused_lines;

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (unused_ds.count(ds_it->id()))
            continue;

        for (unsigned int line_id = 0; line_id < 4; ++line_id)
        {
            if (!useDataSourceLine(ds_it->id(), line_id))
                unused_lines[ds_it->id()].insert(line_id);
        }
    }

    return unused_lines;
}

ReconstructorBase::DataSlice& ReconstructorTask::processingSlice()
{
    assert (processing_slice_);
    return *processing_slice_;
}

const ReconstructorBase::DataSlice& ReconstructorTask::processingSlice() const
{
    assert (processing_slice_);
    return *processing_slice_;
}

ViewPointGenVP* ReconstructorTask::getDebugViewpoint(const std::string& name, const std::string& type, bool* created) const
{
    auto key_str = std::pair<std::string,std::string>(name,type);

    if (created)
        *created = false;

    if (!debug_viewpoints_.count(key_str))
    {
        std::unique_ptr<ViewPointGenVP> vp(new ViewPointGenVP(name, 0, type));
        debug_viewpoints_[ key_str ] = std::move(vp);

        if (created)
            *created = true;
    }

    return debug_viewpoints_.at(key_str).get();
}

ViewPointGenVP* ReconstructorTask::getDebugViewpointNoData(const std::string& name, const std::string& type)
{
    auto vp = getDebugViewpoint(name, type);
    vp->noDataLoaded(true);

    return vp;
}

ViewPointGenVP* ReconstructorTask::getDebugViewpointForUTN(unsigned long utn, const std::string& name_prefix) const
{
    bool created;
    string name;

    if (name_prefix.size())
        name = name_prefix + " UTN " + std::to_string(utn);
    else
        name = "UTN " + std::to_string(utn);

    auto vp = getDebugViewpoint(name, "UTN", &created);

    if (created)
    {
        ViewPointGenFilterUTN* filter_utn = new ViewPointGenFilterUTN(utn);

        vp->filters().addFilter(std::unique_ptr<ViewPointGenFilterUTN>(filter_utn));
    }

    return vp;
}

ViewPointGenAnnotation* ReconstructorTask::getDebugAnnotationForUTNSlice(unsigned long utn, size_t slice_idx) const
{
    auto vp = getDebugViewpointForUTN(utn);

    return vp->annotations().getOrCreateAnnotation("Slice " + std::to_string(slice_idx));
}

void ReconstructorTask::saveDebugViewPoints()
{
    loginf << "ReconstructorTask: saveDebugViewPoints";

    COMPASS::instance().viewManager().clearViewPoints();

    std::vector <nlohmann::json> view_points;

    for (auto& vp_it : debug_viewpoints_)
    {
        nlohmann::json j;
        vp_it.second->toJSON(j);

        view_points.emplace_back(j);
    }

    COMPASS::instance().viewManager().addViewPoints(view_points);

    debug_viewpoints_.clear();
}

bool ReconstructorTask::skipReferenceDataWriting() const
{
    return skip_reference_data_writing_;
}

void ReconstructorTask::skipReferenceDataWriting(bool value)
{
    skip_reference_data_writing_ = value;
}

void ReconstructorTask::checkSubConfigurables()
{
    if (!simple_reconstructor_)
    {
        generateSubConfigurable("SimpleReconstructor", "SimpleReconstructor0");
        assert (simple_reconstructor_);
    }

#if USE_EXPERIMENTAL_SOURCE == true
    if (!probimm_reconstructor_)
    {
        generateSubConfigurable("ProbIMMReconstructor", "ProbIMMReconstructor0");
        assert (probimm_reconstructor_);
    }
#endif
}

void ReconstructorTask::deleteCalculatedReferences() // called in async
{
    loginf << "ReconstructorTask: deleteCalculatedReferences: delete_all_calc_reftraj "
           << currentReconstructor()->settings().delete_all_calc_reftraj;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    loginf << "ReconstructorTask: deleteCalculatedReferences: deleting";

    if (currentReconstructor()->settings().delete_all_calc_reftraj)
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->settings().ds_sac, currentReconstructor()->settings().ds_sic);
    else
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->settings().ds_sac, currentReconstructor()->settings().ds_sic,
            currentReconstructor()->settings().ds_line);

    loginf << "ReconstructorTask: deleteCalculatedReferences: waiting on delete";

    while (dbcontent_man.dbContent("RefTraj").isDeleting())
    {
        QMetaObject::invokeMethod(this, "updateProgressSlot", Qt::QueuedConnection,
                                  Q_ARG(const QString&, "Deleting Previous References"),
                                  Q_ARG(bool, false));
        QThread::msleep(1000);
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id = Number::dsIdFrom(currentReconstructor()->settings().ds_sac,
                                          currentReconstructor()->settings().ds_sic);

    // clear counts
    if (ds_man.hasDBDataSource(ds_id))
    {
        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (currentReconstructor()->settings().delete_all_calc_reftraj)
            ds.clearNumInserted("RefTraj");
        else
            ds.clearNumInserted("RefTraj", currentReconstructor()->settings().ds_line);
    }

    // emit done in run

    loginf << "ReconstructorTask: deleteCalculatedReferences: done";
}

void ReconstructorTask::showDialog()
{
    ReconstructorTaskDialog dlg(*this);
    if (dlg.exec() == QDialog::Rejected)
        return;

    assert(canRun());
    run();
}

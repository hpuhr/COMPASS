#include "probimmreconstructor.h"
#include "probimmreconstructorwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "targetreportaccessor.h"
#include "datasourcemanager.h"
#include "dbinterface.h"
#include "number.h"

#include "timeconv.h"

using namespace std;
using namespace Utils;

ProbIMMReconstructor::ProbIMMReconstructor(const std::string& class_id, 
                                           const std::string& instance_id,
                                           ReconstructorTask& task, 
                                           std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator)
    : ReconstructorBase(class_id, instance_id, task, std::move(acc_estimator), 1)
      , associatior_   (*this)
      , ref_calculator_(*this)
{
    registerParameter("max_time_diff", &settings_.max_time_diff_, settings_.max_time_diff_);
    registerParameter("track_max_time_diff", &settings_.track_max_time_diff_, settings_.track_max_time_diff_);

    registerParameter("max_altitude_diff", &settings_.max_altitude_diff_, settings_.max_altitude_diff_);

    registerParameter("max_mahalanobis_sec_verified_dist", &settings_.max_mahalanobis_sec_verified_dist_,
                      settings_.max_mahalanobis_sec_verified_dist_);
    registerParameter("max_mahalanobis_sec_unknown_dist", &settings_.max_mahalanobis_sec_unknown_dist_,
                      settings_.max_mahalanobis_sec_unknown_dist_);

    registerParameter("max_tgt_est_std_dev", &settings_.max_tgt_est_std_dev_, settings_.max_tgt_est_std_dev_);

    registerParameter("max_sum_est_std_dev", &settings_.max_sum_est_std_dev_, settings_.max_sum_est_std_dev_);
    registerParameter("min_sum_est_std_dev", &settings_.min_sum_est_std_dev_, settings_.min_sum_est_std_dev_);
}

ProbIMMReconstructor::~ProbIMMReconstructor() {}

dbContent::VariableSet ProbIMMReconstructor::getReadSetFor(const std::string& dbcontent_name) const
{
    dbContent::VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

            // ds id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));

            // line id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

            // timestamp
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));

            // aircraft address
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acad_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_));

            // aircraft id
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acid_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_));

            // track num
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_num_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));

            // track end
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_end_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));

            // mode 3a
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));

            // mode c
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    if (dbcontent_name == "CAT062")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
    }

            // latitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));

            // longitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

            // assoc
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_utn_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

            // rec num, must be last for update process
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

            // adsb mops
    if (dbcontent_name == "CAT021")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
    }

    read_set.add(dbContent::TargetReportAccessor::getReadSetFor(dbcontent_name));

    return read_set;
}

void ProbIMMReconstructor::reset()
{
    loginf << "ProbIMMReconstructor: reset";

    associatior_.reset();
    ref_calculator_.reset();

    ReconstructorBase::reset();
}

ProbIMMReconstructorSettings& ProbIMMReconstructor::settings()
{
    return settings_;
}


ProbIMMReconstructorWidget* ProbIMMReconstructor::widget() // ownage by caller
{
    ProbIMMReconstructorWidget* widget = new ProbIMMReconstructorWidget(*this);

    connect (this, &ProbIMMReconstructor::updateWidgetsSignal,
            widget, &ProbIMMReconstructorWidget::updateSlot);

    return widget;
}

void ProbIMMReconstructor::updateWidgets()
{
    emit updateWidgetsSignal();
}

void ProbIMMReconstructor::processSlice_impl()
{
    loginf << "ProbIMMReconstructor: processSlice_impl: current_slice_begin "
           << Time::toString(currentSlice().slice_begin_)
           << " end " << Time::toString(currentSlice().slice_begin_ + baseSettings().sliceDuration())
           << " is last " << currentSlice().is_last_slice_;

            // remove_before_time_, new data >= current_slice_begin_

    clearOldTargetReports();

    ref_calculator_.settings() = settings_.ref_calc_settings_;
    ref_calculator_.prepareForNextSlice();

    createTargetReports();

    assert (acc_estimator_);
    associatior_.associateNewData();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations = createAssociations();
    // only for ts < write_before_time, also updates target counts
    currentSlice().assoc_data_ = createAssociationBuffers(associations);

    ref_calculator_.computeReferences();

    acc_estimator_->estimateAccuracies();

    currentSlice().reftraj_data_ = createReferenceBuffers(); // only for ts < write_before_time

    return;
}




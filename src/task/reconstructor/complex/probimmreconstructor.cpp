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

ProbIMMReconstructor::ProbIMMReconstructor(
    const std::string& class_id, const std::string& instance_id,
    ReconstructorTask& task, std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator)
    : ReconstructorBase(class_id, instance_id, task, std::move(acc_estimator))
      , associatior_   (*this)
      , ref_calculator_(*this)
{
    ds_line_ = 1;
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

bool ProbIMMReconstructor::processSlice_impl()
{
    loginf << "ProbIMMReconstructor: processSlice_impl: current_slice_begin " << Time::toString(current_slice_begin_)
           << " end " << Time::toString(current_slice_begin_ + slice_duration_)
           << " has next " << hasNextTimeSlice();

            // remove_before_time_, new data >= current_slice_begin_

    bool is_last_slice = !hasNextTimeSlice();

    clearOldTargetReports();

    ref_calculator_.settings() = settings_.ref_calc_settings_;
    ref_calculator_.prepareForNextSlice();

    createTargetReports();

    assert (acc_estimator_);
    associatior_.associateNewData();

    auto associations = createAssociations(); // only for ts < write_before_time, also updates target counts
    saveAssociations(associations);

    ref_calculator_.computeReferences();

    acc_estimator_->estimateAccuracies();

    saveReferences(); // only for ts < write_before_time

    if (is_last_slice)
        saveTargets();

    return true;
}




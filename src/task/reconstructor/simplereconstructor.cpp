#include "simplereconstructor.h"
#include "simplereconstructorwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/variable.h"


#include "timeconv.h"

using namespace std;
using namespace Utils;

SimpleReconstructor::SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                                         ReconstructorTask& task, std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator)
    : ReconstructorBase(class_id, instance_id, task, std::move(acc_estimator))
    , associatior_   (*this)
    , ref_calculator_(*this)
{
    //association
    {
        // common
        registerParameter("associate_non_mode_s", &settings_.associate_non_mode_s_, true);

        // tracker stuff
        registerParameter("max_time_diff_tracker", &settings_.max_time_diff_tracker_, 15.0);

        registerParameter("max_distance_quit_tracker", &settings_.max_distance_quit_tracker_, 10*NM2M); // kb 5nm
        registerParameter("max_distance_dubious_tracker", &settings_.max_distance_dubious_tracker_, 3*NM2M);
        //kb 2.5? 2.5 lowest
        registerParameter("max_positions_dubious_tracker", &settings_.max_positions_dubious_tracker_, 5u);

        registerParameter("max_distance_acceptable_tracker", &settings_.max_distance_acceptable_tracker_, NM2M/2.0);
        registerParameter("max_altitude_diff_tracker", &settings_.max_altitude_diff_tracker_, 300.0);

        registerParameter("min_updates_tracker", &settings_.min_updates_tracker_, 2u); // kb 3!!!
        registerParameter("prob_min_time_overlap_tracker", &settings_.prob_min_time_overlap_tracker_, 0.5); //kb 0.7

        registerParameter("cont_max_time_diff_tracker", &settings_.cont_max_time_diff_tracker_, 30.0);
        registerParameter("cont_max_distance_acceptable_tracker", &settings_.cont_max_distance_acceptable_tracker_, 1852.0);

                // sensor
        registerParameter("max_time_diff_sensor", &settings_.max_time_diff_sensor_, 15.0);
        registerParameter("max_distance_acceptable_sensor", &settings_.max_distance_acceptable_sensor_, 2*NM2M);
        registerParameter("max_altitude_diff_sensor", &settings_.max_altitude_diff_sensor_, 300.0);

        // target id? kb: nope
        // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
    }

    //reference computation
    {
        registerParameter("ref_rec_type", (int*)&settings_.ref_calc_settings_.rec_type, (int)SimpleReferenceCalculator::Settings().rec_type);

        registerParameter("ref_Q_std", &settings_.ref_calc_settings_.Q_std, SimpleReferenceCalculator::Settings().Q_std);

        registerParameter("ref_min_chain_size", &settings_.ref_calc_settings_.min_chain_size   , SimpleReferenceCalculator::Settings().min_chain_size);
        registerParameter("ref_min_dt"        , &settings_.ref_calc_settings_.min_dt   , SimpleReferenceCalculator::Settings().min_dt);
        registerParameter("ref_max_dt"        , &settings_.ref_calc_settings_.max_dt   , SimpleReferenceCalculator::Settings().max_dt);
        registerParameter("ref_max_distance"  , &settings_.ref_calc_settings_.max_distance   , SimpleReferenceCalculator::Settings().max_distance);

        registerParameter("ref_smooth_rts", &settings_.ref_calc_settings_.smooth_rts, SimpleReferenceCalculator::Settings().smooth_rts);

        registerParameter("ref_resample_result", &settings_.ref_calc_settings_.resample_result, SimpleReferenceCalculator::Settings().resample_result);
        registerParameter("ref_resample_Q_std" , &settings_.ref_calc_settings_.resample_Q_std , SimpleReferenceCalculator::Settings().resample_Q_std);
        registerParameter("ref_resample_dt"    , &settings_.ref_calc_settings_.resample_dt    , SimpleReferenceCalculator::Settings().resample_dt);

        registerParameter("ref_max_proj_distance_cart", &settings_.ref_calc_settings_.max_proj_distance_cart, SimpleReferenceCalculator::Settings().max_proj_distance_cart);

        registerParameter("ref_resample_systracks"       , &settings_.ref_calc_settings_.resample_systracks       , SimpleReferenceCalculator::Settings().resample_systracks);
        registerParameter("ref_resample_systracks_dt"    , &settings_.ref_calc_settings_.resample_systracks_dt    , SimpleReferenceCalculator::Settings().resample_systracks_dt);
        registerParameter("ref_resample_systracks_max_dt", &settings_.ref_calc_settings_.resample_systracks_max_dt, SimpleReferenceCalculator::Settings().resample_systracks_max_dt);
    }
}

SimpleReconstructor::~SimpleReconstructor() {}

dbContent::VariableSet SimpleReconstructor::getReadSetFor(const std::string& dbcontent_name) const
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

    return read_set;
}

void SimpleReconstructor::reset()
{
    loginf << "SimpleReconstructor: reset";

    target_reports_.clear();
    tr_timestamps_.clear();
    tr_ds_.clear();

    ReconstructorBase::reset();
}

SimpleReconstructorSettings& SimpleReconstructor::settings()
{
    return settings_;
}

SimpleReconstructorWidget* SimpleReconstructor::widget() // ownage by caller
{
    SimpleReconstructorWidget* widget = new SimpleReconstructorWidget(*this);

    connect (this, &SimpleReconstructor::updateWidgetsSignal,
            widget, &SimpleReconstructorWidget::updateSlot);

    return widget;
}

void SimpleReconstructor::updateWidgets()
{
    emit updateWidgetsSignal();
}

bool SimpleReconstructor::processSlice_impl()
{
    loginf << "SimpleReconstructor: processSlice_impl: current_slice_begin " << Time::toString(current_slice_begin_)
           << " end " << Time::toString(current_slice_begin_ + slice_duration_)
           << " has next " << hasNextTimeSlice();

            // remove_before_time_, new data >= current_slice_begin_

    bool is_last_slice = !hasNextTimeSlice();

    clearOldTargetReports();

    ref_calculator_.settings() = settings_.ref_calc_settings_;
    ref_calculator_.prepareForNextSlice();

    createTargetReports();

    associatior_.associateNewData();

    auto associations = createAssociations(); // only for ts < write_before_time, also updates target counts
    saveAssociations(associations);

    ref_calculator_.computeReferences();

    saveReferences(); // only for ts < write_before_time

    if (is_last_slice)
        saveTargets();

    return true;
}



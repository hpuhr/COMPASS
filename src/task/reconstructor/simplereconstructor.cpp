#include "simplereconstructor.h"
#include "simplereconstructorwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/target/targetreportaccessor.h"


#include "timeconv.h"

using namespace std;
using namespace Utils;

SimpleReconstructor::SimpleReconstructor(const std::string& class_id, 
                                         const std::string& instance_id,
                                         ReconstructorTask& task, 
                                         std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator)
    : ReconstructorBase(class_id, instance_id, task, std::move(acc_estimator), 0)
    , associatior_   (*this)
    , ref_calculator_(*this)
{
    //association
    {
        // common
        //registerParameter("associate_non_mode_s", &settings_.associate_non_mode_s_, true);

        // tracker stuff
        //registerParameter("max_time_diff", &settings_.max_time_diff_, 15.0);

        registerParameter("max_distance_quit", &settings_.max_distance_quit_, 10*NM2M); // kb 5nm
        registerParameter("max_distance_dubious", &settings_.max_distance_dubious_, 3*NM2M);
        //kb 2.5? 2.5 lowest
        //registerParameter("max_positions_dubious", &settings_.max_positions_dubious_, 5u);

        registerParameter("max_distance_acceptable", &settings_.max_distance_acceptable_, 1*NM2M);
        //registerParameter("max_altitude_diff", &settings_.max_altitude_diff_, 300.0);
        //registerParameter("track_max_time_diff", &settings_.track_max_time_diff_, 300.0);

        //registerParameter("min_updates", &settings_.min_updates_, 3u); // kb 3!!!
        //registerParameter("prob_min_time_overlap", &settings_.prob_min_time_overlap_, 0.5); //kb 0.7

        //registerParameter("cont_max_time_diff_tracker", &settings_.cont_max_time_diff_, 30.0);
        //registerParameter("cont_max_distance_acceptable_tracker", &settings_.cont_max_distance_acceptable_, 1852.0);

                // sensor
        //registerParameter("max_time_diff_sensor", &settings_.max_time_diff_sensor_, 15.0);
        //registerParameter("max_distance_acceptable_sensor", &settings_.max_distance_acceptable_sensor_, 2*NM2M);
        //registerParameter("max_altitude_diff_sensor", &settings_.max_altitude_diff_sensor_, 300.0);

        // target id? kb: nope
        // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
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

    read_set.add(dbContent::TargetReportAccessor::getReadSetFor(dbcontent_name));

    return read_set;
}

void SimpleReconstructor::reset()
{
    loginf << "SimpleReconstructor: reset";

//    target_reports_.clear(); // done in base
//    tr_timestamps_.clear();
//    tr_ds_.clear();

    associatior_.reset();
    ref_calculator_.reset();

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

void SimpleReconstructor::processSlice_impl()
{
    loginf << "SimpleReconstructor: processSlice_impl: current_slice_begin "
           << Time::toString(currentSlice().slice_begin_)
           << " end " << Time::toString(currentSlice().slice_begin_ + settings().sliceDuration())
           << " is last " << currentSlice().is_last_slice_;

            // remove_before_time_, new data >= current_slice_begin_

    clearOldTargetReports();

    ref_calculator_.settings() = referenceCalculatorSettings();
    ref_calculator_.prepareForNextSlice();

    createTargetReports();

    associatior_.associateNewData();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations = createAssociations();
    // only for ts < write_before_time, also updates target counts
    currentSlice().assoc_data_ = createAssociationBuffers(associations);

    ref_calculator_.computeReferences();

    currentSlice().reftraj_data_ = createReferenceBuffers(); // only for ts < write_before_time

    return;
}



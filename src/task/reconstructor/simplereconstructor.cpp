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
    : ReconstructorBase(class_id, instance_id, task, std::move(acc_estimator), settings_, 0)
    , associatior_   (*this)
    , ref_calculator_(*this)
{
    registerParameter("max_distance_quit", &settings_.max_distance_quit_, 5*NM2M); // kb 5nm
    registerParameter("max_distance_dubious", &settings_.max_distance_dubious_, 2*NM2M);
    registerParameter("max_distance_acceptable", &settings_.max_distance_acceptable_, 1*NM2M);
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

    if (cancelled_)
        return;

    acc_estimator_->prepareForNewSlice(); // does nothing here

    if (cancelled_)
        return;

    clearOldTargetReports();

    if (cancelled_)
        return;

    ref_calculator_.settings() = referenceCalculatorSettings();
    ref_calculator_.prepareForNextSlice();

    if (cancelled_)
        return;

    createTargetReports();

    if (cancelled_)
        return;

    associatior_.associateNewData();

    if (cancelled_)
        return;

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations = createAssociations();
    // only for ts < write_before_time, also updates target counts
    currentSlice().assoc_data_ = createAssociationBuffers(associations);

    if (cancelled_)
        return;

    ref_calculator_.computeReferences();

    if (cancelled_)
        return;

    acc_estimator_->postProccessNewSlice(); // does nothing here

    if (cancelled_)
        return;

    currentSlice().reftraj_data_ = createReferenceBuffers(); // only for ts < write_before_time

    return;
}

void SimpleReconstructor::createAdditionalAnnotations()
{
    ref_calculator_.createAnnotations();
}

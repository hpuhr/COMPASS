/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "simplereferencecalculator.h"
#include "spline_interpolator.h"
#include "reconstructorbase.h"
#include "reconstructortask.h"
#include "targetreportdefs.h"

#include "kalman_estimator.h"
#include "kalman_interface.h"
#include "kalman_projection.h"

#include "viewpointgenerator.h"
#include "histograminitializer.h"
#include "scatterseries.h"
#include "grid2d.h"
#include "grid2dlayer.h"

#include "util/timeconv.h"
#include "util/number.h"
#include "stringconv.h"
#include "logger.h"

#include "projector.h"

#include <ogr_spatialref.h>

#include "tbbhack.h"

using namespace Utils;

const QColor SimpleReferenceCalculator::ColorMeasurements    = QColor(102, 178, 255);
const QColor SimpleReferenceCalculator::ColorKalman          = QColor(255, 102, 178);
const QColor SimpleReferenceCalculator::ColorKalmanSmoothed  = QColor(255, 178, 102);
const QColor SimpleReferenceCalculator::ColorKalmanResampled = QColor(200, 200, 200);

const float SimpleReferenceCalculator::PointSizeOSG             = 10.0f;
const float SimpleReferenceCalculator::PointSizeMeasurements    = 6.0f;
const float SimpleReferenceCalculator::PointSizeKalman          = 4.0f;
const float SimpleReferenceCalculator::PointSizeKalmanSmoothed  = 2.0f;
const float SimpleReferenceCalculator::PointSizeKalmanResampled = 2.0f;

const float SimpleReferenceCalculator::LineWidthBase = 1.0f;

/**
 */
void SimpleReferenceCalculator::TargetReferences::reset()
{
    measurements.resize(0);
    references.resize(0);
    init_update.reset();
    start_index.reset();

    resetCounts();
}

/**
 */
void SimpleReferenceCalculator::TargetReferences::resetCounts()
{
    num_updates                 = 0;
    num_updates_valid           = 0;
    num_updates_failed          = 0;
    num_updates_failed_numeric  = 0;
    num_updates_failed_badstate = 0;
    num_updates_failed_other    = 0;
    num_updates_skipped         = 0;
    num_smooth_steps_failed     = 0;
    num_smoothing_failed        = 0;
    num_interp_steps_failed     = 0;
}

/**
 */
SimpleReferenceCalculator::SimpleReferenceCalculator(ReconstructorBase& reconstructor)
    :   reconstructor_(reconstructor)
{
}

/**
 */
SimpleReferenceCalculator::~SimpleReferenceCalculator() = default;

/**
 */
void SimpleReferenceCalculator::prepareForNextSlice()
{
    if (!reconstructor_.currentSlice().first_slice_)
    {
        prepareForCurrentSlice(); // also does resetDataStructs()

        ++slice_idx_;
    }
    else
    {
        slice_idx_ = 0;

        //reset data structs
        resetDataStructs();
    }

    is_last_slice_ = reconstructor_.currentSlice().is_last_slice_;
}

/**
 */
void SimpleReferenceCalculator::prepareForCurrentSlice()
{
    auto ThresRemove = reconstructor_.currentSlice().remove_before_time_;
    auto ThresJoin   = getJoinThreshold();

            //remove previous updates which are no longer needed (either too old or above the join threshold)
    for (auto& ref : references_)
    {
        auto it = std::remove_if(ref.second.updates.begin(),
                                 ref.second.updates.end(),
                                 [ & ] (const kalman::KalmanUpdate& update) { return update.t <  ThresRemove ||
                                                                                     update.t >= ThresJoin; });
        ref.second.updates.erase(it, ref.second.updates.end());

        auto it_s = std::remove_if(ref.second.updates_smooth.begin(),
                                   ref.second.updates_smooth.end(),
                                   [ & ] (const kalman::KalmanUpdate& update) { return update.t <  ThresRemove ||
                                                                                       update.t >= ThresJoin; });
        ref.second.updates_smooth.erase(it_s, ref.second.updates_smooth.end());
    }

    //reset data structs
    resetDataStructs();
}

/**
 */
void SimpleReferenceCalculator::reset()
{
    loginf << "SimpleReferenceCalculator: reset";

    references_.clear();
    interp_options_.clear();

    slice_idx_ = 0;
}

/**
 */
void SimpleReferenceCalculator::resetDataStructs()
{
    for (auto& ref : references_)
        ref.second.reset();

    updateInterpOptions();
}

/**
 */
void SimpleReferenceCalculator::updateInterpOptions()
{
    if (settings_.resample_systracks)
    {
        interp_options_[ 62 ].sample_dt = settings_.resample_systracks_dt;
        interp_options_[ 62 ].max_dt    = settings_.resample_systracks_max_dt;
    }
    else if (interp_options_.count(62))
    {
        interp_options_.erase(62);
    }
}

/**
 */
bool SimpleReferenceCalculator::computeReferences()
{
    loginf << "SimpleReferenceCalculator: computeReferences";

    resetDataStructs();

    //skip slice? (debug)
    if (settings_.max_slice_index >= 0 && slice_idx_ > settings_.max_slice_index)
        return true;

    generateMeasurements();
    reconstructMeasurements();
    updateReferences();

    return true;
}

/**
 */
void SimpleReferenceCalculator::generateMeasurements()
{
    for (const auto& target : reconstructor_.targets_container_.targets_)
        generateTargetMeasurements(target.second);
}

/**
 */
void SimpleReferenceCalculator::generateTargetMeasurements(const dbContent::ReconstructorTarget& target)
{
    for (const auto& dbcontent_targets : target.tr_ds_timestamps_)
        for (const auto& ds_targets : dbcontent_targets.second)
            for (const auto& line_targets : ds_targets.second)
                generateLineMeasurements(target, 
                                         dbcontent_targets.first, 
                                         ds_targets.first, 
                                         line_targets.first, 
                                         line_targets.second);
}

/**
 */
void SimpleReferenceCalculator::generateLineMeasurements(const dbContent::ReconstructorTarget& target,
                                                         unsigned int dbcontent_id,
                                                         unsigned int sensor_id,
                                                         unsigned int line_id,
                                                         const TargetReports& target_reports)
{
    //compatibility mode: only use systracks and adsb (debug)
    if (settings_.compat_mode && dbcontent_id != 21 && dbcontent_id != 62)
        return;

    std::vector<reconstruction::Measurement> line_measurements;

    assert (reconstructor_.acc_estimator_);

    for (const auto& elem : target_reports)
    {
        const auto& tr_info = reconstructor_.target_reports_.at(elem.second);

        if (tr_info.do_not_use_position_)
            continue;

        reconstruction::Measurement mm;
        reconstructor_.createMeasurement(mm, tr_info, &target);

        // if (tr_info.track_number_.value() == 69 || target.utn_ == 69)
        // {
        //     loginf << "POS (" << mm.lat << "," << mm.lon << ") " << "(" << (mm.vx.has_value() ? mm.vx.value() : 666) << "," << (mm.vy.has_value() ? mm.vy.value() : 666) << ")";
        //     loginf << "ACC (" << mm.x_stddev.value() << "," << mm.y_stddev.value() << "," << mm.xy_cov.value() << ") " << "(" << mm.vx_stddev.value() << "," << mm.vy_stddev.value() << ")";
        // }

        line_measurements.push_back(mm);
    }

    addMeasurements(target.utn_, dbcontent_id, line_measurements);
}

/**
 */
void SimpleReferenceCalculator::addMeasurements(unsigned int utn,
                                                unsigned int dbcontent_id,
                                                Measurements& measurements)
{
    //preprocess
    preprocessMeasurements(dbcontent_id, measurements);

    //add to utn measurements
    auto& utn_ref = references_[ utn ];
    utn_ref.utn = utn;

    if (!measurements.empty()) // TODO UGA
        utn_ref.measurements.insert(utn_ref.measurements.end(), measurements.begin(), measurements.end());
}

/**
 */
void SimpleReferenceCalculator::preprocessMeasurements(unsigned int dbcontent_id, 
                                                       Measurements& measurements)
{
    //interpolate if options are set for dbcontent
    if (interp_options_.count(dbcontent_id))
        interpolateMeasurements(measurements, interp_options_.at(dbcontent_id));
}

namespace
{
bool mmSortPred(const reconstruction::Measurement& mm0,
                const reconstruction::Measurement& mm1)
{
    //sort by time first
    if (mm0.t != mm1.t)
        return mm0.t < mm1.t;

    //otherwise sort by source at least
    return mm0.source_id < mm1.source_id;
}
}

/**
 */
void SimpleReferenceCalculator::interpolateMeasurements(Measurements& measurements, 
                                                        const reconstruction::InterpOptions& options) const
{
    //sort measurements by timestamp
    std::sort(measurements.begin(), measurements.end(), mmSortPred);

    reconstruction::SplineInterpolator interp;
    interp.config().check_fishy_segments = true;
    interp.config().interpolate_cart     = false;
    interp.config().sample_dt            = options.sample_dt;
    interp.config().max_dt               = options.max_dt;

    //interpolate using desired timestep
    auto mms_interp = interp.interpolate(measurements);
    
    size_t ni = mms_interp.size();
    if (ni < 1)
        return;

    //assign new measurements
    measurements.resize(ni);

    for (size_t i = 0; i < ni; ++i)
        measurements[ i ] = mms_interp[ i ];
}

/**
 */
void SimpleReferenceCalculator::reconstructMeasurements()
{
    if (references_.empty())
        return;

    std::vector<TargetReferences*> refs;

    //collect jobs
    for (auto& ref : references_)
        refs.push_back(&ref.second);

    unsigned int num_targets = refs.size();

    loginf << "SimpleReferenceCalculator: reconstructMeasurements: reconstructing " << num_targets << " target(s) " << (settings_.multithreading ? "multithreaded" : "") << "...";

    //compute references in parallel
    if (settings_.multithreading)
    {
        tbb::parallel_for(uint(0), num_targets, [&](unsigned int tgt_cnt)
        {
            reconstructMeasurements(*refs[ tgt_cnt ]);
        });
    }
    else
    {
        for (unsigned int i = 0; i < num_targets; ++i)
        {
            reconstructMeasurements(*refs[ i ]);
        }
    }
}

/**
 */
boost::posix_time::ptime SimpleReferenceCalculator::getJoinThreshold() const
{
    const auto ThresJoin = reconstructor_.currentSlice().remove_before_time_
                           + (reconstructor_.currentSlice().slice_begin_ - reconstructor_.currentSlice().remove_before_time_) / 2;
    return ThresJoin;
}

/**
 */
SimpleReferenceCalculator::InitRecResult SimpleReferenceCalculator::initReconstruction(TargetReferences& refs)
{
    refs.start_index.reset();
    refs.init_update.reset();

    if (refs.measurements.size() < 1)
        return InitRecResult::NoMeasurements;

    //sort measurements by timestamp
    std::sort(refs.measurements.begin(), refs.measurements.end(), mmSortPred);

    if (reconstructor_.currentSlice().first_slice_)
    {
        refs.start_index = 0;
        return InitRecResult::Success;
    }

    //join old and new updates in the mid of the overlap timeframe
    const auto ThresJoin = getJoinThreshold();

    //get start index of new measurements (first measurement above join threshold)
    for (size_t i = 0; i < refs.measurements.size(); ++i)
    {
        if (refs.measurements[ i ].t >= ThresJoin)
        {
            refs.start_index = i;
            break;
        }
    }

    //no new measurements to add?
    if (!refs.start_index.has_value())
    {
        return InitRecResult::NoStartIndex;
    }

    //get last slice's last update for init
    if (!refs.updates.empty())
        refs.init_update = *refs.updates.rbegin();

    return InitRecResult::Success;
}

/**
 */
void SimpleReferenceCalculator::reconstructMeasurements(TargetReferences& refs)
{
    refs.resetCounts();

    bool general_debug = reconstructor_.task().debug();
    bool debug_target  = general_debug && reconstructor_.task().debugUTNs().count(refs.utn);

    const auto& debug_rec_nums = reconstructor_.task().debugRecNums();

    const auto& debug_ts_min  = reconstructor_.task().debugTimestampMin();
    const auto& debug_ts_max  = reconstructor_.task().debugTimestampMax();

    const auto& slice_t0 = reconstructor_.currentSlice().slice_begin_;
    const auto& slice_t1 = reconstructor_.currentSlice().next_slice_begin_;

    if(settings_.activeVerbosity() > 0) 
        loginf << "SimpleReferenceCalculator: reconstructMeasurements [UTN = " << refs.utn << "]";

    //try to init
    auto res = initReconstruction(refs);
    if (res != InitRecResult::Success)
    {
        //init failed
        if (settings_.activeVerbosity() > 0)
        {
            if (res == InitRecResult::NoMeasurements)
                loginf << "    skipping: no measurements";
            else if (res == InitRecResult::NoStartIndex)
                loginf << "    skipping: no start index found";
        }
        return;
    }

    if (settings_.activeVerbosity() > 0) 
    {
        loginf << "    #measurements: " << refs.measurements.size();
        loginf << "    #old updates:  " << refs.updates.size();
        loginf << "    start index:   " << refs.start_index.value();
        loginf << "    init update:   " << (refs.init_update.has_value() ? "yes" : "no");
    }

    assert(refs.start_index.has_value());

    //if (refs.utn == 21)
    //    writeTargetData(refs, "/home/mcphatty/track_utn21.json");

    //configure and init estimator
    reconstruction::KalmanEstimator estimator;
    estimator.settings() = settings_.kalmanEstimatorSettings();
    estimator.init(settings_.kalman_type_final);

    kalman::KalmanUpdate update;
    size_t offs     = 0;
    size_t n_before = refs.updates.size();
    size_t n_mm     = refs.measurements.size() - refs.start_index.value() + 1;

    refs.updates.reserve(n_before + n_mm);

    std::vector<unsigned int> used_mms;
    std::vector<QPointF>      failed_updates;
    std::vector<QPointF>      skipped_updates;
    if (debug_target)
    {
        used_mms.resize(n_before);
        used_mms.reserve(n_before + n_mm);
    }

    if (debug_target && shallAddAnnotationData())
    {
        addAnnotationData(refs,
                          "Input Measurements",
                          rec_annotations::AnnotationStyle(ColorMeasurements, PointSizeMeasurements, LineWidthBase),
                          rec_annotations::AnnotationStyle(ColorMeasurements, PointSizeOSG, LineWidthBase),
                          refs.measurements,
                          slice_t0, 
                          slice_t1,
                          refs.start_index.value());
    }

    auto collectUpdate = [ & ] (const kalman::KalmanUpdate& update, const 
                                reconstruction::Measurement& mm,
                                unsigned int mm_idx,
                                bool debug)
    {
        refs.updates.push_back(update);
        refs.updates.back().Q_var_interp = mm.Q_var_interp;

        if (debug)
            refs.updates.back().debugUpdate();

        if (debug_target)
            used_mms.push_back(mm_idx);
    };

    //init kalman (either from last slice's update or from new measurement)
    if (refs.init_update.has_value())
    {
        if (settings_.activeVerbosity() > 0) 
        {
            auto& mm0 = refs.measurements[ refs.start_index.value() ];
            loginf << "    initializing to update t=" << Utils::Time::toString(refs.init_update.value().t) << ", mm0 t=" << Utils::Time::toString(mm0.t);
        }

        //init kalman from last slice's update
        estimator.kalmanInit(refs.init_update.value());

        //continue with first measurement
    }
    else
    {
        auto& mm0      = refs.measurements[ refs.start_index.value() ];
        bool  debug_mm = debug_target && debug_rec_nums.count(mm0.source_id);

        if (settings_.activeVerbosity() > 0) 
        {
            loginf << "    initializing to mm t=" << Utils::Time::toString(mm0.t);
        }

        //reinit kalman with first measurement
        estimator.kalmanInit(update, mm0);
        assert(update.valid);

        collectUpdate(update, mm0, refs.start_index.value(), debug_mm);

        ++offs; //continue with second measurement
    }

    //add new measurements to kalman and collect updates
    for (size_t i = refs.start_index.value() + offs; i < refs.measurements.size(); ++i)
    {
        //estimator.enableDebugging(refs.utn == 0 && i == refs.start_index.value() + offs);

        const auto& mm       = refs.measurements[ i ];
        bool        debug_mm = debug_target && debug_rec_nums.count(mm.source_id);

        if (debug_mm && !debug_ts_min.is_not_a_date_time() && mm.t < debug_ts_min)
            debug_mm = false;
        if (debug_mm && !debug_ts_max.is_not_a_date_time() && mm.t > debug_ts_max)
            debug_mm = false;

        if (debug_mm)
        {
            loginf << "[ Debugging UTN " << refs.utn << " ID " << mm.source_id << " TS " << Utils::Time::toString(mm.t) << " ]\n\n"
                   << " * Before State:               \n\n" << estimator.asString()                             << "\n\n"
                   << " * Before State as Measurement:\n\n" << estimator.currentStateAsMeasurement().asString() << "\n\n"
                   << " * Measurement:                \n\n" << mm.asString()                                    << "\n\n";
        }

        auto res = estimator.kalmanStep(update, mm);

        if (debug_mm)
        {
            loginf << " * After State:                \n\n" << estimator.asString()                             << "\n\n"
                   << " * After State as Measurement: \n\n" << estimator.currentStateAsMeasurement().asString() << "\n";
        }

        //!only add update if valid!
        if (update.valid) 
            collectUpdate(update, mm, i, debug_mm);

        ++refs.num_updates;

        if (update.valid)
        {
            ++refs.num_updates_valid;
        }
        else if (res == reconstruction::KalmanEstimator::StepResult::FailStepTooSmall)
        {
            if (debug_target)
                skipped_updates.push_back(estimator.currentPositionWGS84());
            
            ++refs.num_updates_skipped;
        }
        else if (res == reconstruction::KalmanEstimator::StepResult::FailKalmanError)
        {
            if (debug_target)
                failed_updates.push_back(estimator.currentPositionWGS84());
            
            ++refs.num_updates_failed;

            const auto& step_info = estimator.stepInfo();

            if (step_info.kalman_error == kalman::KalmanError::Numeric)
                ++refs.num_updates_failed_numeric;
            else if (step_info.kalman_error == kalman::KalmanError::InvalidState)
                ++refs.num_updates_failed_badstate;
            else
                ++refs.num_updates_failed_other;  
        }
        else
        {
            ++refs.num_updates_failed;
            ++refs.num_updates_failed_other;
        }
    }

    if (settings_.activeVerbosity() > 0)
    {
        loginf << "    #new updates: " << refs.updates.size() - n_before;
        loginf << "    #updates: " << refs.updates.size();
    }

    if (debug_target && shallAddAnnotationData())
    {
        addAnnotationData(refs,
                          estimator, 
                          "Kalman", 
                          rec_annotations::AnnotationStyle(ColorKalman, PointSizeKalman, LineWidthBase),
                          rec_annotations::AnnotationStyle(ColorKalman, PointSizeOSG, LineWidthBase),
                          refs.updates,
                          slice_t0, 
                          slice_t1,
                          n_before,
                          true,
                          &used_mms,
                          &failed_updates,
                          &skipped_updates,
                          nullptr);
    }

    //start with joined kalman updates
    std::vector<kalman::KalmanUpdate> updates = refs.updates;
    
    //run rts smoothing?
    std::vector<kalman::RTSDebugInfo> rts_debug_infos;

    if (settings_.smooth_rts)
    {
        //jointly smooth old + new kalman updates RTS
        bool ok = estimator.smoothUpdates(updates, 
                                          kalman::SmoothFailStrategy::SetInvalid,
                                          debug_target ? &rts_debug_infos : nullptr);
        //assert(ok);

        if (!ok)
        {
            updates = refs.updates;
            ++refs.num_smoothing_failed;
        }

        //combine old rts updates with new rts updates
        refs.updates_smooth.reserve(n_before + n_mm);

        std::vector<unsigned int> used_mms_smooth;
        if (debug_target)
        {
            used_mms_smooth.resize(n_before);
            used_mms_smooth.reserve(n_before + n_mm);
        }

        size_t n_updates = updates.size();
        for (size_t i = n_before; i < n_updates; ++i)
        {
            if (updates[ i ].valid)
            {
                refs.updates_smooth.push_back(updates[ i ]);

                if (debug_target)
                    used_mms_smooth.push_back(used_mms[ i ]);
            }
            else
            {
                ++refs.num_smooth_steps_failed;
            }
        }

        updates = refs.updates_smooth;

        if (debug_target)
            used_mms = used_mms_smooth;

        if (settings_.activeVerbosity() > 0)
        {
            loginf << "    #updates (smoothed): " << updates.size();
        }
    }

    if (debug_target && shallAddAnnotationData())
    {
        loginf << "Collected " << rts_debug_infos.size() << " debug target(s)";

        addAnnotationData(refs,
                          estimator, 
                          "Kalman (RTS)", 
                          rec_annotations::AnnotationStyle(ColorKalmanSmoothed, PointSizeKalmanSmoothed, LineWidthBase),
                          rec_annotations::AnnotationStyle(ColorKalmanSmoothed, PointSizeOSG, LineWidthBase),
                          updates,
                          slice_t0,
                          slice_t1,
                          n_before,
                          false,
                          &used_mms,
                          nullptr,
                          nullptr,
                          &rts_debug_infos);
    }

    //resample?
    if (settings_.resample_result)
    {
        reconstruction::KalmanEstimator estimator_resample;

        //init resample estimator
        kalman::KalmanType type_resample = 
            settings_.kalman_type_final == kalman::KalmanType::IMMKalman2D ? kalman::KalmanType::AMKalman2D : settings_.kalman_type_final;

        estimator_resample.settings() = settings_.kalmanEstimatorSettings();
        estimator_resample.init(type_resample);

        //interpolate measurements
        size_t num_failed_steps;
        std::vector<kalman::KalmanUpdate> updates_interp;
        estimator_resample.interpUpdates(updates_interp, updates, &num_failed_steps);

        refs.num_interp_steps_failed += num_failed_steps;

        updates = updates_interp;

        if (settings_.activeVerbosity() > 0)
        {
            loginf << "    #updates (resampled): " << updates.size();
        }
    }

    if (debug_target && shallAddAnnotationData())
    {
        addAnnotationData(refs,
                          estimator, 
                          "Kalman (Resampled)", 
                          rec_annotations::AnnotationStyle(ColorKalmanResampled, PointSizeKalmanResampled, LineWidthBase),
                          rec_annotations::AnnotationStyle(ColorKalmanResampled, PointSizeOSG, LineWidthBase),
                          updates,
                          slice_t0,
                          slice_t1,
                          0,
                          false,
                          nullptr,
                          nullptr,
                          nullptr,
                          nullptr);
    }

    //generate references
    estimator.storeUpdates(refs.references, updates);
}

/**
 */
void SimpleReferenceCalculator::updateReferences()
{
    for (auto& ref : references_)
    {
        assert(reconstructor_.targets_container_.targets_.count(ref.first));

        auto& target = reconstructor_.targets_container_.targets_.at(ref.first);
        //target.references_ = std::move(ref.second.references);

        target.references_.clear();

        for (auto& ref_it : ref.second.references)
            target.references_[ref_it.t] = ref_it;

        ref.second.references.clear();

        dbContent::ReconstructorTarget::globalStats().num_rec_updates                 += ref.second.num_updates;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_valid           += ref.second.num_updates_valid;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_failed          += ref.second.num_updates_failed;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_failed_numeric  += ref.second.num_updates_failed_numeric;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_failed_badstate += ref.second.num_updates_failed_badstate;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_failed_other    += ref.second.num_updates_failed_other;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_skipped         += ref.second.num_updates_skipped;
        dbContent::ReconstructorTarget::globalStats().num_rec_smooth_steps_failed     += ref.second.num_smooth_steps_failed;
        dbContent::ReconstructorTarget::globalStats().num_rec_smooth_target_failed    += ref.second.num_smoothing_failed;
        dbContent::ReconstructorTarget::globalStats().num_rec_interp_failed           += ref.second.num_interp_steps_failed;
    }
}

/**
*/
bool SimpleReferenceCalculator::writeTargetData(TargetReferences& refs,
                                                const std::string& fn)
{
    if (refs.measurements.empty())
        return true;

    FrameProjector p;
    if (!refs.export_data)
    {
        refs.export_data.reset(new TargetExportData);

        refs.export_data->t0   = refs.measurements[ 0 ].t;
        refs.export_data->lat_ = refs.measurements[ 0 ].lat;
        refs.export_data->lon_ = refs.measurements[ 0 ].lon;
    }
    p.update(refs.export_data->lat_, refs.export_data->lon_);

    for (const auto& mm : refs.measurements)
    {
        if (mm.t <  reconstructor_.currentSlice().slice_begin_ ||
            mm.t >= reconstructor_.currentSlice().next_slice_begin_)
            continue;

        TestTarget::Measurement mm_tt;
        p.project(mm_tt.x, mm_tt.y, mm.lat, mm.lon);

        mm_tt.vx = mm.vx.has_value() ? mm.vx.value() : 0.0;
        mm_tt.vy = mm.vy.has_value() ? mm.vy.value() : 0.0;

        mm_tt.ax = mm.ax.has_value() ? mm.ax.value() : 0.0;
        mm_tt.ay = mm.ay.has_value() ? mm.ay.value() : 0.0;

        mm_tt.x_stddev = mm.x_stddev;
        mm_tt.y_stddev = mm.y_stddev;
        mm_tt.xy_cov = mm.xy_cov;

        mm_tt.vx_stddev = mm.vx_stddev;
        mm_tt.vy_stddev = mm.vy_stddev;

        mm_tt.ax_stddev = mm.ax_stddev;
        mm_tt.ay_stddev = mm.ay_stddev;

        mm_tt.t = Utils::Time::partialSeconds(mm.t - refs.export_data->t0.value());

        refs.export_data->test_target.addMeasurement(mm_tt);
    }

    return refs.export_data->test_target.exportJSON(fn);
}

/**
*/
bool SimpleReferenceCalculator::shallAddAnnotationData() const
{
    //add only if in last iteration
    return reconstructor_.isLastSliceProcessingRun();
}

/**
*/
void SimpleReferenceCalculator::createAnnotations()
{
    loginf << "SimpleReferenceCalculator: createAnnotations: Creating annotations from " << references_.size() << " reference(s)";

    if (references_.empty())
        return;

    std::vector<TargetReferences*>       refs;
    std::vector<ViewPointGenAnnotation*> annotations;

    const auto& task = reconstructor_.task();

    for (auto& ref : references_)
    {
        if (ref.second.annotation_data.empty())
            continue;

        loginf << "SimpleReferenceCalculator: createAnnotations: Creating annotation for UTN " << ref.second.utn;

        auto vp   = task.getDebugViewpointForUTN(ref.second.utn);
        auto anno = vp->annotations().getOrCreateAnnotation("Final Reconstruction");
        
        refs.push_back(&ref.second);
        annotations.push_back(anno);
    }

    unsigned int num_targets = refs.size();

    loginf << "SimpleReferenceCalculator: createAnnotations: creating annotations in parallel for " << refs.size() << " target(s)";

    tbb::parallel_for(uint(0), num_targets, [&](unsigned int tgt_cnt)
    {
        createAnnotations(annotations[ tgt_cnt ], *refs[ tgt_cnt ]);
    });
}

namespace rec_annotations
{
    /**
    */
    TRAnnotation createTRAnnotation(const reconstruction::Measurement& mm,
                                    const boost::optional<Eigen::Vector2d>& speed_pos,
                                    const boost::optional<Eigen::Vector2d>& accel_pos,
                                    const boost::optional<Eigen::Vector2d>& mm_pos)
    {
        TRAnnotation a;
        a.ts        = mm.t;
        a.pos_wgs84 = Eigen::Vector2d(mm.lat, mm.lon);
        a.pos_mm    = mm_pos;
        a.speed_pos = speed_pos;
        a.acc_pos   = accel_pos;
        a.Q_var     = mm.Q_var;

        if (mm.hasStdDevPosition())
            a.accuracy = Eigen::Vector3d(mm.x_stddev.value(), mm.y_stddev.value(), mm.xy_cov.has_value() ? mm.xy_cov.value() : 0.0);
        if (mm.hasVelocity())
            a.speed = std::sqrt(mm.vx.value() * mm.vx.value() + mm.vy.value() * mm.vy.value());
        if (mm.hasAcceleration())
            a.accel = std::sqrt(mm.ax.value() * mm.ax.value() + mm.ay.value() * mm.ay.value());

        return a;
    }

    /**
    */
    TRAnnotation createTRAnnotation(const reconstruction::Reference& ref,
                                    const boost::optional<Eigen::Vector2d>& speed_pos,
                                    const boost::optional<Eigen::Vector2d>& accel_pos,
                                    const boost::optional<Eigen::Vector2d>& mm_pos)
    {
        const reconstruction::Measurement* mm = &ref;
        auto anno = createTRAnnotation(*mm, speed_pos, accel_pos, mm_pos);

        anno.reset       = ref.reset_pos;
        anno.proj_change = ref.projchange_pos;

        return anno;
    }

    /**
    */
    TRAnnotation createTRAnnotation(const kalman::KalmanUpdate& update,
                                    const reconstruction::KalmanEstimator& estimator,
                                    reconstruction::KalmanProjectionHandler& phandler,
                                    const Eigen::Vector2d& proj_center,
                                    int submodel_idx,
                                    bool debug,
                                    const std::string& name)
    {
        reconstruction::Measurement mm;
        boost::optional<Eigen::Vector2d> speed_pos;
        boost::optional<Eigen::Vector2d> accel_pos;

        estimator.storeUpdateAndUnproject(mm, update, phandler, &speed_pos, &accel_pos, submodel_idx);

        if (debug)
            loginf << name << ": pos " << mm.lat << " " << mm.lon;

        return createTRAnnotation(mm, speed_pos, accel_pos, {});
    }

    /**
    */
    RTSAnnotation createRTSAnnotation(const kalman::RTSDebugInfo& rts_debug_info,
                                      const reconstruction::KalmanEstimator& estimator,
                                      reconstruction::KalmanProjectionHandler& phandler)
    {
        RTSAnnotation anno;

        assert(rts_debug_info.state0.imm_state);

        size_t nm = rts_debug_info.state0.imm_state->filter_states.size();

        anno.rts_step_models.resize(nm);
        anno.rts_step.color = QColor(255, 255, 255);

        auto createUpdate = [ & ] (const kalman::KalmanState& state)
        {
            kalman::KalmanUpdate update;
            update.projection_center = rts_debug_info.projection_center;
            update.state             = state;

            return update;
        };

        kalman::KalmanUpdate update_state0        = createUpdate(rts_debug_info.state0);
        kalman::KalmanUpdate update_state0_smooth = createUpdate(rts_debug_info.state0_smooth);
        kalman::KalmanUpdate update_state1        = createUpdate(rts_debug_info.state1);
        kalman::KalmanUpdate update_state1_smooth = createUpdate(rts_debug_info.state1_smooth);

        bool debug = false;

        anno.rts_step.state0        = createTRAnnotation(update_state0       , estimator, phandler, rts_debug_info.projection_center, -1, debug, "state0");
        anno.rts_step.state1        = createTRAnnotation(update_state1       , estimator, phandler, rts_debug_info.projection_center, -1, debug, "state1");
        anno.rts_step.state0_smooth = createTRAnnotation(update_state0_smooth, estimator, phandler, rts_debug_info.projection_center, -1, debug, "state0_smooth");
        anno.rts_step.state1_smooth = createTRAnnotation(update_state1_smooth, estimator, phandler, rts_debug_info.projection_center, -1, debug, "state1_smooth");

        for (size_t i = 0; i < nm; ++i)
        {
            auto& anno_model = anno.rts_step_models.at(i);

            std::string name = "model" + std::to_string(i);
            
            anno_model.color         = QColor(i % 3 == 0 ? 255 : 0, i % 3 == 1 ? 255 : 0, i % 3 == 2 ? 255 : 0);
            anno_model.state0        = createTRAnnotation(update_state0       , estimator, phandler, rts_debug_info.projection_center, (int)i, debug, name + "state0");
            anno_model.state1        = createTRAnnotation(update_state1       , estimator, phandler, rts_debug_info.projection_center, (int)i, debug, name + "state1");
            anno_model.state0_smooth = createTRAnnotation(update_state0_smooth, estimator, phandler, rts_debug_info.projection_center, (int)i, debug, name + "state0_smooth");
            anno_model.state1_smooth = createTRAnnotation(update_state1_smooth, estimator, phandler, rts_debug_info.projection_center, (int)i, debug, name + "state1_smooth");
        }

        std::stringstream ss;
        for (int i = 0; i < rts_debug_info.mu.size(); ++i)
            ss << rts_debug_info.mu[ i ] << " ";

        anno.extra_info = "";
        anno.extra_info += ss.str();

        return anno;
    }

    /**
    */
    IMMAnnotation createIMMAnnotation(const kalman::KalmanUpdate& imm_update,
                                      const reconstruction::KalmanEstimator& estimator,
                                      reconstruction::KalmanProjectionHandler& phandler)
    {
        assert(imm_update.state.imm_state);

        size_t nm = imm_update.state.imm_state->filter_states.size();

        IMMAnnotation anno;

        anno.imm_step_models.resize(nm);
        anno.imm_step.state = createTRAnnotation(imm_update, estimator, phandler, imm_update.projection_center, -1, false, "");
        anno.imm_step.color = QColor(255, 255, 255);

        for (size_t i = 0; i < nm; ++i)
        {
            auto& anno_model = anno.imm_step_models.at(i);

            std::string name = "model" + std::to_string(i);
            
            anno_model.color = QColor(i % 3 == 0 ? 255 : 0, i % 3 == 1 ? 255 : 0, i % 3 == 2 ? 255 : 0);
            anno_model.state = createTRAnnotation(imm_update, estimator, phandler, imm_update.projection_center, (int)i, false, "");
        }

        std::stringstream ss;
        for (int i = 0; i < imm_update.state.imm_state->mu.size(); ++i)
            ss << imm_update.state.imm_state->mu[ i ] << " ";

        anno.extra_info = "";
        anno.extra_info += ss.str();

        return anno;
    }
}

/**
*/
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const reconstruction::KalmanEstimator& estimator, 
                                                  const std::string& name,
                                                  const rec_annotations::AnnotationStyle& style,
                                                  const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                                                  const std::vector<kalman::KalmanUpdate>& updates,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs,
                                                  bool debug_imm,
                                                  const std::vector<unsigned int>* used_mms,
                                                  const std::vector<QPointF>* fail_pos,
                                                  const std::vector<QPointF>* skip_pos,
                                                  std::vector<kalman::RTSDebugInfo>* rts_debug_infos) const
{
    //store updates to references for easier access
    std::vector<reconstruction::Reference> references;
    std::vector<boost::optional<Eigen::Vector2d>> speed_positions, accel_positions;
    estimator.storeUpdates(references, updates, &speed_positions, &accel_positions);

    std::vector<rec_annotations::TRAnnotation> annos(references.size());

    for (size_t i = 0; i < references.size(); ++i)
    {
        boost::optional<Eigen::Vector2d> mm_pos;
        if (used_mms)
        {
            const auto& mm = target_references.measurements.at(used_mms->at(i));
            mm_pos = Eigen::Vector2d(mm.lat, mm.lon);
        }

        annos[ i ] = rec_annotations::createTRAnnotation(references[ i ], 
                                                         speed_positions[ i ], 
                                                         accel_positions[ i ], 
                                                         mm_pos);
    }

    reconstruction::KalmanProjectionHandler phandler;

    std::vector<rec_annotations::IMMAnnotation> imm_annotations;
    if (debug_imm)
    {
        for (const auto& u : updates)
            if (u.state.imm_state && u.isDebug())
                imm_annotations.push_back(rec_annotations::createIMMAnnotation(u, estimator, phandler));
    }
    
    std::vector<rec_annotations::RTSAnnotation> rts_annotations;
    if (rts_debug_infos)
    {
        for (const auto& rts_info : *rts_debug_infos)
        {
            auto rts_anno = rec_annotations::createRTSAnnotation(rts_info, estimator, phandler);
            rts_annotations.push_back(rts_anno);
        }
    }

    addAnnotationData(target_references, 
                      name, 
                      style, 
                      style_osg, 
                      annos, 
                      t0, 
                      t1, 
                      offs,
                      fail_pos,
                      skip_pos,
                      &imm_annotations,
                      &rts_annotations);
}

/**
 */
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const std::string& name,
                                                  const rec_annotations::AnnotationStyle& style,
                                                  const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                                                  const std::vector<reconstruction::Measurement>& measurements,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs) const
{
    std::vector<boost::optional<Eigen::Vector2d>> speed_positions, accel_positions;
    reconstruction::KalmanEstimator::extractVelAccPositionsWGS84(speed_positions, accel_positions, measurements);

    std::vector<rec_annotations::TRAnnotation> annos(measurements.size());

    for (size_t i = 0; i < measurements.size(); ++i)
    {
        annos[ i ] = rec_annotations::createTRAnnotation(measurements[ i ], 
                                                         speed_positions[ i ], 
                                                         accel_positions[ i ], 
                                                         {});
    }

    addAnnotationData(target_references, 
                      name, 
                      style, 
                      style_osg, 
                      annos, 
                      t0, 
                      t1, 
                      offs,
                      nullptr,
                      nullptr,
                      nullptr);
}

/**
 */
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const std::string& name,
                                                  const rec_annotations::AnnotationStyle& style,
                                                  const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                                                  const std::vector<rec_annotations::TRAnnotation>& annotations,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs,
                                                  const std::vector<QPointF>* fail_pos,
                                                  const std::vector<QPointF>* skip_pos,
                                                  std::vector<rec_annotations::IMMAnnotation>* imm_annotations,
                                                  std::vector<rec_annotations::RTSAnnotation>* rts_annotations) const
{
    rec_annotations::AnnotationData& data = target_references.annotation_data[ name ];

    data.name      = name;
    data.style     = style;
    data.style_osg = style_osg.has_value() ? style_osg.value() : style;

    size_t n     = annotations.size();
    size_t n_cur = data.annotations.size();

    bool added = false;

    for (size_t i = offs; i < n; ++i)
    {
        const auto& a = annotations.at(i);
        
        if (t0.has_value() && a.ts < t0.value()) continue;
        if (t1.has_value() && a.ts > t1.value()) continue;

        added = true;

        data.annotations.push_back(a);
    }

    if (fail_pos)
    {
        for (const auto& fp : *fail_pos)
            data.fail_positions.emplace_back(fp.x(), fp.y());
    }
    if (skip_pos)
    {
        for (const auto& sp : *skip_pos)
            data.skip_positions.emplace_back(sp.x(), sp.y());
    }
    if (imm_annotations)
    {
        data.imm_annotations.insert(data.imm_annotations.end(), imm_annotations->begin(), imm_annotations->end());
    }
    if (rts_annotations)
    {
        data.rts_annotations.insert(data.rts_annotations.end(), rts_annotations->begin(), rts_annotations->end());
    }

    if (added)
        data.slice_begins.push_back(n_cur);
}

/**
 */
void SimpleReferenceCalculator::createAnnotations(ViewPointGenAnnotation* annotation,
                                                  const TargetReferences& target_references) const
{
    //loginf << "SimpleReferenceCalculator: addAnnotations: Adding annotation '" << name << "' to parent '" << root.name() << "'";

    assert (annotation);

    auto parent_anno = annotation;
    auto common_anno = parent_anno->getOrCreateAnnotation("Common");

    const std::string PlotGroup = "Final Reconstruction";

    auto feat_speed_histo = new ViewPointGenFeatureHistogram(RawHistogramCollection(), 
        PlotMetadata("Reconstruction", "Speed Common", "Speed", "", PlotGroup));
    common_anno->addFeature(feat_speed_histo);

    auto feat_speed_scatter = new ViewPointGenFeatureScatterSeries(ScatterSeriesCollection(), 
        PlotMetadata("Reconstruction", "Speed Common", "Timestamp", "Speed", PlotGroup));
    common_anno->addFeature(feat_speed_scatter);

    auto scaleColor = [ & ] (const QColor& color, double factor)
    {
        double f = std::max(0.0, factor);
        return QColor(std::min(255, (int)(color.red()   * f)),
                      std::min(255, (int)(color.green() * f)),
                      std::min(255, (int)(color.blue()  * f)));
    };

    for (auto it = target_references.annotation_data.begin(); it != target_references.annotation_data.end(); ++it)
    {
        const auto& data = it->second;

        auto data_anno = parent_anno->getOrCreateAnnotation(data.name);

        const auto& style     = data.style;
        const auto& style_osg = data.style_osg;

        //const QColor ColorBrightest = scaleColor(style.base_color_, 1.5);
        const QColor ColorBright    = scaleColor(style.base_color_, 1.3);
        //const QColor ColorDark      = scaleColor(style.base_color_, 0.7);
        //const QColor ColorDarkest   = scaleColor(style.base_color_, 0.5);

        size_t na = data.annotations.size();

        //add positions
        {
            auto anno = data_anno->getOrCreateAnnotation("Positions");

            std::vector<Eigen::Vector2d> positions(na);
            for (size_t i = 0; i < na; ++i)
                positions[ i ] = data.annotations[ i ].pos_wgs84;

            //point feature
            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Cross, style_osg.point_size_, positions, {}, false);
            fp->setColor(style_osg.base_color_);
            anno->addFeature(fp);

            //line feature
            auto fl = new ViewPointGenFeatureLineString(false, style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Solid, positions, {}, false);
            fl->setColor(style_osg.base_color_);
            anno->addFeature(fl);
        }

        //add connections to input target reports
        {
            auto anno = data_anno->getOrCreateAnnotation("Input Data Connections", true);

            std::vector<Eigen::Vector2d> conn_lines;

            for (size_t i = 0; i < na; ++i)
            {
                if (!data.annotations[ i ].pos_mm.has_value())
                    continue;
                
                conn_lines.push_back(data.annotations[ i ].pos_wgs84);
                conn_lines.push_back(data.annotations[ i ].pos_mm.value());
            }

            auto f = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Dotted, conn_lines, {}, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);
        }

        //add velocities
        {
            auto anno = data_anno->getOrCreateAnnotation("Velocities", true);

            std::vector<Eigen::Vector2d> speed_lines;
            std::vector<double>          timestamps;
            std::vector<double>          values;

            for (size_t i = 0; i < na; ++i)
            {
                const auto& a = data.annotations[ i ];

                if (a.speed_pos.has_value())
                {
                    speed_lines.push_back(a.pos_wgs84);
                    speed_lines.push_back(a.speed_pos.value());

                    timestamps.push_back(Utils::Time::toLong(a.ts));
                    values.push_back(a.speed.value());
                }
            }

            auto f = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Solid, speed_lines, {}, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);

            //add histogram data series
            {
                //add to common feature
                RawHistogram h;
                HistogramInitializerT<double> init;
                if (init.createRAW(h, values, true, 20))
                    feat_speed_histo->histograms().addDataSeries(h, data.name, style.base_color_);

                //add own feature
                auto f = new ViewPointGenFeatureHistogram(h, data.name, style.base_color_, 
                    PlotMetadata("Reconstruction", "Speed " + data.name, "Speed", "", PlotGroup));
                anno->addFeature(f);
            }

            //add scatter data series
            {
                //add to common feature
                ScatterSeries series;
                for (size_t i = 0; i < timestamps.size(); ++i)
                    series.points.emplace_back(timestamps[ i ], values[ i ]);

                series.data_type_x = ScatterSeries::DataType::DataTypeTimestamp;

                feat_speed_scatter->scatterSeries().addDataSeries(series, data.name, style.base_color_, style.point_size_);

                //add own feature
                auto f = new ViewPointGenFeatureScatterSeries(series, data.name, style.base_color_, style.point_size_, 
                    PlotMetadata("Reconstruction", "Speed " + data.name, "Timestamp", "Speed", PlotGroup));
                anno->addFeature(f);
            }
        }

        //add accelerations
        {
            auto anno = data_anno->getOrCreateAnnotation("Accelerations", true);

            std::vector<Eigen::Vector2d> accel_lines;

            for (size_t i = 0; i < na; ++i)
            {
                const auto& a = data.annotations[ i ];

                if (a.acc_pos.has_value())
                {
                    accel_lines.push_back(a.pos_wgs84);
                    accel_lines.push_back(a.acc_pos.value());
                }
            }

            auto f = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Solid, accel_lines, {}, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);
        }

        //add accuracies
        {
            auto anno = data_anno->getOrCreateAnnotation("Accuracies", true);

            std::vector<Eigen::Vector2d> positions;
            std::vector<Eigen::Vector3d> accuracies;

            double lat_min = std::numeric_limits<double>::max();
            double lat_max = std::numeric_limits<double>::min();
            double lon_min = std::numeric_limits<double>::max();
            double lon_max = std::numeric_limits<double>::min();

            for (size_t i = 0; i < na; ++i)
            {
                const auto& a = data.annotations[ i ];

                if (a.accuracy.has_value())
                {
                    const auto& pos = a.pos_wgs84;

                    positions.push_back(pos);
                    accuracies.push_back(a.accuracy.value());

                    if (pos.x() < lat_min) lat_min = pos.x();
                    if (pos.x() > lat_max) lat_max = pos.x();
                    if (pos.y() < lon_min) lon_min = pos.y();
                    if (pos.y() > lon_max) lon_max = pos.y();
                }
            }

            auto f = new ViewPointGenFeatureErrEllipses(style_osg.line_width_, 32u, positions, {}, accuracies, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);

            //just for fun: add a grid
            QRectF roi(lon_min, lat_min, lon_max - lon_min, lat_max - lat_min);

            if (!roi.isEmpty())
            {
#if 0
                auto anno = data_anno->getOrCreateAnnotation("Accuracy Grid");

                Grid2D grid;
                grid.create(roi, grid2d::GridResolution().setCellCount(100, 100));
                
                for (size_t i = 0; i < positions.size(); ++i)
                    grid.addValue(positions[ i ].y(), positions[ i ].x(), std::max(accuracies[ i ].x(), accuracies[ i ].y()));

                Grid2DLayers layers;
                grid.addToLayers(layers, "min accuracy", grid2d::ValueType::ValueTypeMax);

                Grid2DRenderSettings rsettings;
                rsettings.pixels_per_cell = 10;
                rsettings.min_value       = 0.0;
                rsettings.max_value       = 10.0;

                rsettings.color_map.create(ColorMap::ColorScale::Green2Red, 10);
                
                auto result = Grid2DLayerRenderer::render(layers.layer(0), rsettings);

                auto f = new ViewPointGenFeatureGeoImage(result.first, result.second);
                anno->addFeature(f);
#endif
            }
        }

        //add process noise graph
        {
            auto anno = data_anno->getOrCreateAnnotation("Additional Infos", true);

            ScatterSeries series;
            for (size_t i = 0; i < na; ++i)
            {
                const auto& a = data.annotations[ i ];

                if (a.Q_var.has_value())
                    series.points.emplace_back(Utils::Time::toLong(a.ts), a.Q_var.value());
            }
            series.points.emplace_back(series.points.back().x(), 0.0);
            series.data_type_x = ScatterSeries::DataType::DataTypeTimestamp;

            //add own feature
            auto f = new ViewPointGenFeatureScatterSeries(series, data.name, style.base_color_, style.point_size_, 
                PlotMetadata("Reconstruction", "Used Q_vars " + data.name, "Timestamp", "Q_var", PlotGroup));
            anno->addFeature(f);
        }

        //add special positions
        {
            std::vector<Eigen::Vector2d> positions;
            for (size_t i = 0; i < na; ++i)
                if (data.annotations[ i ].reset)
                    positions.push_back(data.annotations[ i ].pos_wgs84);

            if (!positions.empty())
            {
                auto anno = data_anno->getOrCreateAnnotation("Reset Positions", true);

                auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, positions, {}, false);
                fp->setColor(ColorBright);
                anno->addFeature(fp);
            }
        }
        {
            std::vector<Eigen::Vector2d> positions;
            for (size_t i = 0; i < na; ++i)
                if (data.annotations[ i ].proj_change)
                    positions.push_back(data.annotations[ i ].pos_wgs84);

            if (!positions.empty())
            {
                auto anno = data_anno->getOrCreateAnnotation("Projection Change Positions", true);

                auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, positions, {}, false);
                fp->setColor(ColorBright);
                anno->addFeature(fp);
            }
        }

        //add fail positions
        if (!data.fail_positions.empty())
        {
            auto anno = data_anno->getOrCreateAnnotation("Fail Positions", true);

            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, data.fail_positions, {}, false);
            fp->setColor(ColorBright);
            anno->addFeature(fp);
        }

        //add skip positions
        if (!data.skip_positions.empty())
        {
            auto anno = data_anno->getOrCreateAnnotation("Skip Positions", true);

            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, data.skip_positions, {}, false);
            fp->setColor(ColorBright);
            anno->addFeature(fp);
        }

        //add slice begin positions
        {
            auto anno = data_anno->getOrCreateAnnotation("Slice Begins", true);

            std::vector<Eigen::Vector2d> positions;
            for (auto idx : data.slice_begins)
                positions.push_back(data.annotations[ idx ].pos_wgs84);

            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, positions, {}, false);
            fp->setColor(ColorBright);
            anno->addFeature(fp);
        }

        //add rts debug annotations
        {
            auto anno_rts = data_anno->getOrCreateAnnotation("RTS Infos", true);

            loginf << "Adding " << data.rts_annotations.size() << " RTS info(s)";

            size_t cnt = 0;
            for (const auto& rts_anno : data.rts_annotations)
            {
                std::string name = "RTSInfo" + std::to_string(cnt) + (rts_anno.extra_info.empty() ? "" : ": " + rts_anno.extra_info);

                auto anno_info = anno_rts->getOrCreateAnnotation(name);

                std::vector<Eigen::Vector2d> positions_smooth;
                std::vector<Eigen::Vector2d> positions_nonsmooth;
                std::vector<Eigen::Vector2d> connections;
                std::vector<QColor> colors;

                //smoothed position
                positions_smooth.push_back(rts_anno.rts_step.state0_smooth.pos_wgs84);
                positions_nonsmooth.push_back(rts_anno.rts_step.state0.pos_wgs84);
                connections.push_back(positions_smooth.back());
                connections.push_back(positions_nonsmooth.back());
                colors.push_back(rts_anno.rts_step.color);

                //smoothed submodel positions
                for (const auto& rts_anno_model : rts_anno.rts_step_models)
                {
                    positions_smooth.push_back(rts_anno_model.state0_smooth.pos_wgs84);
                    positions_nonsmooth.push_back(rts_anno_model.state0.pos_wgs84);
                    connections.push_back(positions_smooth.back());
                    connections.push_back(positions_nonsmooth.back());
                    colors.push_back(rts_anno_model.color);
                }

                auto anno_smooth = anno_info->getOrCreateAnnotation("Smooth Positions");

                auto fp_smooth = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Cross, style_osg.point_size_, positions_smooth, colors, true);
                anno_smooth->addFeature(fp_smooth);

                auto anno_input = anno_info->getOrCreateAnnotation("Input Positions");

                auto fp_nonsmooth = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Circle, style_osg.point_size_ * 0.7, positions_nonsmooth, colors, true);
                anno_input->addFeature(fp_nonsmooth);

                auto fl = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Dotted, connections, {}, true);
                fl->setColor(QColor(200, 200, 200));
                anno_input->addFeature(fl);

                ++cnt;
            }
        }

        //add imm debug annotations
        {
            auto anno_rts = data_anno->getOrCreateAnnotation("IMM Infos", true);

            loginf << "Adding " << data.imm_annotations.size() << " IMM info(s)";

            size_t cnt = 0;
            for (const auto& imm_anno : data.imm_annotations)
            {
                std::string name = "IMMInfo" + std::to_string(cnt) + (imm_anno.extra_info.empty() ? "" : ": " + imm_anno.extra_info);

                auto anno_info = anno_rts->getOrCreateAnnotation(name);

                std::vector<Eigen::Vector2d> positions;
                std::vector<QColor> colors;

                //position
                positions.push_back(imm_anno.imm_step.state.pos_wgs84);
                colors.push_back(imm_anno.imm_step.color);

                //submodel positions
                for (const auto& imm_anno_model : imm_anno.imm_step_models)
                {
                    positions.push_back(imm_anno_model.state.pos_wgs84);
                    colors.push_back(imm_anno_model.color);
                }

                auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Cross, style_osg.point_size_, positions, colors, true);
                anno_info->addFeature(fp);

                ++cnt;
            }
        }
    }
}

#if 0

/**
*/
std::vector<std::vector<Measurement>> Reconstructor::splitMeasurements(const std::vector<Measurement>& measurements,
                                                                       double max_dt)
{
    size_t n = measurements.size();
    if (n == 0)
        return {};

    std::vector<std::vector<Measurement>> split_measurements;
    std::vector<Measurement> current;

    current.push_back(measurements[ 0 ]);

    for (size_t i = 1; i < n; ++i)
    {
        const auto& mm0 = measurements[i - 1];
        const auto& mm1 = measurements[i    ];

        double dt = timestep(mm0, mm1);

        if (dt > max_dt)
        {
            if (current.size() > 0)
            {
                split_measurements.push_back(current);
                current.clear();
            }
        }

        current.push_back(mm1);
    }

    if (current.size() > 0)
        split_measurements.push_back(current);

    return split_measurements;
}

#endif

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

#include "viewpointgenerator.h"
#include "histograminitializer.h"
#include "scatterseries.h"
#include "grid2d.h"
#include "grid2dlayer.h"

#include "util/timeconv.h"
#include "util/number.h"
#include "stringconv.h"
#include "logger.h"

#include <ogr_spatialref.h>

#include "tbbhack.h"

using namespace Utils;

const QColor SimpleReferenceCalculator::ColorMeasurements   = QColor(102, 178, 255);
const QColor SimpleReferenceCalculator::ColorKalman         = QColor(255, 102, 178);
const QColor SimpleReferenceCalculator::ColorKalmanSmoothed = QColor(255, 178, 102);

const float SimpleReferenceCalculator::PointSizeBase           = 6.0f;
const float SimpleReferenceCalculator::PointSizeMeasurements   = 6.0f;
const float SimpleReferenceCalculator::PointSizeKalman         = 5.0f;
const float SimpleReferenceCalculator::PointSizeKalmanSmoothed = 4.0f;

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

    annotation = nullptr;
}

/**
 */
void SimpleReferenceCalculator::TargetReferences::resetCounts()
{
    num_updates             = 0;
    num_updates_valid       = 0;
    num_updates_failed      = 0;
    num_updates_skipped     = 0;
    num_smooth_steps_failed = 0;
    num_interp_steps_failed = 0;
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

        ++slice_idx_;
    }
    else
    {
        slice_idx_ = 0;
    }

    is_last_slice_ = reconstructor_.currentSlice().is_last_slice_;

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
    const auto& task       = reconstructor_.task();
    const auto& debug_utns = task.debugUTNs();

    for (auto& ref : references_)
    {
        ref.second.reset();

        if (task.debug() && debug_utns.count(ref.second.utn))
        {
            auto vp = task.getDebugViewpointForUTN(ref.second.utn);
            ref.second.annotation = vp->annotations().getOrCreateAnnotation("Final Reconstruction");
        }
    }

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
    for (const auto& target : reconstructor_.targets_)
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
        reconstructor_.createMeasurement(mm, tr_info);

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
    {
        loginf << "SimpleReferenceCalculator: reconstructMeasurements [UTN = " << refs.utn << "]";
    }

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

    //configure and init estimator
    reconstruction::KalmanEstimator estimator;
    estimator.settings() = settings_.kalmanEstimatorSettings();
    estimator.init(settings_.kalman_type);

    kalman::KalmanUpdate update;
    size_t offs     = 0;
    size_t n_before = refs.updates.size();
    size_t n_mm     = refs.measurements.size() - refs.start_index.value() + 1;

    refs.updates.reserve(n_before + n_mm);

    std::vector<unsigned int> used_mms;
    if (debug_target)
    {
        used_mms.resize(n_before);
        used_mms.reserve(n_before + n_mm);
    }

    // auto ThresRemove = reconstructor_.remove_before_time_;
    // auto ThresJoin   = getJoinThreshold();

    // loginf << "update_end: " << (refs.updates.empty() ? "-" : Utils::Time::toString(refs.updates.rbegin()->t)) << ", "
    //        << "mm_begin: " << Utils::Time::toString(refs.measurements[ refs.start_index.value() ].t) << ", "
    //        << "thres_rem: " << Utils::Time::toString(ThresRemove) << ", thres_join: " << Utils::Time::toString(ThresJoin) << ", "
    //        << "idx: " << refs.start_index.value();

#if 0
    for (size_t i = refs.start_index.value(); i < refs.measurements.size(); ++i)
    {
        const auto& mm = refs.measurements[ i ];

        reconstruction::Reference ref;
        (reconstruction::Measurement&)ref = mm;

        refs.references.push_back(ref);
    }

    return;
#endif

    if (debug_target)
    {
        addAnnotationData(refs,
                          "Input Measurements",
                          AnnotationStyle(ColorMeasurements, PointSizeMeasurements, LineWidthBase),
                          AnnotationStyle(ColorMeasurements, PointSizeBase, LineWidthBase),
                          refs.measurements,
                          slice_t0, 
                          slice_t1,
                          refs.start_index.value());
    }

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
        auto& mm0 = refs.measurements[ refs.start_index.value() ];

        if (settings_.activeVerbosity() > 0) 
        {
            loginf << "    initializing to mm t=" << Utils::Time::toString(mm0.t);
        }

        //reinit kalman with first measurement
        estimator.kalmanInit(update, mm0);
        assert(update.valid);

        refs.updates.push_back(update);

        if (debug_target)
            used_mms.push_back(refs.start_index.value());

        ++offs; //continue with second measurement
    }

    //add new measurements to kalman and collect updates
    
    for (size_t i = refs.start_index.value() + offs; i < refs.measurements.size(); ++i)
    {
        const auto& mm = refs.measurements[ i ];

        bool debug_mm = debug_target && debug_rec_nums.count(mm.source_id);

        if (debug_mm && !debug_ts_min.is_not_a_date_time() && mm.t < debug_ts_min)
            debug_mm = false;
        if (debug_mm && !debug_ts_max.is_not_a_date_time() && mm.t > debug_ts_max)
            debug_mm = false;

        if (debug_mm)
        {
            loginf << "[ Debugging UTN " << refs.utn << " ID " << mm.source_id << " TS " << Utils::Time::toString(mm.t) << " ]\n\n"
                   << " * State:               \n\n" << estimator.asString()                             << "\n\n"
                   << " * State as Measurement:\n\n" << estimator.currentStateAsMeasurement().asString() << "\n\n"
                   << " * Measurement:         \n\n" << mm.asString()                                    << "\n";
        }

        auto res = estimator.kalmanStep(update, mm);

        //!only add update if valid!
        if (update.valid)
        {
            refs.updates.push_back(update);

            if (debug_target)
                used_mms.push_back(i);
        }

        ++refs.num_updates;

        if (update.valid)
            ++refs.num_updates_valid;
        else if (res == reconstruction::KalmanEstimator::StepResult::FailStepTooSmall)
            ++refs.num_updates_skipped;
        else
            ++refs.num_updates_failed;
    }

    if (settings_.activeVerbosity() > 0)
    {
        loginf << "    #new updates: " << refs.updates.size() - n_before;
        loginf << "    #updates: " << refs.updates.size();
    }

    if (debug_target)
    {
        addAnnotationData(refs,
                          estimator, 
                          "Kalman", 
                          AnnotationStyle(ColorKalman, PointSizeKalman, LineWidthBase),
                          AnnotationStyle(ColorKalman, PointSizeBase, LineWidthBase),
                          refs.updates,
                          used_mms,
                          slice_t0, 
                          slice_t1,
                          n_before);
    }

    //start with joined kalman updates
    std::vector<kalman::KalmanUpdate> updates = refs.updates;
    
    //run rts smoothing?
    if (settings_.smooth_rts)
    {
        //jointly smooth old + new kalman updates RTS
        bool ok = estimator.smoothUpdates(updates, kalman::SmoothFailStrategy::SetInvalid);
        assert(ok);

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

    if (debug_target)
    {
        addAnnotationData(refs,
                          estimator, 
                          "Kalman (RTS)", 
                          AnnotationStyle(ColorKalmanSmoothed, PointSizeKalmanSmoothed, LineWidthBase),
                          AnnotationStyle(ColorKalmanSmoothed, PointSizeBase, LineWidthBase),
                          updates,
                          used_mms, 
                          slice_t0, 
                          slice_t1,
                          n_before);
    }

    //resample?
    if (settings_.resample_result)
    {
        //interpolate measurements
        size_t num_failed_steps;
        std::vector<kalman::KalmanUpdate> updates_interp;
        estimator.interpUpdates(updates_interp, updates, &num_failed_steps);

        refs.num_interp_steps_failed += num_failed_steps;

        updates = updates_interp;

        if (settings_.activeVerbosity() > 0)
        {
            loginf << "    #updates (resampled): " << updates.size();
        }
    }

    //generate references
    estimator.storeUpdates(refs.references, updates);

    //create annotations?
    if (debug_target && is_last_slice_)
        addAnnotations(refs);
}

/**
*/
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const reconstruction::KalmanEstimator& estimator, 
                                                  const std::string& name,
                                                  const AnnotationStyle& style,
                                                  const boost::optional<AnnotationStyle>& style_osg,
                                                  const std::vector<kalman::KalmanUpdate>& updates,
                                                  const std::vector<unsigned int>& used_mms,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs) const
{
    //store updates to references for easier access
    std::vector<reconstruction::Reference> references;
    std::vector<boost::optional<Eigen::Vector2d>> speed_positions, accel_positions;
    estimator.storeUpdates(references, updates, &speed_positions, &accel_positions);

    std::vector<reconstruction::Measurement> measurements(references.size());
    for (size_t i = 0; i < references.size(); ++i)
        measurements[ i ] = references[ i ];

    addAnnotationData(target_references, 
                      name, 
                      style, 
                      style_osg, 
                      measurements, 
                      t0, 
                      t1, 
                      offs, 
                      speed_positions, 
                      accel_positions,
                      &used_mms);
}

/**
 */
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const std::string& name,
                                                  const AnnotationStyle& style,
                                                  const boost::optional<AnnotationStyle>& style_osg,
                                                  const std::vector<reconstruction::Measurement>& measurements,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs) const
{
    std::vector<boost::optional<Eigen::Vector2d>> speed_positions, accel_positions;
    reconstruction::KalmanEstimator::extractVelAccPositionsWGS84(speed_positions, accel_positions, measurements);

    addAnnotationData(target_references, 
                      name, 
                      style, 
                      style_osg, 
                      measurements, 
                      t0, 
                      t1, 
                      offs, 
                      speed_positions, 
                      accel_positions,
                      nullptr);
}

/**
 */
void SimpleReferenceCalculator::addAnnotationData(TargetReferences& target_references,
                                                  const std::string& name,
                                                  const AnnotationStyle& style,
                                                  const boost::optional<AnnotationStyle>& style_osg,
                                                  const std::vector<reconstruction::Measurement>& reconstructed_measurements,
                                                  const boost::optional<boost::posix_time::ptime>& t0,
                                                  const boost::optional<boost::posix_time::ptime>& t1,
                                                  size_t offs,
                                                  const std::vector<boost::optional<Eigen::Vector2d>>& vel_pos_wgs84,
                                                  const std::vector<boost::optional<Eigen::Vector2d>>& acc_pos_wgs84,
                                                  const std::vector<unsigned int>* used_input_mms) const
{
    AnnotationData& data = target_references.annotation_data[ name ];

    data.name      = name;
    data.style     = style;
    data.style_osg = style_osg.has_value() ? style_osg.value() : style;

    size_t n     = reconstructed_measurements.size();
    size_t n_cur = data.positions.size();

    bool added = false;

    for (size_t i = offs; i < n; ++i)
    {
        const auto& r = reconstructed_measurements.at(i);
        
        if (t0.has_value() && r.t < t0.value()) continue;
        if (t1.has_value() && r.t > t1.value()) continue;

        added = true;

        const auto mm = used_input_mms ? &target_references.measurements.at(used_input_mms->at(i)) : nullptr;

        data.timestamps.push_back(r.t);

        data.positions.emplace_back(r.lat, r.lon);
        if (mm) data.positions_mm.emplace_back(mm->lat, mm->lon);

        data.speed_positions.push_back(vel_pos_wgs84.at(i));
        data.accel_positions.push_back(acc_pos_wgs84.at(i));

        data.accuracies.push_back(r.hasStdDevPosition() ? Eigen::Vector3d(r.x_stddev.value(), r.y_stddev.value(), r.xy_cov.has_value() ? r.xy_cov.value() : 0.0) :
                                                          boost::optional<Eigen::Vector3d>());

        data.speeds.push_back(r.hasVelocity() ? std::sqrt(r.vx.value() * r.vx.value() + r.vy.value() * r.vy.value()) : 
                                                boost::optional<double>());
        data.accelerations.push_back(r.hasAcceleration() ? std::sqrt(r.ax.value() * r.ax.value() + r.ay.value() * r.ay.value()) : 
                                                           boost::optional<double>());
    }

    if (added)
        data.slice_begins.push_back(n_cur);
}

/**
 */
void SimpleReferenceCalculator::addAnnotations(const TargetReferences& target_references) const
{
    //loginf << "SimpleReferenceCalculator: addAnnotations: Adding annotation '" << name << "' to parent '" << root.name() << "'";

    assert (target_references.annotation);

    auto parent_anno = target_references.annotation;
    auto common_anno = parent_anno->getOrCreateAnnotation("Common");

    const std::string PlotGroup = "Final Reconstruction";

    auto feat_speed_histo = new ViewPointGenFeatureHistogram(RawHistogramCollection(), 
        PlotMetadata("Reconstruction", "Speed Common", "Speed", "", PlotGroup));
    common_anno->addFeature(feat_speed_histo);

    auto feat_speed_scatter = new ViewPointGenFeatureScatterSeries(ScatterSeriesCollection(), 
        PlotMetadata("Reconstruction", "Speed Common", "Timestamp", "Speed", PlotGroup));
    common_anno->addFeature(feat_speed_scatter);

    for (const auto& elem : target_references.annotation_data)
    {
        const auto& data = elem.second;

        auto data_anno = parent_anno->getOrCreateAnnotation(data.name);

        const auto& style     = data.style;
        const auto& style_osg = data.style_osg;

        //add positions
        {
            auto anno = data_anno->getOrCreateAnnotation("Positions");

            //point feature
            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Cross, style_osg.point_size_, data.positions, {}, false);
            fp->setColor(style_osg.base_color_);
            anno->addFeature(fp);

            //line feature
            auto fl = new ViewPointGenFeatureLineString(false, style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Solid, data.positions, {}, false);
            fl->setColor(style_osg.base_color_);
            anno->addFeature(fl);
        }

        //add slice begin positions
        {
            auto anno = data_anno->getOrCreateAnnotation("Slice Begins");

            std::vector<Eigen::Vector2d> positions;
            for (auto idx : data.slice_begins)
                positions.push_back(data.positions.at(idx));

            auto fp = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::BorderThick, style_osg.point_size_ * 1.5f, positions, {}, false);
            fp->setColor(style_osg.base_color_);
            anno->addFeature(fp);
        }

        //add connections to input target reports
        if (data.positions_mm.size() == data.positions.size())
        {
            auto anno = data_anno->getOrCreateAnnotation("Input Data Connections");

            std::vector<Eigen::Vector2d> conn_lines;

            for (size_t i = 0; i < data.positions.size(); ++i)
            {
                conn_lines.push_back(data.positions[ i ]);
                conn_lines.push_back(data.positions_mm[ i ]);
            }

            auto f = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Dotted, conn_lines, {}, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);
        }

        //add velocities
        if (data.speeds.size() == data.positions.size())
        {
            auto anno = data_anno->getOrCreateAnnotation("Velocities");

            std::vector<Eigen::Vector2d> speed_lines;
            std::vector<double>          timestamps;
            std::vector<double>          values;

            for (size_t i = 0; i < data.positions.size(); ++i)
            {
                if (data.speed_positions[ i ].has_value())
                {
                    speed_lines.push_back(data.positions[ i ]);
                    speed_lines.push_back(data.speed_positions[ i ].value());

                    timestamps.push_back(Utils::Time::toLong(data.timestamps[ i ]));
                    values.push_back(data.speeds[ i ].value());
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
        if (data.accelerations.size() == data.positions.size())
        {
            auto anno = data_anno->getOrCreateAnnotation("Accelerations");

            std::vector<Eigen::Vector2d> accel_lines;

            for (size_t i = 0; i < data.positions.size(); ++i)
            {
                if (data.accel_positions[ i ].has_value())
                {
                    accel_lines.push_back(data.positions[ i ]);
                    accel_lines.push_back(data.accel_positions[ i ].value());
                }
            }

            auto f = new ViewPointGenFeatureLines(style_osg.line_width_, ViewPointGenFeatureLineString::LineStyle::Solid, accel_lines, {}, false);
            f->setColor(style_osg.base_color_);
            anno->addFeature(f);
        }

        //add accuracies
        if (data.accuracies.size() == data.positions.size())
        {
            auto anno = data_anno->getOrCreateAnnotation("Accuracies");

            std::vector<Eigen::Vector2d> positions;
            std::vector<Eigen::Vector3d> accuracies;

            double lat_min = std::numeric_limits<double>::max();
            double lat_max = std::numeric_limits<double>::min();
            double lon_min = std::numeric_limits<double>::max();
            double lon_max = std::numeric_limits<double>::min();

            for (size_t i = 0; i < data.positions.size(); ++i)
            {
                if (data.accuracies[ i ].has_value())
                {
                    const auto& pos = data.positions[ i ];

                    positions.push_back(pos);
                    accuracies.push_back(data.accuracies[ i ].value());

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
    }
}

/**
 */
void SimpleReferenceCalculator::updateReferences()
{
    for (auto& ref : references_)
    {
        assert(reconstructor_.targets_.count(ref.first));

        auto& target = reconstructor_.targets_.at(ref.first);
        //target.references_ = std::move(ref.second.references);

        target.references_.clear();

        for (auto& ref_it : ref.second.references)
            target.references_[ref_it.t] = ref_it;

        ref.second.references.clear();

        dbContent::ReconstructorTarget::globalStats().num_rec_updates         += ref.second.num_updates;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_valid   += ref.second.num_updates_valid;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_failed  += ref.second.num_updates_failed;
        dbContent::ReconstructorTarget::globalStats().num_rec_updates_skipped += ref.second.num_updates_skipped;
        dbContent::ReconstructorTarget::globalStats().num_rec_smooth_failed   += ref.second.num_smooth_steps_failed;
        dbContent::ReconstructorTarget::globalStats().num_rec_interp_failed   += ref.second.num_interp_steps_failed;
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

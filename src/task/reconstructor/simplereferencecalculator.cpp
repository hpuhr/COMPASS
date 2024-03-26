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
#include "simplereconstructor.h"
#include "targetreportdefs.h"

#include "kalman_estimator.h"
#include "kalman_interface_umkalman2d.h"
#if USE_EXPERIMENTAL_SOURCE
#include "kalman_interface_amkalman2d.h"
#endif

#include "util/timeconv.h"
#include "util/number.h"
#include "logger.h"

#include <ogr_spatialref.h>

#include "tbbhack.h"

/**
*/
SimpleReferenceCalculator::SimpleReferenceCalculator(SimpleReconstructor& reconstructor)
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
    if (!reconstructor_.first_slice_)
    {
        auto ThresRemove = reconstructor_.remove_before_time_;
        auto ThresJoin   = getJoinThreshold();

        //remove previous updates which are no longer needed (either too old or above the join threshold)
        for (auto& ref : references_)
        {
            auto it = std::remove_if(ref.second.updates.begin(),
                                     ref.second.updates.end(),
                                     [ & ] (const kalman::KalmanUpdate& update) { return update.t <  ThresRemove ||
                                                                                         update.t >= ThresJoin; });
            ref.second.updates.erase(it, ref.second.updates.end());
        }
    }

    //reset data structs
    reset();
}

/**
*/
void SimpleReferenceCalculator::reset()
{
    for (auto& ref : references_)
    {
        ref.second.measurements.resize(0);
        ref.second.references.resize(0);
        ref.second.init_update.reset();
        ref.second.start_index.reset();
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

    reset();
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
    std::vector<reconstruction::Measurement> line_measurements;

    for (const auto& elem : target_reports)
    {
        const auto& tr_info = reconstructor_.target_reports_.at(elem.second);

        reconstruction::Measurement mm;
        mm.source_id = elem.second;
        mm.t         = elem.first;
        
        auto pos = tr_info.position_;
        assert(pos.has_value());

        auto vel = tr_info.velocity_;

        auto pos_acc = reconstructor_.acc_estimator_.positionAccuracy(tr_info);
        auto vel_acc = reconstructor_.acc_estimator_.velocityAccuracy(tr_info);
        auto acc_acc = reconstructor_.acc_estimator_.accelerationAccuracy(tr_info);

        //position
        mm.lat = pos.value().latitude_;
        mm.lon = pos.value().longitude_;

        //velocity
        if (vel.has_value())
        {
            auto speed_vec = Utils::Number::speedAngle2SpeedVec(vel->speed_, vel->track_angle_);

            mm.vx = speed_vec.first;
            mm.vy = speed_vec.second;
            //@TODO: vz?
        }

        //@TODO: acceleration?

        //accuracies
        mm.x_stddev = pos_acc.x_stddev_;
        mm.y_stddev = pos_acc.y_stddev_;
        mm.xy_cov   = pos_acc.xy_cov_;

        mm.vx_stddev = vel_acc.vx_stddev_;
        mm.vy_stddev = vel_acc.vy_stddev_;

        mm.ax_stddev = acc_acc.ax_stddev_;
        mm.ay_stddev = acc_acc.ay_stddev_;

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

    if (!measurements.empty())
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

    //compute references in parallel
#if 0
    tbb::parallel_for(uint(0), num_targets, [&](unsigned int tgt_cnt)
    {
        reconstructMeasurements(*refs[ tgt_cnt ]);
    });
#else
    for (unsigned int i = 0; i < num_targets; ++i)
    {
        reconstructMeasurements(*refs[ i ]);
    }
#endif
}

/**
*/
boost::posix_time::ptime SimpleReferenceCalculator::getJoinThreshold() const
{
    const auto ThresJoin = reconstructor_.remove_before_time_ + (reconstructor_.current_slice_begin_ - reconstructor_.remove_before_time_) / 2;
    return ThresJoin;
}

/**
*/
bool SimpleReferenceCalculator::initReconstruction(TargetReferences& refs)
{
    refs.start_index.reset();
    refs.init_update.reset();

    if (refs.measurements.size() < 1)
    {
        loginf << "    initReconstruction: UTN " << refs.utn << " obtains no measurements";
        return false;
    }

    //sort measurements by timestamp
    std::sort(refs.measurements.begin(), refs.measurements.end(), mmSortPred);

    if (reconstructor_.first_slice_)
    {
        refs.start_index = 0;
        return true;
    }

    //join old and new updates in the mid of the overlap timeframe
    const auto ThresJoin = getJoinThreshold();

    loginf << "    initReconstruction: ThresJoin = " 
           << Utils::Time::toString(ThresJoin)
           << ", remove before time = " << Utils::Time::toString(reconstructor_.remove_before_time_)
           << ", slice begin = " << Utils::Time::toString(reconstructor_.current_slice_begin_);

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
        loginf << "    initReconstruction: UTN " << refs.utn << " obtains no measurement above t = " << Utils::Time::toString(ThresJoin);
        return false;
    }

    //get last slice's last update for init
    if (!refs.updates.empty())
        refs.init_update = *refs.updates.rbegin();

    return true;
}

/**
*/
void SimpleReferenceCalculator::reconstructMeasurements(TargetReferences& refs)
{
    loginf << "SimpleReferenceCalculator: reconstructMeasurements [UTN = " << refs.utn << "]";

    //init
    if (!initReconstruction(refs))
    {
        loginf << "    init failed";
        return;
    }

    loginf << "    #measurements: " << refs.measurements.size();
    loginf << "    #old updates:  " << refs.updates.size();
    loginf << "    start index:   " << refs.start_index.value();
    loginf << "    init update:   " << (refs.init_update.has_value() ? "yes" : "no");

    assert(refs.start_index.has_value());

    //create and configure kalman estimator
    reconstruction::KalmanEstimator estimator;

    auto rec_type = settings_.rec_type;

    std::unique_ptr<reconstruction::KalmanInterface> interface;
    if (rec_type == Settings::Rec_UMKalman2D)
    {
        interface.reset(new reconstruction::KalmanInterfaceUMKalman2D(true));
    }
    else
    {
#if USE_EXPERIMENTAL_SOURCE
        interface.reset(new reconstruction::KalmanInterfaceAMKalman2D());
#else
        throw std::runtime_error("SimpleReferenceCalculator: reconstructMeasurements: reconstructor type not supported by build");
#endif
    }

    estimator.init(std::move(interface));

    estimator.settings().min_chain_size = settings_.min_chain_size;
    estimator.settings().min_dt         = settings_.min_dt;
    estimator.settings().max_dt         = settings_.max_dt;
    estimator.settings().max_distance   = settings_.max_distance;

    estimator.settings().Q_var     = settings_.Q_std * settings_.Q_std;
    estimator.settings().verbosity = settings_.verbosity;

    estimator.settings().resample_dt          = settings_.resample_dt;
    estimator.settings().resample_Q_var       = settings_.resample_Q_std * settings_.resample_Q_std;
    estimator.settings().resample_interp_mode = reconstruction::StateInterpMode::BlendVar;

    estimator.settings().max_proj_distance_cart = settings_.max_proj_distance_cart;

    std::vector<kalman::KalmanUpdate> updates_new;
    kalman::KalmanUpdate update;
    size_t offs = 0;
            
    //init kalman (either from last slice's update or from new measurement)
    if (refs.init_update.has_value())
    {
        auto& mm0 = refs.measurements[ refs.start_index.value() ];
        loginf << "    initializing to update t=" << Utils::Time::toString(refs.init_update.value().t) << ", mm0 t=" << Utils::Time::toString(mm0.t);

        //init kalman from last slice's update
        estimator.kalmanInit(refs.init_update.value());

        //continue with first measurement
    }
    else
    {
        //reinit kalman with first measurement
        auto& mm0 = refs.measurements[ refs.start_index.value() ];
        loginf << "    initializing to mm t=" << Utils::Time::toString(mm0.t);

        estimator.kalmanInit(update, mm0);

        assert(update.valid);

        updates_new.push_back(update);

        ++offs; //continue with second measurement
    }

    for (const auto& update : refs.updates)
        assert(update.valid);

    //add new measurements to kalman and collect updates
    for (size_t i = refs.start_index.value() + offs; i < refs.measurements.size(); ++i)
    {
        //loginf << "    adding mm" << i << ", t = " << Utils::Time::toString(refs.measurements[ i ].t) << ", current time = " << Utils::Time::toString(estimator.currentTime());

        estimator.kalmanStep(update, refs.measurements[ i ]);

        assert(update.valid);

        updates_new.push_back(update);
    }

    loginf << "    #new updates (initial): " << updates_new.size();

    //run rts smoothing?
    if (settings_.smooth_rts)
    {
        //jointly smooth old + new updates RTS
        std::vector<kalman::KalmanUpdate> updates_joint = refs.updates;
        updates_joint.insert(updates_joint.end(), updates_new.begin(), updates_new.end());

        size_t i = 0;
        for (const auto& update : updates_joint)
        {
            if (update.state.F.size() == 0)
                loginf << "   F is null @" << i << "(" << (i < refs.updates.size() ? "old" : "new") << ")";
            if (update.state.P.size() == 0)
                loginf << "   P is null @" << i << "(" << (i < refs.updates.size() ? "old" : "new") << ")";
            if (update.state.Q.size() == 0)
                loginf << "   Q is null @" << i << "(" << (i < refs.updates.size() ? "old" : "new") << ")";
            if (update.state.x.size() == 0)
                loginf << "   x is null @" << i << "(" << (i < refs.updates.size() ? "old" : "new") << ")";

            ++i;
        }

        estimator.smoothUpdates(updates_joint);

        //retrieve new RTS updates
        size_t offs = refs.updates.size();
        for (size_t i = 0; i < updates_new.size(); ++i)
            updates_new[ i ] = updates_joint[ offs + i ];

        for (const auto& update : updates_new)
            assert(update.valid);
    }

    loginf << "    #new updates (smoothed): " << updates_new.size();

    //resample?
    if (settings_.resample_result)
    {
        //add last update of old slice if available
        if (refs.init_update.has_value())
            updates_new.insert(updates_new.begin(), refs.init_update.value());
        
        //interpolate measurements
        std::vector<kalman::KalmanUpdate> updates_interp;
        estimator.interpUpdates(updates_interp, updates_new);

        updates_new = updates_interp;
    }

    loginf << "    #new updates (resampled): " << updates_new.size();

    //join old and new measurements
    const auto ThresJoin = getJoinThreshold();

    refs.updates.reserve(refs.updates.size() + updates_new.size());
    for (size_t i = 0; i < updates_new.size(); ++i)
        if (reconstructor_.first_slice_ || updates_new[ i ].t >= ThresJoin)
            refs.updates.push_back(updates_new[ i ]);

    for (const auto& update : refs.updates)
        assert(update.valid);

    refs.updates.shrink_to_fit();

    loginf << "    #updates final: " << refs.updates.size();

    //generate references
    estimator.storeUpdates(refs.references, refs.updates);

    loginf << "    #references final: " << refs.references.size();

    if (refs.references.size() > 0)
        loginf << "    timeframe: " << Utils::Time::toString(refs.references.begin()->t) + " - "
                                    << Utils::Time::toString(refs.references.rbegin()->t);
}

/**
*/
void SimpleReferenceCalculator::updateReferences()
{
    for (auto& ref : references_)
    {
        assert(reconstructor_.targets_.count(ref.first));

        auto& target = reconstructor_.targets_.at(ref.first);
        target.references_ = std::move(ref.second.references);

        loginf << "UTN " << ref.first << ": " << target.references_.size();
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

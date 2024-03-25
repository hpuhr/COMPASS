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

#include "util/timeconv.h"
#include "util/number.h"
#include "targetreportchain.h"
#include "viewpointgenerator.h"
#include "spline_interpolator.h"
#include "simplereconstructor.h"
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
bool SimpleReferenceCalculator::computeReferences()
{
    reset();
    generateMeasurements();
    reconstructMeasurements();
    

    return true;
}

/**
*/
void SimpleReferenceCalculator::reset()
{
    measurements_.clear();
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
                generateLineMeasurements(target, dbcontent_targets.first, ds_targets.first, line_targets.first, line_targets.second);

}

/**
*/
void SimpleReferenceCalculator::generateLineMeasurements(const dbContent::ReconstructorTarget& target,
                                                         unsigned int dbcontent_id,
                                                         unsigned int sensor_id,
                                                         unsigned int line_id,
                                                         const std::multimap<boost::posix_time::ptime, unsigned long>& target_reports)
{
    std::vector<reconstruction::Measurement> line_measurements;

    //@TODO: collect measurements

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
    auto& utn_mms = measurements_[ utn ];

    if (!measurements.empty())
        utn_mms.insert(utn_mms.end(), measurements.begin(), measurements.end());
}

/**
*/
void SimpleReferenceCalculator::preprocessMeasurements(unsigned int dbcontent_id, 
                                                       Measurements& measurements)
{
    //interpolate if options are set for dbcontent
    if (settings_.interp_options.count(dbcontent_id))
        interpolateMeasurements(measurements, settings_.interp_options.at(dbcontent_id));
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
    if (measurements_.empty())
        return;

    

    std::vector<CalcRefJob> jobs;

    //collect jobs
    for (auto& mms : measurements_)
    {
        CalcRefJob job;
        job.utn          = mms.first;
        job.measurements = &mms.second;

        jobs.push_back(job);
    }

    unsigned int num_targets = jobs.size();

    //compute references in parallel
    tbb::parallel_for(uint(0), num_targets, [&](unsigned int tgt_cnt)
    {
        reconstructMeasurements(jobs[ tgt_cnt ]);
    });
}

/**
*/
void SimpleReferenceCalculator::reconstructMeasurements(CalcRefJob& job)
{
    assert(job.measurements);

    //sort measurements by timestamp
    std::sort(job.measurements->begin(), job.measurements->end(), mmSortPred);

    // 1) create kalman variant
    // 2) init kalman to last slice end (if available)
    // 3) run kalman and get updates
    // 4) store updates


}


















#if 0

/**
*/
void Reconstructor::addChain(const dbContent::TargetReport::Chain* tr_chain, const std::string& dbcontent)
{
    assert(tr_chain);

    uint32_t source_id = source_cnt_++;
    sources_[source_id] = tr_chain;

    source_uncerts_.push_back({}); //add empty uncertainty

    DBContentInfo& info = dbcontent_infos_[dbcontent];

    //if uncertainty is stored for dbcontent => assign
    if (info.uncert.has_value())
        source_uncerts_.back() = info.uncert.value();

    const auto& indices = tr_chain->timestampIndexes();

    info.count += indices.size();

    std::vector<Measurement> mms;
    mms.reserve(indices.size());

    for (const auto& index : indices)
    {
        Measurement mm;
        mm.source_id = source_id;
        mm.t         = index.first;

        boost::optional<dbContent::TargetPosition>         pos;
        boost::optional<dbContent::TargetVelocity>         speed;
        boost::optional<dbContent::TargetPositionAccuracy> accuracy_pos;
        boost::optional<dbContent::TargetVelocityAccuracy> accuracy_vel;

        if (tr_chain->ignorePosition(index))
            continue;

        pos = tr_chain->pos(index);

        mm.lat = pos->latitude_;
        mm.lon = pos->longitude_;
        
        //kepp track of min/max height
        if (pos->has_altitude_ && (!min_height_.has_value() || pos->altitude_ < min_height_.value()))
            min_height_ = pos->altitude_;
        if (pos->has_altitude_ && (!max_height_.has_value() || pos->altitude_ > max_height_.value()))
            max_height_ = pos->altitude_;

        speed        = tr_chain->speed(index);
        accuracy_pos = tr_chain->posAccuracy(index);
        accuracy_vel = tr_chain->speedAccuracy(index);

        if (speed.has_value())
        {
            auto speed_vec = Utils::Number::speedAngle2SpeedVec(speed->speed_, speed->track_angle_);

            mm.vx = speed_vec.first;
            mm.vy = speed_vec.second;
            //@TODO: vz?
        }

        //@TODO: acceleration?

        if (accuracy_pos.has_value())
        {
            mm.x_stddev = accuracy_pos->x_stddev_;
            mm.y_stddev = accuracy_pos->y_stddev_;
            mm.xy_cov   = accuracy_pos->xy_cov_;
        }

        if (accuracy_vel.has_value())
        {
            mm.vx_stddev = accuracy_vel->vx_stddev_;
            mm.vy_stddev = accuracy_vel->vy_stddev_;
        }

        mms.push_back(mm);
    }

    //resample input data?
    if (info.interp_options.has_value())
    {
        //loginf << "Interpolating measurements of dbcontent " << dbcontent;
        interpolateMeasurements(mms, info.interp_options.value());
    }

    if (!mms.empty())
        measurements_.insert(measurements_.end(), mms.begin(), mms.end());
}


/**
*/
void Reconstructor::postprocessReferences(std::vector<Reference>& references)
{
    //nothing to do at the moment
}

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
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

#include "reconstructor_kalman.h"
#include "kalman.h"
#include "logger.h"
#include "spline_interpolator.h"
#include "util/timeconv.h"
#include "viewpointgenerator.h"

namespace reconstruction
{

/**
*/
void ReconstructorKalman::init()
{
    //reset chains
    chains_.clear();
    chain_cur_ = {};

    //cache some values
    Q_var_      = base_config_.Q_std      * base_config_.Q_std;
    R_var_      = base_config_.R_std      * base_config_.R_std;
    R_var_high_ = base_config_.R_std_high * base_config_.R_std_high;
    P_var_      = base_config_.P_std      * base_config_.P_std;
    P_var_high_ = base_config_.P_std_high * base_config_.P_std_high;

    min_chain_size_ = std::max((size_t)1, base_config_.min_chain_size);

    init_impl(); //invoke derived impl
}

/**
*/
bool ReconstructorKalman::smoothChain(KalmanChain& chain)
{
    if (chain.references.empty())
    {
        //loginf << "no references to smooth...";
        return true;
    }
    if (chain.references.size() != chain.rts_infos.size())
    {
        //loginf << "not enough RTS infos: " << chain.rts_infos.size() << "/" << chain.references.size();
        return false;
    }

    if (verbosity() > 1)
    {
        loginf << "Obtains " << chain.rts_infos.size() << " info(s)";

        int cnt = 0;
        for (const auto& rts_info : chain.rts_infos)
        {
            loginf << "Info " << cnt++ << "\n"
                   << "    x: " << rts_info.x << "\n"
                   << "    P: " << rts_info.P << "\n"
                   << "    Q: " << rts_info.Q << "\n"
                   << "    F: " << rts_info.F << "\n";
        }
    }

    //loginf << "Smoothing chain...";

    std::vector<kalman::Vector> x_smooth;
    std::vector<kalman::Matrix> P_smooth;

    if (!kalman::KalmanFilter::rtsSmoother(x_smooth, P_smooth, chain.rts_infos, baseConfig().smooth_scale))
        return false;

    //update chain data
    for (size_t i = 0; i < chain.references.size(); ++i)
    {
        storeState(chain.references[ i ], kalman::KalmanState(x_smooth[ i ], P_smooth[ i ]));

        chain.rts_infos[ i ].P = P_smooth[ i ];
        chain.rts_infos[ i ].x = x_smooth[ i ];
    }
    
    return true;
}

/**
*/
void ReconstructorKalman::resampleResult(KalmanChain& result_chain, double dt_sec)
{
    if (result_chain.references.size() < 2)
        return;

    size_t n = result_chain.references.size();

    size_t n_estim = SplineInterpolator::estimatedSamples(result_chain.references.front(), 
                                                          result_chain.references.back(),
                                                          dt_sec);
    KalmanChain resampled_chain;
    resampled_chain.reserve(n_estim);

    resampled_chain.add(result_chain.references.front(), 
                        result_chain.rts_infos.front());

    boost::posix_time::milliseconds time_incr((int)(dt_sec * 1000.0));

    auto tcur = result_chain.references.front().t + time_incr;

    size_t errors          = 0;
    size_t small_intervals = 0;

    for (size_t i = 1; i < n; ++i)
    {
        const auto& ref0   = result_chain.references[i - 1];
        const auto& ref1   = result_chain.references[i    ];
        const auto& state0 = result_chain.rts_infos [i - 1];
        const auto& state1 = result_chain.rts_infos [i    ];

        auto t0 = ref0.t;
        auto t1 = ref1.t;

        if (tcur < t0 || tcur >= t1)
            continue;

        while (tcur >= t0 && tcur < t1)
        {
            double dt = Utils::Time::partialSeconds(tcur - t0);
            if (dt <= base_config_.min_dt)
            {
                //use first ref if timestep from first ref is very small
                resampled_chain.add(ref0, state0);
                ++small_intervals;
                tcur = t0 + time_incr;
                continue;
            }

            auto new_state = interpStep(state0, state1, dt);

            Reference ref;
            ref.t            = tcur;
            ref.source_id    = ref0.source_id;
            ref.noaccel_pos  = ref0.noaccel_pos;
            ref.nospeed_pos  = ref0.nospeed_pos;
            ref.nostddev_pos = ref0.nostddev_pos;

            storeState(ref, new_state);

            resampled_chain.add(ref, new_state);

            tcur += time_incr;
        }
    }

    if (resampled_chain.references.size() >= 2)
    {
        resampled_chain.references.pop_back();
        resampled_chain.rts_infos.pop_back();
        resampled_chain.add(result_chain.references.back(), 
                            result_chain.rts_infos.back());
    }

    result_chain = std::move(resampled_chain);
}

/**
*/
boost::optional<std::vector<Reference>> ReconstructorKalman::finalize()
{
    KalmanChain result_chain;

    //collect (optionally smoothed) results
    for (auto& c : chains_)
    {
        //does chain obtain enough references?
        if (c.references.size() < min_chain_size_)
        {
            if (verbosity() > 0)
                loginf << "Dropping chain of " << c.references.size() << " point(s)";
            
            continue;
        }
            
        if (verbosity() > 0)
            loginf << "Adding chain of " << c.references.size() << " point(s)";

        //apply RTS smoother?
        if (base_config_.smooth)
        {
            if (!smoothChain(c))
            {
                logerr << "ReconstructorKalman::finalize(): RTS smoother failed";
                return {};
            }
        }

        if (base_config_.resample_result)
        {
            resampleResult(c, base_config_.resample_dt);
        }

        //add to result chain
        result_chain.add(c);
    }

    //interpolate result if desired
    //if (base_config_.resample_result)
    //{
        // if (hasViewPoint())
        // {
        //     auto anno = viewPoint()->annotations().addAnnotation("positions_pre_interp");

        //     ViewPointGenFeaturePoints* points = new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Square);
        //     points->reserve(result_chain.references.size(), false);
        //     for (size_t i = 0; i < result_chain.references.size(); ++i)
        //     {
        //         double x = result_chain.references[ i ].x;
        //         double y = result_chain.references[ i ].y;
        //         points->addPoint(transformBack(x, y));
        //     }
        //     points->setColor(QColor(255, 125, 0));

        //     anno->addFeature(std::unique_ptr<ViewPointGenFeaturePoints>(points));
        // }

      //  if (verbosity() > 0)
       //     loginf << "Resampling result chain containing " << result_chain.references.size() << " reference(s)"; 
    //}
    
    return result_chain.references;
}

/**
*/
const Reference& ReconstructorKalman::lastReference() const
{
    return chain_cur_.references.back();
}

/**
*/
void ReconstructorKalman::newChain()
{
    if (!chain_cur_.empty())
        chains_.push_back(chain_cur_);

    chain_cur_ = {};
}

/**
*/
void ReconstructorKalman::addReference(const Reference& ref,
                                       const kalman::KalmanState& rts_info)
{
    chain_cur_.references.push_back(ref);

    if (base_config_.smooth)
        chain_cur_.rts_infos.push_back(rts_info);
}

/**
*/
void ReconstructorKalman::storeState(Reference& ref,
                                     const kalman::KalmanState& state) const
{
    storeState_impl(ref, state); //invoke derived impl

    ref.cov = state.P;
}

/**
*/
Reference ReconstructorKalman::storeState(const kalman::KalmanState& state,
                                          const Measurement& mm) const
{
    Reference ref;
    ref.source_id    =  mm.source_id;
    ref.t            =  mm.t;
    ref.nostddev_pos = !mm.hasStdDevPosition();

    storeState(ref, state);
    
    return ref;
}

/**
*/
bool ReconstructorKalman::needsReinit(const Reference& ref, const Measurement& mm) const
{
    if (base_config_.max_distance > 0)
    {
        double d = distance(ref, mm, CoordSystem::Cart);
        if (d > base_config_.max_distance)
            return true;
    }

    if (base_config_.max_dt > 0)
    {
        double dt = Reconstructor::timestep(ref, mm);
        if (dt > base_config_.max_dt)
            return true;
    }

    return false;
}

/**
*/
void ReconstructorKalman::init(const Measurement& mm)
{
    //start new chain
    newChain();

    //reinit kalman state
    init_impl(mm); //invoke derived impl

    //add first state
    kalman::KalmanState state = kalmanState();
    Reference           ref   = storeState(state, mm);

    ref.reset_pos   = true;
    ref.nospeed_pos = mm.hasVelocity();
    ref.noaccel_pos = mm.hasAcceleration();

    addReference(ref, state);
}

/**
*/
bool ReconstructorKalman::reinitIfNeeded(const Measurement& mm, const std::string& data_info)
{
    const auto& last_ref = lastReference();

    if (needsReinit(last_ref, mm))
    {
        if (verbosity() > 0)
            loginf << data_info << ": Reinitializing kalman filter at t = " << mm.t;

        init(mm);
        return true;
    }

    return false;
}

/**
*/
double ReconstructorKalman::timestep(const Measurement& mm) const
{
    return Reconstructor::timestep(lastReference(), mm);
}

/**
*/
reconstruction::Uncertainty ReconstructorKalman::defaultUncertaintyOfMeasurement(const Measurement& mm) const
{
    //init to standard values
    reconstruction::Uncertainty uncert;
    uncert.pos_var   = rVar();
    uncert.speed_var = rVar();
    uncert.acc_var   = rVar();

    //try to get uncertainty for source
    const auto& source_uncert = sourceUncertainty(mm.source_id);
    if (source_uncert)
        uncert = source_uncert.value();

    //set to high uncertainty if value is missing (pos is always available)
    if (!mm.hasVelocity())
        uncert.speed_var = rVarHigh();
    if (!mm.hasAcceleration())
        uncert.acc_var = rVarHigh();

    return uncert;
}

/**
*/
boost::optional<std::vector<Reference>> ReconstructorKalman::reconstruct_impl(const std::vector<Measurement>& measurements,
                                                                              const std::string& data_info)
{
    init();

    if (measurements.size() < min_chain_size_)
        return {};

    //init kalman using first target report
    const auto& mm0 = measurements.at(0);
    init(mm0);

    size_t n = measurements.size();

    for (size_t i = 1; i < n; ++i)
    {
        const auto& mm = measurements[ i ];

        //reinit?
        if (reinitIfNeeded(mm, data_info))
            continue;

        double dt = timestep(mm);
        if (dt <= base_config_.min_dt)
        {
            if (verbosity() > 0)
                loginf << data_info << ": Skipping very small timestep of " << dt << " @ mm=" << i << " t=" << mm.t;
            
            continue;
        }

        //do kalman step
        auto state = kalmanStep(dt, mm);
        if (!state.has_value())
        {
            //@TODO: what to do?
            logerr << data_info << ": Kalman step failed @ mm=" << i << " t=" << mm.t;
            return {};
        }

        //store new state
        auto ref = storeState(state.value(), mm);

        addReference(ref, state.value());
    }

    //add last uncollected chain
    newChain();

    //finalize and check result
    auto result = finalize();
    if (!result.has_value() || result->size() < min_chain_size_)
        return {};

    return result;
}

} // namespace reconstruction

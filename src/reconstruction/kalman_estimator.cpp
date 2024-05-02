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

#include "kalman_estimator.h"
#include "kalman_projection.h"
#include "kalman_interface.h"

#include "targetreportdefs.h"

#include "timeconv.h"
#include "spline_interpolator.h"

#include "logger.h"

#include "kalman_interface_umkalman2d.h"
#if USE_EXPERIMENTAL_SOURCE
#include "kalman_interface_amkalman2d.h"
#endif

namespace reconstruction
{

const double KalmanEstimator::HighStdDev = 1000.0;
const double KalmanEstimator::HighVar    = KalmanEstimator::HighStdDev * KalmanEstimator::HighStdDev;

/**
*/
KalmanEstimator::KalmanEstimator()
:   proj_handler_(new KalmanProjectionHandler)
{
}

/**
*/
KalmanEstimator::~KalmanEstimator() = default;

/**
*/
bool KalmanEstimator::isInit() const
{
    return (kalman_interface_ != nullptr);
}

/**
 * Initializes the estimator based on its current settings and the passed interface.
*/
void KalmanEstimator::init(std::unique_ptr<KalmanInterface>&& interface)
{
    assert(interface);
    
    kalman_interface_ = std::move(interface);
    assert(kalman_interface_);

    kalman_interface_->setVerbosity(settings_.verbosity);
    kalman_interface_->init();
    
    proj_handler_->settings().proj_max_dist_cart_sqr = settings_.max_proj_distance_cart * settings_.max_proj_distance_cart;

    max_distance_sqr_ = settings_.max_distance * settings_.max_distance;
}

/**
*/
std::unique_ptr<KalmanInterface> KalmanEstimator::createInterface(kalman::KalmanType ktype,
                                                                  bool track_velocity, 
                                                                  bool track_accel)
{
    if (ktype == kalman::KalmanType::UMKalman2D)
    {
        return std::unique_ptr<KalmanInterface>(new reconstruction::KalmanInterfaceUMKalman2D(track_velocity));
    }
    else if (ktype == kalman::KalmanType::AMKalman2D)
    {
#if USE_EXPERIMENTAL_SOURCE
        return std::unique_ptr<KalmanInterface>(new reconstruction::KalmanInterfaceAMKalman2D());
#else
        throw std::runtime_error("KalmanEstimator: createInterface: reconstructor type not supported by build");
#endif
    }
    else
    {
        throw std::runtime_error("KalmanEstimator: createInterface: unknown kalman type");
    }

    return {};
}

/**
 * Initializes the estimator based on its current settings and the passed kalman variant.
 */
void KalmanEstimator::init(kalman::KalmanType ktype)
{
    auto interface = KalmanEstimator::createInterface(ktype, settings_.track_velocities, settings_.track_accelerations);
    init(std::move(interface));
}

/**
*/
const boost::posix_time::ptime& KalmanEstimator::currentTime() const
{
    assert(isInit());
    return kalman_interface_->currrentTime();
}

/**
*/
void KalmanEstimator::storeUpdate(Reference& ref, 
                                  const kalman::KalmanUpdate& update,
                                  KalmanProjectionHandler& phandler) const
{
    assert(isInit());

    kalman_interface_->storeState(ref, update.state);

    ref.t   = update.t;
    ref.cov = update.state.P;

    //unproject to lat/lon
    phandler.unproject(ref.lat, ref.lon, ref.x, ref.y, &update.projection_center);

    //loginf << "KalmanEstimator: storeUpdate: (" << update.projection_center.x() << "," << update.projection_center.y() << ") "
    //                                            << ref.x << "," << ref.y << " => " << ref.lat << "," << ref.lon;
}

/**
*/
void KalmanEstimator::storeUpdates(std::vector<Reference>& refs,
                                   const std::vector<kalman::KalmanUpdate>& updates) const
{
    KalmanProjectionHandler phandler;

    size_t n = updates.size();

    refs.resize(n);

    for (size_t i = 0; i < n; ++i)
        storeUpdate(refs[ i ], updates[ i ], phandler);
}

/**
 * Inits the measurement and its respective update.
*/
void KalmanEstimator::initMeasurement(kalman::KalmanUpdate& update,
                                      Measurement& mm)
{
    //reset update flags
    update.resetFlags();

    //store info
    update.t = mm.t;

    //project measurement to cartesian
    proj_handler_->project(mm.x, mm.y, mm.lat, mm.lon);
}

/**
 * Init kalman from measurement.
 */
void KalmanEstimator::kalmanInit(kalman::KalmanUpdate& update,
                                 Measurement& mm)
{
    assert(isInit());

    //init projection
    proj_handler_->initProjection(mm.lat, mm.lon);
    
    //init measurement
    initMeasurement(update, mm);

    //reinit kalman
    reinit(update, mm);

    update.projection_center = proj_handler_->projectionCenter();
    update.valid             = true;
}

/**
 * Init kalman from update.
 */
void KalmanEstimator::kalmanInit(const kalman::KalmanUpdate& update)
{
    assert(isInit());
    assert(update.valid);

    //init projection
    proj_handler_->initProjection(update.projection_center.x(), update.projection_center.y());

    //reinit kalman from update
    kalman_interface_->kalmanInit(update.state, update.t);
}

/**
 * Checks if the measurement triggers a reinitialization of the kalman filter based on different criteria.
*/
KalmanEstimator::ReinitState KalmanEstimator::needsReinit(const Measurement& mm) const
{
    if ((settings_.reinit_check_flags & Settings::ReinitFlags::ReinitCheckDistance) && settings_.max_distance > 0)
    {
        const double d_sqr = kalman_interface_->distanceSqr(mm);
        if (d_sqr > max_distance_sqr_)
            return KalmanEstimator::ReinitState::ReinitDistance;
    }

    if ((settings_.reinit_check_flags & Settings::ReinitFlags::ReinitCheckTime) && settings_.max_dt > 0)
    {
        const double dt = kalman_interface_->timestep(mm);
        if (dt > settings_.max_dt)
            return KalmanEstimator::ReinitState::ReinitTime;
    }

    return KalmanEstimator::ReinitState::ReinitNotNeeded;
}

/**
*/
reconstruction::Uncertainty KalmanEstimator::defaultUncert(const Measurement& mm) const
{
    reconstruction::Uncertainty uncert = settings_.default_uncert;

    //set to high uncertainty if value is missing (pos is always available)
    if (!mm.hasVelocity())
        uncert.speed_var = settings_.R_var_undef;
    if (!mm.hasAcceleration())
        uncert.acc_var = settings_.R_var_undef;

    return uncert;
}

/**
 * Reinitializes the kalman filter and marks the update.
*/
void KalmanEstimator::reinit(kalman::KalmanUpdate& update,
                             const Measurement& mm)
{
    if (settings_.verbosity > 0)
        loginf << "KalmanEstimator: reinit: Reinitializing kalman filter at t = " << mm.t;

    //reinit kalman state
    kalman_interface_->kalmanInit(update.state, mm, defaultUncert(mm), settings_.Q_var);

    update.reinit = true;
}

/**
 * Executes a kalman step.
*/
bool KalmanEstimator::step(kalman::KalmanUpdate& update,
                           const Measurement& mm)
{
    if (!kalman_interface_->kalmanStep(update.state, mm, defaultUncert(mm), settings_.Q_var))
    {
        logwrn << "KalmanEstimator: step: Kalman step failed @ t=" << mm.t;
        return false;
    }

    return true;
}

/**
 * Change projection center if needed and keep kalman state coordinates up-to-date with projection.
*/
void KalmanEstimator::checkProjection(kalman::KalmanUpdate& update)
{
    //change projection?
    if (proj_handler_->changeProjectionIfNeeded(update, *kalman_interface_))
    {
        //projection changed => update kalman state to reprojected position
        kalman_interface_->stateVecX(update.state.x);

        if (settings_.verbosity > 1)
            loginf << "Changed map projection @t=" << update.t;
    }

    //store current projection to update
    update.projection_center = proj_handler_->projectionCenter();
}

/**
*/
KalmanEstimator::StepResult KalmanEstimator::kalmanStep(kalman::KalmanUpdate& update,
                                                        Measurement& mm)
{
    assert(isInit());

    //init measurement
    initMeasurement(update, mm);

    //check if timestep is too small
    auto tstep = kalman_interface_->timestep(mm);
    assert(tstep >= 0);

    if (tstep < settings_.min_dt)
    {
        logwrn << "KalmanEstimator: kalmanStep: step " << kalman_interface_->timestep(mm) << " too small (<" << settings_.min_dt << "), skipping...";
        return KalmanEstimator::StepResult::FailStepTooSmall;
    }

    //check if reinit is needed
    if (needsReinit(mm) != ReinitState::ReinitNotNeeded)
    {
        //reinit
        reinit(update, mm);
    }
    else
    {
        //normal step
        bool kalman_step_ok = step(update, mm);

        //handle failed step?
        if (!kalman_step_ok)
        {
            logwrn << "KalmanEstimator: kalmanStep: step failed";

            if (settings_.step_fail_strategy == Settings::StepFailStrategy::Reinit)
            {
                //reinit
                reinit(update, mm);
            }
            else if (settings_.step_fail_strategy == Settings::StepFailStrategy::ReturnInvalid)
            {
                //return error
                return KalmanEstimator::StepResult::FailKalmanError;
            }
            else
            {
                //assert
                assert(kalman_step_ok);
                return KalmanEstimator::StepResult::FailKalmanError;
            }
        }
    }

    //update projection if needed
    checkProjection(update);

    update.valid = true;

    return KalmanEstimator::StepResult::Success;
}

/**
*/
bool KalmanEstimator::kalmanPrediction(Measurement& mm,
                                       double dt) const
{
    assert(isInit());

    kalman::KalmanState state;
    bool kalman_prediction_ok = kalman_interface_->kalmanPrediction(state.x, state.P, dt, settings_.Q_var);

    if (!kalman_prediction_ok)
    {
        if (settings_.step_fail_strategy == Settings::StepFailStrategy::Assert)
            assert(kalman_prediction_ok);
        return false;
    }

    kalman_interface_->storeState(mm, state);
    proj_handler_->unproject(mm.lat, mm.lon, mm.x, mm.y);

    return true;
}

/**
*/
bool KalmanEstimator::kalmanPrediction(Measurement& mm,
                                       const boost::posix_time::ptime& ts) const
{
    assert(isInit());

    kalman::KalmanState state;
    bool kalman_prediction_ok = kalman_interface_->kalmanPrediction(state.x, state.P, ts, settings_.Q_var);
    
    if (!kalman_prediction_ok)
    {
        if (settings_.step_fail_strategy == Settings::StepFailStrategy::Assert)
            assert(kalman_prediction_ok);
        return false;
    }

    kalman_interface_->storeState(mm, state);
    proj_handler_->unproject(mm.lat, mm.lon, mm.x, mm.y);

    return true;
}

bool KalmanEstimator::kalmanPrediction(Measurement& mm,
                                       const kalman::KalmanUpdate& ref_update,
                                       double dt)
{
    kalmanInit(ref_update);
    return kalmanPrediction(mm, dt);


}

bool KalmanEstimator::kalmanPrediction(Measurement& mm,
                                       const kalman::KalmanUpdate& ref_update,
                                       const boost::posix_time::ptime& ts)
{
    kalmanInit(ref_update);
    return kalmanPrediction(mm, ts);
}

/**
*/
void KalmanEstimator::executeChainFunc(Updates& updates, const ChainFunc& func) const
{
    if (!func || updates.empty())
        return;

    if (updates.size() < 2)
    {
        func(updates, 0, 0);
        return;
    }

    size_t idx0 = 0;
    size_t n    = updates.size();
    for (size_t i = 1; i < n; ++i)
    {
        if (updates[ i ].reinit)
        {
            //chain ended => execute func on chain and start new one
            func(updates, idx0, i - 1);
            idx0 = i;
        }
    }

    //last chain
    func(updates, idx0, n - 1);
}

/**
*/
void KalmanEstimator::smoothUpdates(std::vector<kalman::KalmanUpdate>& updates) const
{
    assert(isInit());

    KalmanProjectionHandler phandler;

    auto func = [ & ] (std::vector<kalman::KalmanUpdate>& updates, size_t idx0, size_t idx1)
    {
        kalman_interface_->smoothUpdates(updates, idx0, idx1, phandler);
    };

    executeChainFunc(updates, func);
}

/**
*/
void KalmanEstimator::interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                                    std::vector<kalman::KalmanUpdate>& updates) const
{
    KalmanProjectionHandler phandler;

    interp_updates.clear();

    auto func = [ & ] (std::vector<kalman::KalmanUpdate>& updates, size_t idx0, size_t idx1)
    {
        std::vector<kalman::KalmanUpdate> interp_updates_chain;
        this->interpUpdates(interp_updates_chain,
                            updates, 
                            idx0, 
                            idx1, 
                            settings_.resample_dt,
                            settings_.min_dt,
                            settings_.resample_Q_var,
                            settings_.resample_interp_mode,
                            phandler);
        interp_updates.insert(interp_updates.end(), interp_updates_chain.begin(), interp_updates_chain.end());
    };

    executeChainFunc(updates, func);
}

/**
*/
bool KalmanEstimator::interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                                    const std::vector<kalman::KalmanUpdate>& updates,
                                    size_t idx0,
                                    size_t idx1,
                                    double dt_sec,
                                    double min_dt_sec,
                                    double Q_var,
                                    StateInterpMode interp_mode,
                                    KalmanProjectionHandler& proj_handler) const
{
    assert(isInit());

    interp_updates.clear();

    assert(idx1 >= idx0);

    size_t n = idx1 - idx0 + 1;

    if (n <= 1)
        return true;

    const auto& first_update = updates[ idx0 ];
    const auto& last_update  = updates[ idx1 ];

    double dt_total = Utils::Time::partialSeconds(last_update.t - first_update.t);
    size_t n_estim  = std::ceil(dt_total / dt_sec) + 2;

    interp_updates.reserve(n_estim);

    auto addUpdate = [ & ] (const kalman::KalmanState& state, 
                            const Eigen::Vector2d& proj_center, 
                            const boost::posix_time::ptime& t)
    {
        interp_updates.push_back(kalman::KalmanUpdate());
        interp_updates.back().state             = state;
        interp_updates.back().projection_center = proj_center;
        interp_updates.back().t                 = t;
        interp_updates.back().valid             = true;
        interp_updates.back().reinit            = false;
    };

    addUpdate(first_update.state, first_update.projection_center, first_update.t);

    boost::posix_time::milliseconds time_incr((int)(dt_sec * 1000.0));

    auto tcur = first_update.t + time_incr;

    size_t small_intervals = 0;

    auto x_tr_func = proj_handler.reprojectionTransform(&updates, kalman_interface_.get(), 0);

    kalman::KalmanState state1_tr;

    auto blendFunc = [ & ] (double dt0, 
                            double dt1, 
                            double dt,
                            const kalman::KalmanState& state0,
                            const kalman::KalmanState& state1,
                            StateInterpMode mode)
    {
        if (mode == StateInterpMode::BlendLinear)
        {
            return dt0 / dt;
        }
        else if (mode == StateInterpMode::BlendStdDev)
        {
            double stddev_x0 = std::sqrt(kalman_interface_->xVar(state0.P));
            double stddev_y0 = std::sqrt(kalman_interface_->yVar(state0.P));

            double stddev_x1 = std::sqrt(kalman_interface_->xVar(state1.P));
            double stddev_y1 = std::sqrt(kalman_interface_->yVar(state1.P));

            double w0 = std::max(stddev_x0, stddev_y0);
            double w1 = std::max(stddev_x1, stddev_y1);

            return w0 / (w0 + w1);
        }
        else if (mode == StateInterpMode::BlendVar)
        {
            double var_x0 = kalman_interface_->xVar(state0.P);
            double var_y0 = kalman_interface_->yVar(state0.P);

            double var_x1 = kalman_interface_->xVar(state1.P);
            double var_y1 = kalman_interface_->yVar(state1.P);

            double w0 = std::max(var_x0, var_y0);
            double w1 = std::max(var_x1, var_y1);

            return w0 / (w0 + w1);
        }

        //StateInterpMode::BlendHalf
        return 0.5;
    };

    for (size_t i = idx0 + 1; i <= idx1; ++i)
    {
        const auto& update0       = updates[ i - 1];
        const auto& update1       = updates[ i    ];
        const auto& state0        = update0.state;
        const auto& state1        = update1.state;
        const auto& proj_center0  = update0.projection_center;
        const auto& proj_center1  = update1.projection_center;

        auto t0 = update0.t;
        auto t1 = update1.t;

        if (tcur < t0 || tcur >= t1)
            continue;

        //transfer state1 to map projection of state0?
        if (x_tr_func)
        { 
            state1_tr = state1;
            x_tr_func(state1_tr.x, state1.x, i, i - 1);
        }
        const auto& state1_ref = x_tr_func ? state1_tr : state1;

        const double dt = Utils::Time::partialSeconds(t1 - t0);

        while (tcur >= t0 && tcur < t1)
        {
            double dt0 = Utils::Time::partialSeconds(tcur - t0);
            double dt1 = Utils::Time::partialSeconds(t1 - tcur);
            if (dt0 <= min_dt_sec)
            {
                //use first ref if timestep from first ref is very small
                addUpdate(state0, proj_center0, t0);

                ++small_intervals;
                tcur = t0 + time_incr;
                continue;
            }
            if (dt1 <= min_dt_sec)
            {
                //use second ref if timestep from second ref is very small
                addUpdate(state1, proj_center1, t1);

                ++small_intervals;
                tcur = t1 + time_incr;
                continue;
            }

            auto new_state0 = kalman_interface_->interpStep(state0, state1_ref,  dt0, Q_var);
            auto new_state1 = kalman_interface_->interpStep(state1_ref, state0, -dt1, Q_var);

            if (!new_state0.has_value() || !new_state1.has_value())
                return false;

            double interp_factor = blendFunc(dt0, dt1, dt, *new_state0, *new_state1, interp_mode);

            //std::cout << interp_factor << std::endl;

            kalman::KalmanState new_state;
            new_state.x = SplineInterpolator::interpStateVector(new_state0->x, new_state1->x, interp_factor);
            new_state.P = SplineInterpolator::interpCovarianceMat(new_state0->P, new_state1->P, interp_factor);

            // Reference ref;
            // ref.t              = tcur;
            // ref.source_id      = ref0.source_id;
            // ref.noaccel_pos    = ref0.noaccel_pos || ref1.noaccel_pos;
            // ref.nospeed_pos    = ref0.nospeed_pos || ref1.nospeed_pos;
            // ref.nostddev_pos   = ref0.nostddev_pos || ref1.nostddev_pos;
            // ref.mm_interp      = ref0.mm_interp || ref1.mm_interp;
            // ref.projchange_pos = ref1.projchange_pos; //if a change in map projection happened between the two reference states, 
            //                                           //this should be logged in the second reference
            // ref.ref_interp     = true;

            addUpdate(new_state, proj_center0, tcur);

            tcur += time_incr;
        }
    }

    if (settings_.verbosity >= 1 && small_intervals > 0)
        logdbg << "KalmanEstimator: interpUpdates: Encountered " << small_intervals << " small interval(s) during resampling";

    if (interp_updates.size() >= 2)
    {
        interp_updates.pop_back();
        addUpdate(last_update.state, last_update.projection_center, last_update.t);
    }

    return true;
}

} // reconstruction

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

#include "kalman_interface.h"
#include "reconstruction_defs.h"
#include "kalman_projection.h"
#include "kalman_filter.h"

#include "timeconv.h"
#include "logger.h"

namespace reconstruction
{

/**
*/
KalmanInterface::KalmanInterface(kalman::KalmanFilter* kalman_filter)
:   kalman_filter_(kalman_filter)
{
    assert(kalman_filter_);
}

/**
*/
KalmanInterface::~KalmanInterface() = default;

/**
*/
double KalmanInterface::timestep(const boost::posix_time::ptime& ts0, const boost::posix_time::ptime& ts1)
{
    return (ts1 >= ts0 ? Utils::Time::partialSeconds(ts1 - ts0) : -Utils::Time::partialSeconds(ts0 - ts1));
}

/**
*/
double KalmanInterface::timestep(const Measurement& mm) const
{
    return KalmanInterface::timestep(ts_, mm.t);
}

/**
*/
double KalmanInterface::timestep(const Measurement& mm0, const Measurement& mm1)
{
    return KalmanInterface::timestep(mm0.t, mm1.t);
}

/**
*/
double KalmanInterface::distanceSqr(const Measurement& mm) const
{
    //last cartesian position of kalman
    double x0, y0;
    xPos(x0, y0);

    return (Eigen::Vector2d(x0, y0) - Eigen::Vector2d(mm.x, mm.y)).squaredNorm();
}


/**
*/
void KalmanInterface::kalmanInit(kalman::KalmanState& init_state,
                                 const Measurement& mm,
                                 const reconstruction::Uncertainty& default_uncert,
                                 double Q_var)
{
    assert(kalman_filter_);

    ts_ = mm.t;
    
    const double dt_start = 1.0;

    kalman_filter_->settings().process_noise_var = Q_var;

    //init kalman
    stateVecXFromMM(kalman_filter_->xVec(), mm);
    covarianceMatP(kalman_filter_->pMat(), mm, default_uncert);
    //measurementUncertMatR(kalman_filter_->rMat(), mm, default_uncert);

    kalman_filter_->updateInternalMatrices(dt_start);

    if (verbosity() > 1)
    {
        loginf << "[Reinit Kalman] \n"
               << kalman_filter_->asString(kalman::InfoState);
    }

    kalman_filter_->state(init_state);
    init_state.dt = dt_start;
}

/**
*/
void KalmanInterface::kalmanInit(const kalman::KalmanState& init_state,
                                 const boost::posix_time::ptime& ts)
{
    assert(kalman_filter_);

    ts_ = ts;
    
    kalman_filter_->init(init_state);
}

/**
*/
void KalmanInterface::kalmanInit(const kalman::Vector& x,
                                 const kalman::Matrix& P,
                                 const boost::posix_time::ptime& ts)
{
    assert(kalman_filter_);

    ts_ = ts;
    
    kalman_filter_->init(x, P);
}

/**
*/
kalman::KalmanError KalmanInterface::kalmanStep(kalman::KalmanState& new_state,
                                                const Measurement& mm, 
                                                const reconstruction::Uncertainty& default_uncert, 
                                                double Q_var)
{
    assert(kalman_filter_);

    const double dt = timestep(mm);

    kalman_filter_->settings().process_noise_var = Q_var;

    //set kalman internal matrices
    measurementUncertMatR(kalman_filter_->rMat(), mm, default_uncert);

    //get measurement vector
    kalman::Vector z;
    measurementVecZ(z, mm);

    if (verbosity() > 1)
    {
        loginf << "[Kalman step] dt = " << dt << "\n" 
               << kalman_filter_->asString(kalman::InfoState)
               << "z: " << z << "\n";
    }

    auto err = kalman_filter_->predictAndUpdate(dt, z, {}, {});

    if (err != kalman::KalmanError::NoError)
        return err;

    new_state = kalman_filter_->state();
    new_state.dt = dt;

    ts_ = mm.t;

    return kalman::KalmanError::NoError;
}

/**
*/
kalman::KalmanError KalmanInterface::kalmanPrediction(kalman::Vector& x,
                                                      kalman::Matrix& P,
                                                      double dt,
                                                      double Q_var,
                                                      bool fix_estimate,
                                                      bool* fixed) const
{
    assert(kalman_filter_);

    kalman_filter_->settings().process_noise_var = Q_var;

    auto err = kalman_filter_->predictState(x, P, dt, fix_estimate, fixed, {});

    return err;
}

/**
*/
kalman::KalmanError KalmanInterface::kalmanPrediction(kalman::Vector& x,
                                                      kalman::Matrix& P,
                                                      const boost::posix_time::ptime& ts,
                                                      double Q_var,
                                                      bool fix_estimate,
                                                      bool* fixed) const
{
    double dt = KalmanInterface::timestep(ts_, ts);
    return kalmanPrediction(x, P, dt, Q_var, fix_estimate, fixed);
}

/**
*/
bool KalmanInterface::smoothUpdates(std::vector<kalman::KalmanUpdate>& updates,
                                    size_t idx0,
                                    size_t idx1,
                                    KalmanProjectionHandler& proj_handler,
                                    double smooth_scale,
                                    kalman::SmoothFailStrategy fail_strategy) const
{
    assert(kalman_filter_);
    assert(idx1 >= idx0);

    size_t n = idx1 - idx0 + 1;

    if (n <= 1)
        return true;

    std::vector<kalman::Vector> x_smooth;
    std::vector<kalman::Matrix> P_smooth;
    std::vector<bool>           state_valid_tmp;

    //loginf << "KalmanInterface: smoothUpdates: #updates: " << updates.size() << ", idx0: " << idx0 << ", idx1: " << idx1 << ", n: " << n;

    //@TODO: duplicate states really needed!?
    std::vector<kalman::KalmanState> states(n); 
    for (size_t i = idx0; i <= idx1; ++i)
    {
        states[ i - idx0 ] = updates[ i ].state;
    }

    //get reprojection transfer func
    auto x_tr = proj_handler.reprojectionTransform(&updates, this, idx0);

    //smooth states
    if (!kalman_filter_->smooth(x_smooth, 
                                P_smooth, 
                                states, 
                                x_tr, 
                                smooth_scale, 
                                fail_strategy == kalman::SmoothFailStrategy::Stop, 
                                fail_strategy == kalman::SmoothFailStrategy::SetInvalid ? &state_valid_tmp : nullptr))
    {
        return false;
    }
    
    //write smoothed states back to updates
    for (size_t i = 0; i < n; ++i)
    {
        updates[ idx0 + i ].state.x = x_smooth[ i ];
        updates[ idx0 + i ].state.P = P_smooth[ i ];

        if (fail_strategy == kalman::SmoothFailStrategy::SetInvalid && !state_valid_tmp[ i ])
            updates[ idx0 + i ].valid = false;
    }
    
    return true;
}

/**
 * Note: !Assumes that it is ok to modify the filter's internal state!
*/
kalman::KalmanError KalmanInterface::interpStep(kalman::KalmanState& state_interp,
                                                const kalman::KalmanState& state0,
                                                const kalman::KalmanState& state1,
                                                double dt,
                                                double Q_var,
                                                bool fix_estimate,
                                                bool* fixed) const
{
    assert(kalman_filter_);

    kalman_filter_->settings().process_noise_var = Q_var;

    return kalman_filter_->predictStateFrom(state_interp.x,
                                            state_interp.P,
                                            state0.x, 
                                            state0.P,
                                            dt,
                                            fix_estimate,
                                            fixed,
                                            {},
                                            &state_interp);
}

/**
*/
bool KalmanInterface::checkKalmanState(kalman::KalmanState& state) const
{
    assert(kalman_filter_);
    return kalman_filter_->checkState(state.x, state.P);
}

/**
*/
void KalmanInterface::storeState(Measurement& mm, const kalman::KalmanState& state) const
{
    storeState(mm, state.x, state.P);
}

/**
*/
Measurement KalmanInterface::currentStateAsMeasurement() const
{
    Measurement mm;
    storeState(mm, currentState());

    return mm;
}

/**
*/
kalman::KalmanState KalmanInterface::currentState() const
{
    assert(kalman_filter_);
    return kalman_filter_->state();
}

/**
*/
void KalmanInterface::stateVecX(const kalman::Vector& x)
{
    assert(kalman_filter_);
    kalman_filter_->setX(x);
}

/**
*/
void KalmanInterface::xPos(double& x, double& y) const
{
    assert(kalman_filter_);
    xPos(x, y, kalman_filter_->getX());
}

/**
*/
std::string KalmanInterface::asString(int flags, 
                                      const std::string& prefix) const
{
    assert(kalman_filter_);
    return kalman_filter_->asString(flags, prefix);
}

} // reconstruction

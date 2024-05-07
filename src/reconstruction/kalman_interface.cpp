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

#include "timeconv.h"
#include "logger.h"

namespace reconstruction
{

/**
*/
void KalmanInterface::kalmanInit(kalman::KalmanState& init_state,
                                 const Measurement& mm,
                                 const reconstruction::Uncertainty& default_uncert,
                                 double Q_var)
{
    ts_ = mm.t;
    kalmanInit_impl(init_state, mm, default_uncert, Q_var);
}

/**
*/
void KalmanInterface::kalmanInit(const kalman::KalmanState& init_state,
                                 const boost::posix_time::ptime& ts)
{
    ts_ = ts;
    kalmanInit_impl(init_state);
}

/**
*/
void KalmanInterface::kalmanInit(const kalman::Vector& x,
                                 const kalman::Matrix& P,
                                 const boost::posix_time::ptime& ts)
{
    ts_ = ts;
    kalmanInit_impl(x, P);
}

/**
*/
bool KalmanInterface::kalmanStep(kalman::KalmanState& new_state,
                                 const Measurement& mm, 
                                 const reconstruction::Uncertainty& default_uncert, 
                                 double Q_var)
{
    bool ok = kalmanStep_impl(new_state, timestep(mm), mm, default_uncert, Q_var);

    ts_ = mm.t;

    return ok;
}

/**
*/
bool KalmanInterface::kalmanPrediction(kalman::Vector& x,
                                       kalman::Matrix& P,
                                       double dt,
                                       double Q_var) const
{
    return kalmanPrediction_impl(x, P, dt, Q_var);
}

/**
*/
bool KalmanInterface::kalmanPrediction(kalman::Vector& x,
                                       kalman::Matrix& P,
                                       const boost::posix_time::ptime& ts,
                                       double Q_var) const
{
    double dt = Utils::Time::partialSeconds(ts - ts_);
    return kalmanPrediction_impl(x, P, dt, Q_var);
}

/**
*/
double KalmanInterface::timestep(const Measurement& mm) const
{
    return Utils::Time::partialSeconds(mm.t - ts_);
}

/**
*/
double KalmanInterface::timestep(const Measurement& mm0, const Measurement& mm1)
{
    return Utils::Time::partialSeconds(mm1.t - mm0.t);
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
kalman::Vector KalmanInterface::stateVecXInv(const kalman::Vector& x) const
{
    kalman::Vector x_inv;
    stateVecXInv(x_inv, x);
    return x;
}

/**
*/
bool KalmanInterface::smoothUpdates(std::vector<kalman::KalmanUpdate>& updates,
                                    size_t idx0,
                                    size_t idx1,
                                    KalmanProjectionHandler& proj_handler) const
{
    assert(idx1 >= idx0);

    size_t n = idx1 - idx0 + 1;

    if (n <= 1)
        return true;

    std::vector<kalman::Vector> x_smooth;
    std::vector<kalman::Matrix> P_smooth;

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
    if (!smoothUpdates_impl(x_smooth, P_smooth, states, x_tr))
        return false;

    //write smoothed states back to updates
    for (size_t i = 0; i < n; ++i)
    {
        updates[ idx0 + i ].state.x = x_smooth[ i ];
        updates[ idx0 + i ].state.P = P_smooth[ i ];
    }

    return true;
}

/**
*/
void KalmanInterface::storeState(Measurement& mm, const kalman::KalmanState& state) const
{
    storeState(mm, state.x, state.P);
}

} // reconstruction

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

#pragma once

#include <Eigen/Core>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace kalman
{

enum KalmanType
{
    UMKalman2D = 0, // uniform motion kalman in the plane
    AMKalman2D      // accelerated motion kalman in the plane
};

typedef Eigen::MatrixXd         Matrix;
typedef Eigen::VectorXd         Vector;
typedef boost::optional<Matrix> OMatrix;
typedef boost::optional<Vector> OVector;

/**
 * Transfer func for a state vector.
 * Used to transfer the given state vector between two collected kalman states.
 * This functional is kept very general on purpose and can be used e.g. to convert from
 * one map projection system to another one.
 * params: state vec transformed | state vec | index of the old kalman state | index of the new kalman state
 */
typedef std::function<void(Vector&,const Vector&,size_t,size_t)> XTransferFunc;

/**
*/
struct KalmanState
{
    KalmanState() {}
    KalmanState(const Vector& _x, const Matrix& _P) : x(_x), P(_P) {}
    KalmanState(const Vector& _x, const Matrix& _P, double _dt) : dt(_dt), x(_x), P(_P) {}

    double dt = 0.0; // used timestep
    Vector x;        // state
    Matrix P;        // state uncertainty
    Matrix F;        // transition mat
    Matrix Q;        // process noise
};

/**
 * Low-level struct for a kalman update.
 * Contains everything needed for postprocessing and extraction of a final reference.
 */
struct KalmanUpdate
{
    void resetFlags()
    {
        valid  = false;
        reinit = false;
    }

    KalmanState              state;             // kalman internal state, can be used for rts smooting, state interpolation, etc.
    Eigen::Vector2d          projection_center; // center of the local stereographic projection used for this update
    boost::posix_time::ptime t;                 // time of update

    bool valid  = false; // kalman update is valid
    bool reinit = false; // kalman was reinitialized at this update, represents the beginning of a new kalman chain
};

} // namespace kalman

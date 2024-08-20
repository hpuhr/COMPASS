
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

#include "kalman_filter_um2d.h"
#include "logger.h"

#include <Eigen/Dense>

namespace kalman
{

/**
*/
KalmanFilterUM2D::KalmanFilterUM2D(bool track_velocities)
:   KalmanFilterLinear(4, track_velocities ? 4 : 2, 0)
{
    //configure H
    if (track_velocities)
    {
        H_.setIdentity(4, 4);
    }
    else
    {
        H_.setZero(2, 4);
        H_(0, 0) = 1.0;
        H_(1, 2) = 1.0;
    }

    auto F_func  = [ this ] (Matrix& F, double dt) { return this->configureFMat(F, dt); };
    auto Q_func  = [ this ] (Matrix& Q, double dt, double Q_var) { return this->configureQMat(Q, dt, Q_var); };
    auto inv_func = [ this ] (Vector& x) { this->invertState(x); };

    setFMatFunc(F_func);
    setQMatFunc(Q_func);
    
    setInvertStateFunc(inv_func);
}

/**
*/
KalmanFilterUM2D::~KalmanFilterUM2D() = default;

/**
*/
void KalmanFilterUM2D::configureFMat(Matrix& F, double dt) const
{
    F.setIdentity(4, 4);

    F(0, 1) = dt;
    F(2, 3) = dt;
}

/**
*/
void KalmanFilterUM2D::configureQMat(Matrix& Q, double dt, double Q_var) const
{
    KalmanFilter::continuousWhiteNoise(Q, 2, dt, Q_var, 2);
}

/**
*/
void KalmanFilterUM2D::invertState(Vector& x) const
{
    x[ 1 ] *= -1.0;
    x[ 3 ] *= -1.0;
}

} // namespace kalman

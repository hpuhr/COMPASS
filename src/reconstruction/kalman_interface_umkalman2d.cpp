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

#include "kalman_interface_umkalman2d.h"

#include "kalman.h"
#include "reconstructor_defs.h"

namespace reconstruction
{

/**
*/
KalmanInterfaceUMKalman2D::KalmanInterfaceUMKalman2D(bool track_velocities) 
:   track_velocities_(track_velocities) 
{
}

/**
*/
KalmanInterface* KalmanInterfaceUMKalman2D::clone() const
{
    return new KalmanInterfaceUMKalman2D(track_velocities_);
}

/**
*/
size_t KalmanInterfaceUMKalman2D::dimX() const
{
    return 4;
}

/**
*/
size_t KalmanInterfaceUMKalman2D::dimZ() const
{
    return (track_velocities_ ? 4 : 2);
}

/**
*/
size_t KalmanInterfaceUMKalman2D::dimU() const
{
    return 0;
}

/**
*/
void KalmanInterfaceUMKalman2D::stateVecXFromMM(kalman::Vector& x, const Measurement& mm) const
{
    x.setZero(4);

    xPos(x, mm.x, mm.y);

    //start with zero velocity

    if (mm.hasVelocity())
    {
        x[ 1 ] = mm.vx.value();
        x[ 3 ] = mm.vy.value();
    }
}

/**
*/
void KalmanInterfaceUMKalman2D::measurementVecZ(kalman::Vector& z, const Measurement& mm) const
{
    if (track_velocities_)
    {
        z.setZero(4);
        z[ 0 ] = mm.x;
        z[ 2 ] = mm.y;

        if (mm.hasVelocity())
        {
            z[ 1 ] = mm.vx.value();
            z[ 3 ] = mm.vy.value();
        }
    }
    else
    {
        z.setZero(2);
        z[ 0 ] = mm.x;
        z[ 1 ] = mm.y;
    }
}

/**
*/
 void KalmanInterfaceUMKalman2D::covarianceMatP(kalman::Matrix& P,
                                                const Measurement& mm, 
                                                const reconstruction::Uncertainty& default_uncert) const
{
    P.setZero(4, 4);

    double var_x  = default_uncert.pos_var;
    double var_y  = default_uncert.pos_var;
    double cov_xy = 0.0;

    if (mm.hasStdDevPosition())
    {
        var_x = mm.x_stddev.value() * mm.x_stddev.value();
        var_y = mm.y_stddev.value() * mm.y_stddev.value();

        if (mm.xy_cov.has_value())
            cov_xy = mm.xy_cov.value();
    }

    double var_vx = default_uncert.speed_var;
    double var_vy = default_uncert.speed_var;

    if (mm.hasVelocity() && mm.hasStdDevVelocity())
    {
        var_vx = mm.vx_stddev.value() * mm.vx_stddev.value();
        var_vy = mm.vy_stddev.value() * mm.vy_stddev.value();
    }

    P(0, 0) = var_x;
    P(1, 1) = var_vx;
    P(2, 2) = var_y;
    P(3, 3) = var_vy;
    P(0, 2) = cov_xy;
    P(2, 0) = cov_xy;
}

/**
*/
void KalmanInterfaceUMKalman2D::processUncertMatQ(kalman::Matrix& Q, double dt, double Q_var) const
{
    kalman::KalmanFilter::continuousWhiteNoise(Q, 2, dt, Q_var, 2);
}

/**
*/
void KalmanInterfaceUMKalman2D::measurementMatH(kalman::Matrix& H) const
{
    if (track_velocities_)
    {
        H.setIdentity(4, 4);
    }
    else
    {
        H.setZero(2, 4);
        H(0, 0) = 1.0;
        H(1, 2) = 1.0;
    }
}

namespace helpers
{
    /**
    */
    void measurementUMatNoVelocity(kalman::Matrix& R, 
                                   const Measurement& mm, 
                                   const reconstruction::Uncertainty& default_uncert)
    {
        R.setIdentity(2, 2);

        if (mm.hasStdDevPosition())
        {
            R(0, 0) = mm.x_stddev.value() * mm.x_stddev.value();
            R(1, 1) = mm.y_stddev.value() * mm.y_stddev.value();

            if (mm.xy_cov.has_value())
            {
                R(0, 1) = mm.xy_cov.value();
                R(1, 0) = mm.xy_cov.value();
            }
        }
        else 
        {
            R *= default_uncert.pos_var;
        }
    }

    /**
    */
    void measurementUMatVelocity(kalman::Matrix& R, 
                                 const Measurement& mm, 
                                 const reconstruction::Uncertainty& default_uncert)
    {
        R.setIdentity(4, 4);

        //handle position uncertainty
        R(0, 0) = default_uncert.pos_var;
        R(2, 2) = default_uncert.pos_var;

        if (mm.hasStdDevPosition())
        {
            R(0, 0) = mm.x_stddev.value() * mm.x_stddev.value();
            R(2, 2) = mm.y_stddev.value() * mm.y_stddev.value();

            if (mm.xy_cov.has_value())
            {
                R(0, 2) = mm.xy_cov.value();
                R(2, 0) = mm.xy_cov.value();
            }
        }

        //handle speed uncertainty
        R(1, 1) = default_uncert.speed_var;
        R(3, 3) = default_uncert.speed_var;

        if (mm.hasVelocity() && mm.hasStdDevVelocity())
        {
            R(1, 1) = mm.vx_stddev.value() * mm.vx_stddev.value();
            R(3, 3) = mm.vy_stddev.value() * mm.vy_stddev.value();
        }
    }
}

/**
*/
void KalmanInterfaceUMKalman2D::measurementUncertMatR(kalman::Matrix& R,
                                                      const Measurement& mm, 
                                                      const reconstruction::Uncertainty& default_uncert) const
{
    if (track_velocities_)
        helpers::measurementUMatVelocity(R, mm, default_uncert);
    else
        helpers::measurementUMatNoVelocity(R, mm, default_uncert);
}

/**
*/
void KalmanInterfaceUMKalman2D::stateTransitionMatF(kalman::Matrix& F, double dt) const
{
    F.setIdentity(4, 4);

    F(0, 1) = dt;
    F(2, 3) = dt;
}

/**
*/
void KalmanInterfaceUMKalman2D::storeState(Measurement& mm, const kalman::KalmanState& state) const
{
    mm.x        = state.x[0];
    mm.y        = state.x[2];
    mm.vx       = state.x[1];
    mm.vy       = state.x[3];
    mm.x_stddev = std::sqrt(state.P(0, 0));
    mm.y_stddev = std::sqrt(state.P(2, 2));
    mm.xy_cov   = state.P(2, 0);
}

/**
*/
void KalmanInterfaceUMKalman2D::xPos(double& x, double& y, const kalman::Vector& x_vec) const
{
    x = x_vec[ 0 ];
    y = x_vec[ 2 ];
}

/**
*/
void KalmanInterfaceUMKalman2D::xPos(double& x, double& y) const
{
    return xPos(x, y, kalman_filter_->getX());
}

/**
*/
void KalmanInterfaceUMKalman2D::xPos(kalman::Vector& x_vec, double x, double y) const
{
    x_vec[ 0 ] = x;
    x_vec[ 2 ] = y;
}

/**
*/
double KalmanInterfaceUMKalman2D::xVar(const kalman::Matrix& P) const
{
    return P(0, 0);
}

/**
*/
double KalmanInterfaceUMKalman2D::yVar(const kalman::Matrix& P) const
{
    return P(2, 2);
}

/**
*/
double KalmanInterfaceUMKalman2D::xyCov(const kalman::Matrix& P) const
{
    return P(2, 0);
}

/**
*/
void KalmanInterfaceUMKalman2D::stateVecXInv(kalman::Vector& x_inv, const kalman::Vector& x) const
{
    x_inv = x;

    x_inv[ 1 ] *= -1.0;
    x_inv[ 3 ] *= -1.0;
}

} // reconstruction

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

#include "kalman_filter_um2d.h"
#include "reconstruction_defs.h"

namespace reconstruction
{

/**
*/
KalmanInterfaceUMKalman2D::KalmanInterfaceUMKalman2D(bool track_velocities) 
:   KalmanInterface  (new kalman::KalmanFilterUM2D(track_velocities))
,   track_velocities_(track_velocities)
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

} // reconstruction

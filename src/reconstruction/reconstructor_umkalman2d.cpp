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

#include "reconstructor_umkalman2d.h"
#include "kalman.h"

#include "logger.h"

namespace reconstruction
{

namespace helpers
{
    kalman::Matrix processMat(double dt, double Q_var)
    {
        return kalman::KalmanFilter::continuousWhiteNoise(2, dt, Q_var, 2);
    }

    kalman::Matrix measurementUMat(double R_var, const Measurement& mm)
    {
        kalman::Matrix R;
        R.setIdentity(2, 2);

        if (mm.hasStdDev())
        {
            R(0, 0) = mm.stddev_x.value() * mm.stddev_x.value();
            R(1, 1) = mm.stddev_y.value() * mm.stddev_y.value();

            if (mm.cov_xy.has_value())
            {
                R(0, 1) = mm.cov_xy.value();
                R(1, 0) = mm.cov_xy.value();
            }

            return R;
        }

        return R * R_var;
    }

    kalman::Matrix transitionMat(double dt)
    {
        kalman::Matrix F(4, 4);
        F.setIdentity();

        F(0, 1) = dt;
        F(2, 3) = dt;

        return F;
    }

    kalman::Matrix measurementMat()
    {
        kalman::Matrix F(2, 4);
        F.setZero();

        F(0, 0) = 1.0;
        F(1, 2) = 1.0;

        return F;
    }
}

/**
*/
Reconstructor_UMKalman2D::Reconstructor_UMKalman2D()
{
    kalman_.reset(new kalman::KalmanFilter(4, 2, 0));
}

/**
*/
Reconstructor_UMKalman2D::~Reconstructor_UMKalman2D() = default;

/**
*/
kalman::KalmanState Reconstructor_UMKalman2D::kalmanState() const
{
    return kalman_->state();
}

/**
*/
boost::optional<kalman::KalmanState> Reconstructor_UMKalman2D::kalmanStep(double dt, const Measurement& mm)
{
    auto F = helpers::transitionMat(dt);
    auto Q = helpers::processMat(dt, qVar());
    auto z = zVec(mm);
    auto R = helpers::measurementUMat(rVar(), mm);

    if (verbosity() > 1)
    {
        loginf << "[Kalman step] dt = " << dt << "\n" 
               << "F: " << F << "\n"
               << "Q: " << Q << "\n"
               << "z: " << z << "\n"
               << "R: " << R;
    }

    kalman_->predict(F, Q);
    bool ok = kalman_->update(z, R);
    if (!ok)
        return {};

    kalman::KalmanState state;
    state.x = kalman_->getX();
    state.P = kalman_->getP();
    state.F = F;
    state.Q = Q;

    return state;
}

/**
*/
kalman::Vector Reconstructor_UMKalman2D::xVec(const Measurement& mm) const
{
    kalman::Vector x(4);

    x[ 0 ] = mm.x;
    x[ 2 ] = mm.y;

    //start with zero velocity
    x[ 1 ] = 0.0;
    x[ 3 ] = 0.0;

    if (mm.hasVelocity())
    {
        x[ 1 ] = mm.vx.value();
        x[ 3 ] = mm.vy.value();
    }

    return x;
}

/**
*/
kalman::Matrix Reconstructor_UMKalman2D::pMat(const Measurement& mm) const
{
    kalman::Matrix P(4, 4);
    P.setZero();
    
    if (config_.simple_init)
    {
        double speed_var = pVar() * baseConfig().P_std;

        P(0, 0) = pVar();
        P(1, 1) = speed_var;
        P(2, 2) = pVar();
        P(3, 3) = speed_var;

        return P;
    }

    double var_x = pVar();
    double var_y = pVar();

    if (mm.hasStdDev())
    {
        var_x = mm.stddev_x.value() * mm.stddev_x.value();
        var_y = mm.stddev_y.value() * mm.stddev_y.value();
    }

    double var_vx = pVar();
    double var_vy = pVar();

    if (!mm.hasVelocity())
    {
        var_vx = pVarHigh();
        var_vy = pVarHigh();
    }

    P(0, 0) = var_x;
    P(1, 1) = var_vx;
    P(2, 2) = var_y;
    P(3, 3) = var_vy;

    return P;
}

/**
*/
kalman::Vector Reconstructor_UMKalman2D::zVec(const Measurement& mm) const
{
    kalman::Vector z(2);
    z[ 0 ] = mm.x;
    z[ 1 ] = mm.y;

    return z;
}

/**
*/
void Reconstructor_UMKalman2D::storeState_impl(Reference& ref,
                                               const kalman::KalmanState& state) const
{
    ref.x  = state.x[0];
    ref.y  = state.x[2];
    ref.vx = state.x[1];
    ref.vy = state.x[3];
}

/**
*/
void Reconstructor_UMKalman2D::init_impl(const Measurement& mm) const
{
    //init kalman state
    kalman_->setStateVec(xVec(mm));
    kalman_->setCovarianceMat(pMat(mm));
    kalman_->setStateTransitionMat(helpers::transitionMat(1.0));
    kalman_->setProcessUncertMat(helpers::processMat(1.0, qVar()));
    kalman_->setMeasurementMat(helpers::measurementMat());
    kalman_->setMeasurementUncertMat(helpers::measurementUMat(rVar(), mm));

    if (verbosity() > 1)
    {
        loginf << "[Reinit Kalman] \n"
         << "x: " << kalman_->getX() << "\n"
         << "P: " << kalman_->getP() << "\n"
         << "F: " << kalman_->getF() << "\n"
         << "Q: " << kalman_->getQ() << "\n"
         << "H: " << kalman_->getH() << "\n"
         << "R: " << kalman_->getR();
    }
}

} // namespace reconstruction

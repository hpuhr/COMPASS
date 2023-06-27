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

namespace helpers_umkalman2d
{
    kalman::Matrix processMat(double dt, double Q_var)
    {
        return kalman::KalmanFilter::continuousWhiteNoise(2, dt, Q_var, 2);
    }

    kalman::Matrix measurementUMatNoVelocity(const Measurement& mm, const reconstruction::Uncertainty& uncert)
    {
        kalman::Matrix R;
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
            
            return R;
        }

        return R * uncert.pos_var;
    }

    kalman::Matrix measurementUMatVelocity(const Measurement& mm, const reconstruction::Uncertainty& uncert)
    {
        kalman::Matrix R;
        R.setIdentity(4, 4);

        //handle position uncertainty
        R(0, 0) = uncert.pos_var;
        R(2, 2) = uncert.pos_var;

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
        R(1, 1) = uncert.speed_var;
        R(3, 3) = uncert.speed_var;

        if (mm.hasVelocity() && mm.hasStdDevVelocity())
        {
            R(1, 1) = mm.vx_stddev.value() * mm.vx_stddev.value();
            R(3, 3) = mm.vy_stddev.value() * mm.vy_stddev.value();
        }

        return R;
    }

    kalman::Matrix measurementUMat(const Measurement& mm, const reconstruction::Uncertainty& uncert, bool track_vel)
    {
        if (track_vel)
            return measurementUMatVelocity(mm, uncert);

        return measurementUMatNoVelocity(mm, uncert);
    }

    kalman::Matrix transitionMat(double dt)
    {
        kalman::Matrix F(4, 4);
        F.setIdentity();

        F(0, 1) = dt;
        F(2, 3) = dt;

        return F;
    }

    kalman::Matrix measurementMat(bool track_vel)
    {
        kalman::Matrix F;

        if (track_vel)
        {
            F.setIdentity(4, 4);
        }
        else
        {
            F.setZero(2, 4);
            F(0, 0) = 1.0;
            F(1, 2) = 1.0;
        }
        
        return F;
    }
}

/**
*/
Reconstructor_UMKalman2D::Reconstructor_UMKalman2D(bool track_velocities)
:   track_velocities_(track_velocities)
{
    kalman_.reset(new kalman::KalmanFilter(4, track_velocities_ ? 4 : 2, 0));
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
    auto uncert = defaultUncertaintyOfMeasurement(mm);

    auto F = helpers_umkalman2d::transitionMat(dt);
    auto Q = helpers_umkalman2d::processMat(dt, qVar());
    auto z = zVec(mm);
    auto R = helpers_umkalman2d::measurementUMat(mm, uncert, track_velocities_);

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

    ReconstructorKalman::xPos(x, mm);

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
kalman::Vector Reconstructor_UMKalman2D::xVecInv(const kalman::Vector& x) const
{
    kalman::Vector x_inv = x;

    x_inv[ 1 ] *= -1.0;
    x_inv[ 3 ] *= -1.0;

    return x_inv;
}

/**
*/
void Reconstructor_UMKalman2D::xVec(const kalman::Vector& x) const
{
    //update state vector after projection change
    kalman_->setStateVec(x);
}

/**
*/
void Reconstructor_UMKalman2D::xPos(double& x, double& y, const kalman::Vector& x_vec) const
{
    x = x_vec[ 0 ];
    y = x_vec[ 2 ];
}

/**
*/
void Reconstructor_UMKalman2D::xPos(kalman::Vector& x_vec, double x, double y) const
{
    x_vec[ 0 ] = x;
    x_vec[ 2 ] = y;
}

/**
*/
kalman::Matrix Reconstructor_UMKalman2D::pMat(const Measurement& mm) const
{
    auto uncert = defaultUncertaintyOfMeasurement(mm);

    kalman::Matrix P(4, 4);
    P.setZero();

    double var_x  = uncert.pos_var;
    double var_y  = uncert.pos_var;
    double cov_xy = 0.0;

    if (mm.hasStdDevPosition())
    {
        var_x = mm.x_stddev.value() * mm.x_stddev.value();
        var_y = mm.y_stddev.value() * mm.y_stddev.value();

        if (mm.xy_cov.has_value())
            cov_xy = mm.xy_cov.value();
    }

    double var_vx = uncert.speed_var;
    double var_vy = uncert.speed_var;

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

    return P;
}

/**
*/
kalman::Vector Reconstructor_UMKalman2D::zVec(const Measurement& mm) const
{
    kalman::Vector z;

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

    return z;
}

/**
*/
double Reconstructor_UMKalman2D::xVar(const kalman::Matrix& P) const
{
    return P(0, 0);
}

/**
*/
double Reconstructor_UMKalman2D::yVar(const kalman::Matrix& P) const
{
    return P(2, 2);
}

/**
*/
void Reconstructor_UMKalman2D::storeState_impl(Reference& ref,
                                               const kalman::KalmanState& state) const
{
    ref.x        = state.x[0];
    ref.y        = state.x[2];
    ref.vx       = state.x[1];
    ref.vy       = state.x[3];
    ref.x_stddev = std::sqrt(state.P(0, 0));
    ref.y_stddev = std::sqrt(state.P(2, 2));
    ref.xy_cov   = state.P(2, 0);
}

/**
*/
void Reconstructor_UMKalman2D::init_impl(const Measurement& mm) const
{
    auto uncert = defaultUncertaintyOfMeasurement(mm);

    //init kalman state
    kalman_->setStateVec(xVec(mm));
    kalman_->setCovarianceMat(pMat(mm));
    kalman_->setStateTransitionMat(helpers_umkalman2d::transitionMat(1.0));
    kalman_->setProcessUncertMat(helpers_umkalman2d::processMat(1.0, qVar()));
    kalman_->setMeasurementMat(helpers_umkalman2d::measurementMat(track_velocities_));
    kalman_->setMeasurementUncertMat(helpers_umkalman2d::measurementUMat(mm, uncert, track_velocities_));

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

/**
*/
boost::optional<kalman::KalmanState> Reconstructor_UMKalman2D::interpStep(const kalman::KalmanState& state0, 
                                                                          const kalman::KalmanState& state1,
                                                                          double dt) const
{
#if 0
    kalman_->setX(state0.x);
    kalman_->setP(state0.P);

    auto F = helpers_umkalman2d::transitionMat(dt);
    auto Q = helpers_umkalman2d::processMat(dt, qVarResample());

    kalman_->predict(F, Q);

    kalman::KalmanState new_state;
    new_state.x = kalman_->getX();
    new_state.P = kalman_->getP();
    new_state.F = F;
    new_state.Q = Q;

    return new_state;
#else
    bool forward = dt >= 0.0;

    if (!forward)
        dt = std::fabs(dt);

    kalman_->setX(forward ? state0.x : xVecInv(state0.x));
    kalman_->setP(state0.P);

    auto F = helpers_umkalman2d::transitionMat(dt);
    auto Q = helpers_umkalman2d::processMat(dt, qVarResample());

    kalman_->predict(F, Q);

    kalman::KalmanState new_state;
    new_state.x = forward ? kalman_->getX() : xVecInv(kalman_->getX());
    new_state.P = kalman_->getP();
    new_state.F = F;
    new_state.Q = Q;

    return new_state;
#endif
}

/**
*/
bool Reconstructor_UMKalman2D::smoothChain_impl(std::vector<kalman::Vector>& x_smooth,
                                                std::vector<kalman::Matrix>& P_smooth,
                                                const KalmanChain& chain,
                                                const kalman::XTransferFunc& x_tr) const
{
    return kalman::KalmanFilter::rtsSmoother(x_smooth, 
                                             P_smooth, 
                                             chain.kalman_states,
                                             x_tr);
}

} // namespace reconstruction

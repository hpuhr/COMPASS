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

#include "kalman_interface_linear.h"
#include "kalman.h"

#include "logger.h"

namespace reconstruction
{

/**
*/
KalmanInterfaceLinear::KalmanInterfaceLinear() = default;

/**
*/
KalmanInterfaceLinear::~KalmanInterfaceLinear() = default;

/**
*/
bool KalmanInterfaceLinear::init()
{
    kalman_filter_.reset(new kalman::KalmanFilter(dimX(), dimZ(), dimU()));
    return true;
}

/**
*/
void KalmanInterfaceLinear::kalmanInit_impl(kalman::KalmanState& init_state,
                                            const Measurement& mm,
                                            const reconstruction::Uncertainty& default_uncert,
                                            double Q_var)
{
    const double dt_start = 1.0;

    //init kalman
    stateVecXFromMM(kalman_filter_->getX(), mm);
    covarianceMatP(kalman_filter_->getP(), mm, default_uncert);
    stateTransitionMatF(kalman_filter_->getF(), dt_start);
    processUncertMatQ(kalman_filter_->getQ(), dt_start, Q_var);
    measurementMatH(kalman_filter_->getH());
    measurementUncertMatR(kalman_filter_->getR(), mm, default_uncert);

    if (verbosity() > 1)
    {
        loginf << "[Reinit Kalman] \n"
         << "x: " << kalman_filter_->getX() << "\n"
         << "P: " << kalman_filter_->getP() << "\n"
         << "F: " << kalman_filter_->getF() << "\n"
         << "Q: " << kalman_filter_->getQ() << "\n"
         << "H: " << kalman_filter_->getH() << "\n"
         << "R: " << kalman_filter_->getR();
    }

    init_state.x  = kalman_filter_->getX();
    init_state.P  = kalman_filter_->getP();
    init_state.F  = kalman_filter_->getF();
    init_state.Q  = kalman_filter_->getQ();
    init_state.dt = dt_start;
}

/**
*/
void KalmanInterfaceLinear::kalmanInit_impl(const kalman::KalmanState& init_state)
{
    kalman_filter_->init(init_state.x, init_state.P);
    
    kalman_filter_->setQ(init_state.Q);
    kalman_filter_->setF(init_state.F);

    measurementMatH(kalman_filter_->getH());
}

/**
*/
void KalmanInterfaceLinear::kalmanInit_impl(const kalman::Vector& x,
                                            const kalman::Matrix& P)
{
    kalman_filter_->init(x, P);

    measurementMatH(kalman_filter_->getH());
}

/**
*/
bool KalmanInterfaceLinear::kalmanStep_impl(kalman::KalmanState& new_state,
                                            double dt, 
                                            const Measurement& mm, 
                                            const reconstruction::Uncertainty& default_uncert, 
                                            double Q_var)
{
    assert(kalman_filter_);

    //set kalman internal matrices
    stateTransitionMatF(kalman_filter_->getF(), dt);
    processUncertMatQ(kalman_filter_->getQ(), dt, Q_var);
    measurementUncertMatR(kalman_filter_->getR(), mm, default_uncert);

    //get measurement vector
    kalman::Vector z;
    measurementVecZ(z, mm);

    if (verbosity() > 1)
    {
        loginf << "[Kalman step] dt = " << dt << "\n" 
               << "F: " << kalman_filter_->getF() << "\n"
               << "Q: " << kalman_filter_->getQ() << "\n"
               << "z: " << z << "\n"
               << "R: " << kalman_filter_->getR();
    }

    //use internal kalman matrices if possible
    if (!kalman_filter_->predict({}, {}))
    {
        kalman_filter_->revert();
        return false;
    }

    if (!kalman_filter_->update(z, {}))
    {
        kalman_filter_->revert();
        return false;
    }

    new_state.x  = kalman_filter_->getX();
    new_state.P  = kalman_filter_->getP();
    new_state.F  = kalman_filter_->getF();
    new_state.Q  = kalman_filter_->getQ();
    new_state.dt = dt;

    return true;
}

/**
*/
bool KalmanInterfaceLinear::kalmanPrediction_impl(kalman::Vector& x,
                                                  kalman::Matrix& P,
                                                  double dt,
                                                  double Q_var,
                                                  bool fix_estimate,
                                                  bool* fixed) const
{
    assert(kalman_filter_);

    if (dt < 0.0)
    {
        //extra treatment for negative steps

        //backup current state
        kalman::Vector x_backup = kalman_filter_->getX();

        //invert state...
        stateVecXInv(kalman_filter_->getX(), x_backup);

        //...and use positive timestep
        dt = std::fabs(dt);

        //predict
        kalman::Matrix F, Q;
        stateTransitionMatF(F, dt);
        processUncertMatQ(Q, dt, Q_var);

        bool ok = kalman_filter_->predictState(x, P, F, Q, {}, {}, fix_estimate, fixed);

        //revert state
        kalman_filter_->setX(x_backup);

        if (!ok)
            return false;
    }
    else
    {
        kalman::Matrix F, Q;
        stateTransitionMatF(F, dt);
        processUncertMatQ(Q, dt, Q_var);

        if (!kalman_filter_->predictState(x, P, F, Q, {}, {}, fix_estimate, fixed))
            return false;
    }

    return true;
}

/**
*/
kalman::KalmanState KalmanInterfaceLinear::currentState() const
{
    return kalman_filter_->state();
}

/**
*/
void KalmanInterfaceLinear::stateVecX(const kalman::Vector& x)
{
    kalman_filter_->setX(x);
}

/**
*/
bool KalmanInterfaceLinear::smoothUpdates_impl(std::vector<kalman::Vector>& x_smooth,
                                               std::vector<kalman::Matrix>& P_smooth,
                                               const std::vector<kalman::KalmanState>& states,
                                               const kalman::XTransferFunc& x_tr) const
{
    return kalman::KalmanFilter::rtsSmoother(x_smooth, P_smooth, states, x_tr);
}

/**
*/
boost::optional<kalman::KalmanState> KalmanInterfaceLinear::interpStep(const kalman::KalmanState& state0,
                                                                       const kalman::KalmanState& state1,
                                                                       double dt,
                                                                       double Q_var,
                                                                       bool fix_estimate,
                                                                       bool* fixed) const
{
#if 0
    kalman_filter_->setX(state0.x);
    kalman_filter_->setP(state0.P);

    stateTransitionMatF(kalman_filter_->getF(), dt);
    processUncertMatQ(kalman_filter_->getQ(), dt, Q_var);

    if (!kalman_filter_->predict({}, {}, {}, {}, fix_estimate, fixed))
        return {};

    kalman::KalmanState new_state;
    new_state.x = kalman_filter_->getX();
    new_state.P = kalman_filter_->getP();
    new_state.F = F;
    new_state.Q = Q;

    return new_state;
#else
    bool forward = dt >= 0.0;

    if (!forward)
        dt = std::fabs(dt);

    if (forward)
        kalman_filter_->setX(state0.x);
    else
        stateVecXInv(kalman_filter_->getX(), state0.x);

    kalman_filter_->setP(state0.P);

    stateTransitionMatF(kalman_filter_->getF(), dt);
    processUncertMatQ(kalman_filter_->getQ(), dt, Q_var);

    if (!kalman_filter_->predict({}, {}, {}, {}, fix_estimate, fixed))
        return {};

    kalman::KalmanState new_state;

    if (forward)
        new_state.x = kalman_filter_->getX();
    else
        stateVecXInv(new_state.x, kalman_filter_->getX());

    new_state.P = kalman_filter_->getP();
    new_state.F = kalman_filter_->getF();
    new_state.Q = kalman_filter_->getQ();

    return new_state;
#endif
}

/**
*/
std::string KalmanInterfaceLinear::asString(const std::string& prefix) const
{
    return kalman_filter_->asString(prefix);
}

} // reconstruction

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
bool KalmanInterfaceLinear::kalmanInit(kalman::KalmanState& init_state,
                                       const Measurement& mm,
                                       const reconstruction::Uncertainty& default_uncert,
                                       double Q_var)
{
    const double dt_start = 1.0;

    //init kalman
    stateVecX(kalman_filter_->getX(), mm);
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

    return true;
}

/**
*/
bool KalmanInterfaceLinear::kalmanStep(kalman::KalmanState& new_state,
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
    kalman_filter_->predict({}, {});
    if (!kalman_filter_->update(z, {}))
        return false;

    new_state.x  = kalman_filter_->getX();
    new_state.P  = kalman_filter_->getP();
    new_state.F  = kalman_filter_->getF();
    new_state.Q  = kalman_filter_->getQ();
    new_state.dt = dt;

    return true;
}

} // reconstruction

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

    z_ = kalman::Vector(kalman_filter_->dimZ());

    return true;
}

/**
*/
bool KalmanInterfaceLinear::kalmanInit(kalman::KalmanState& new_state,
                                       const Measurement& mm,
                                       const reconstruction::Uncertainty& default_uncert)
{
    //init kalman state
    // kalman_->setStateVec(xVec(mm));
    // kalman_->setCovarianceMat(pMat(mm));
    // kalman_->setStateTransitionMat(helpers_umkalman2d::transitionMat(1.0));
    // kalman_->setProcessUncertMat(helpers_umkalman2d::processMat(1.0, qVar()));
    // kalman_->setMeasurementMat(helpers_umkalman2d::measurementMat(track_velocities_));
    // kalman_->setMeasurementUncertMat(helpers_umkalman2d::measurementUMat(mm, uncert, track_velocities_));

    // if (verbosity() > 1)
    // {
    //     loginf << "[Reinit Kalman] \n"
    //      << "x: " << kalman_->getX() << "\n"
    //      << "P: " << kalman_->getP() << "\n"
    //      << "F: " << kalman_->getF() << "\n"
    //      << "Q: " << kalman_->getQ() << "\n"
    //      << "H: " << kalman_->getH() << "\n"
    //      << "R: " << kalman_->getR();
    // }

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
    measurementVecZ(z_, mm);
    measurementUncertMatR(kalman_filter_->getR(), mm, default_uncert);

    // if (verbosity() > 1)
    // {
    //     loginf << "[Kalman step] dt = " << dt << "\n" 
    //            << "F: " << F << "\n"
    //            << "Q: " << Q << "\n"
    //            << "z: " << z << "\n"
    //            << "R: " << R;
    // }

    //use internal kalman matrices if possible
    kalman_filter_->predict({}, {});
    if (!kalman_filter_->update(z_, {}))
        return false;

    new_state.x = kalman_filter_->getX();
    new_state.P = kalman_filter_->getP();
    new_state.F = kalman_filter_->getF();
    new_state.Q = kalman_filter_->getQ();

    return true;
}

} // reconstruction

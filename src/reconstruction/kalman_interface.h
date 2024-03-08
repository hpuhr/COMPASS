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

#include "kalman_defs.h"

#include <boost/optional.hpp>

namespace reconstruction
{

struct Measurement;
struct Uncertainty;

/**
 * Provides needed data structures for kalman and means to retrieve data from kalman state. 
 * Derive for a certain kalman variant.
 */
class KalmanInterface
{
public:
    KalmanInterface() = default;
    virtual ~KalmanInterface() = default;

    virtual bool init() = 0;

    virtual bool kalmanInit(kalman::KalmanState& init_state,
                            const Measurement& mm,
                            const reconstruction::Uncertainty& default_uncert) = 0;
    virtual bool kalmanStep(kalman::KalmanState& new_state,
                            double dt, 
                            const Measurement& mm, 
                            const reconstruction::Uncertainty& default_uncert, 
                            double Q_var) = 0;
    //needed for feeding kalman
    virtual void stateVecX(kalman::Vector& x, const Measurement& mm) const = 0;
    virtual void measurementVecZ(kalman::Vector& z, const Measurement& mm) const = 0;
    virtual void covarianceMatP(kalman::Matrix& P,
                                const Measurement& mm, 
                                const reconstruction::Uncertainty& default_uncert) const = 0;
    virtual void processUncertMatQ(kalman::Matrix& Q, double dt, double Q_var) const = 0;
    virtual void measurementMatH(kalman::Matrix& H) const = 0;
    virtual void measurementUncertMatR(kalman::Matrix& R, 
                                       const Measurement& mm, 
                                       const reconstruction::Uncertainty& default_uncert) const = 0;
    virtual void stateTransitionMatF(kalman::Matrix& F, double dt) const = 0;
    
    //needed for retrieval from kalman
    virtual void storeState(Measurement& mm, const kalman::KalmanState& state) const = 0;
    
    //helpers
    virtual void xPos(double& x, double& y, const kalman::Vector& x_vec) const = 0;
    virtual void xPos(kalman::Vector& x_vec, double x, double y) const = 0;
    virtual double xVar(const kalman::Matrix& P) const = 0;
    virtual double yVar(const kalman::Matrix& P) const = 0;
    virtual void stateVecXInv(kalman::Vector& x_inv, const kalman::Vector& x) const = 0;
};

} // reconstruction

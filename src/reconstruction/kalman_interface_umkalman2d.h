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

#include "kalman_interface.h"

namespace kalman
{
    class KalmanFilter;
}

namespace reconstruction
{

struct Measurement;
struct Uncertainty;

/**
 */
class KalmanInterfaceUMKalman2D : public KalmanInterface
{
public:
    KalmanInterfaceUMKalman2D(bool track_velocities);
    virtual ~KalmanInterfaceUMKalman2D() = default;

    KalmanInterface* clone() const override final;

    void stateVecXFromMM(kalman::Vector& x, const Measurement& mm) const override final;
    void measurementVecZ(kalman::Vector& z, const Measurement& mm) const override final;
    void covarianceMatP(kalman::Matrix& P, const Measurement& mm, const reconstruction::Uncertainty& default_uncert) const override final;
    void measurementUncertMatR(kalman::Matrix& R, 
                               const Measurement& mm, 
                               const reconstruction::Uncertainty& default_uncert) const override final;
private:
    bool track_velocities_;
};

} // reconstruction

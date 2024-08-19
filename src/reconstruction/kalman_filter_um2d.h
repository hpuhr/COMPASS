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

#include "kalman_filter_linear.h"

namespace kalman
{

/**
*/
class KalmanFilterUM2D : public KalmanFilterLinear
{
public:
    KalmanFilterUM2D(bool track_velocities);
    virtual ~KalmanFilterUM2D();

protected:
    void configureFMat(Matrix& F, double dt) const;
    void configureQMat(Matrix& Q, double dt, double var) const;

private:
    void invertState(Vector& x) const;

    using KalmanFilterLinear::setFMatFunc;
    using KalmanFilterLinear::setQMatFunc;
    using KalmanFilter::setInvertStateFunc;

    using KalmanFilterLinear::B_;
    using KalmanFilterLinear::M_;
    using KalmanFilterLinear::H_;
};

} // namespace kalman

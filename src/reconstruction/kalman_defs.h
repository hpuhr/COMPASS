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

#include <Eigen/Core>

#include <boost/optional.hpp>

namespace kalman
{

typedef Eigen::MatrixXd         Matrix;
typedef Eigen::VectorXd         Vector;
typedef boost::optional<Matrix> OMatrix;
typedef boost::optional<Vector> OVector;

/**
*/
struct KalmanState
{
    KalmanState() {}
    KalmanState(const Vector& _x, const Matrix& _P) : x(_x), P(_P) {}

    Vector x;
    Matrix P;
    Matrix F;
    Matrix Q;
};

} // namespace kalman

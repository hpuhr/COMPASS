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

#include <Eigen/Dense>

#include <boost/optional.hpp>

/**
 * https://stackoverflow.com/questions/41538095/evaluate-multivariate-normal-gaussian-density-in-c
*/
class GaussianPDF
{
public:
    GaussianPDF(const Eigen::MatrixXd& P_cov)
    {
        //store decomposition for later evaluations
        chol_P_cov_ = Eigen::LLT<Eigen::MatrixXd>(P_cov);
        LogSqrt2Pi_ = 0.5 * std::log(2 * M_PI);
    }
    virtual ~GaussianPDF() = default;

    /**
    */
    bool valid() const
    {
        return (chol_P_cov_.info() == Eigen::Success);
    }

    /**
    */
    boost::optional<double> eval(const Eigen::VectorXd& x) const
    {
        // Handle non positive definite covariance somehow:
        if(!valid())
            return {};

        const Eigen::LLT<Eigen::MatrixXd>::Traits::MatrixL& L = chol_P_cov_.matrixL();
        double quadform = (L.solve(x)).squaredNorm();

        return std::max(std::numeric_limits<double>::epsilon(), std::exp(-x.rows() * LogSqrt2Pi_ - 0.5 * quadform) / L.determinant());
    }

private:
    Eigen::LLT<Eigen::MatrixXd> chol_P_cov_;
    double                      LogSqrt2Pi_;
};

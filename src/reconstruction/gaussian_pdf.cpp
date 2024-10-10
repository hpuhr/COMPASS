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

#include "gaussian_pdf.h"

#include "logger.h"

const double GaussianPDF::LikelihoodEpsilonDefault    = std::numeric_limits<double>::epsilon();
const double GaussianPDF::LogLikelihoodEpsilonDefault = -16.0;
const double GaussianPDF::LogSqrt2Pi                  = 0.5 * std::log(2 * M_PI);

/**
*/
GaussianPDF::GaussianPDF(const Eigen::MatrixXd& P_cov,
                         double eps_lh,
                         double eps_llh)
:   eps_lh_ (eps_lh )
,   eps_llh_(eps_llh)
{
    //store decomposition for later evaluations
    chol_P_cov_ = Eigen::LLT<Eigen::MatrixXd>(P_cov);
}

/**
*/
bool GaussianPDF::valid() const
{
    return (chol_P_cov_.info() == Eigen::Success);
}

/**
*/
boost::optional<double> GaussianPDF::likelihood(const Eigen::VectorXd& x, bool check_eps) const
{
    // Handle non positive definite covariance somehow:
    if(!valid())
        return {};

    const Eigen::LLT<Eigen::MatrixXd>::Traits::MatrixL& L = chol_P_cov_.matrixL();
    const double quadform = (L.solve(x)).squaredNorm();
    const double lh       = std::exp(-x.rows() * LogSqrt2Pi - 0.5 * quadform) / L.determinant();

    return (check_eps ? std::max(eps_lh_, lh) : lh);
}

/**
*/
boost::optional<double> GaussianPDF::logLikelihood(const Eigen::VectorXd& x, bool check_eps) const
{
    // Handle non positive definite covariance somehow:
    if(!valid())
        return {};

    const Eigen::LLT<Eigen::MatrixXd>::Traits::MatrixL& L = chol_P_cov_.matrixL();
    const double quadform = (L.solve(x)).squaredNorm();
    const double llh      = -x.rows() * LogSqrt2Pi - 0.5 * quadform - std::log(L.determinant());

    return (check_eps ? std::max(eps_llh_, llh) : llh);
}

namespace
{
    void normalizeLikelihoodWinnerTakesAll(Eigen::VectorXd& likelihoods, bool debug)
    {
        int    max_idx = -1;
        double max_val = 0.0;

        for (int i = 0; i < likelihoods.size(); ++i)
        {
            if (likelihoods[ i ] > max_val)
            {
                max_val = likelihoods[ i ];
                max_idx = i;
            }
        }

        if (debug)
            loginf << "winner takes all: max_val = " << max_val << ", max_idx = " << max_idx;

        if (max_idx < 0)
        {
            //spread evenly
            likelihoods.setOnes();
            likelihoods /= likelihoods.size();

            if (debug)
                loginf << "renormed spread even:\n" << likelihoods;

            return;
        }

        //max lh takes it all
        likelihoods.array() = std::numeric_limits<double>::epsilon();
        likelihoods[ max_idx ] = 1.0;
        likelihoods /= likelihoods.sum();

        if (debug)
            loginf << "renormed wta:\n" << likelihoods;
    }
}

/**
*/
void GaussianPDF::normalizeLikelihoods(Eigen::VectorXd& likelihoods, 
                                       NormalizeMode mode, 
                                       double eps, 
                                       bool debug)
{
    if (likelihoods.size() < 1)
        return;

    //preemptive maximum value check
    const double max_val = likelihoods.maxCoeff();
    if (max_val < 1e-300)
    {
        normalizeLikelihoodWinnerTakesAll(likelihoods, debug);
        return;
    }

    if (mode == NormalizeMode::Sum || max_val >= eps)
    {
        //assure minimum prob
        for (auto& lh : likelihoods)
            lh = std::max(lh, std::numeric_limits<double>::epsilon());

        //normalize directly via sum
        likelihoods /= likelihoods.sum();
        return;
    }

    // NormalizeMode::Log

    //convert to log likelihoods
    for (auto& lh : likelihoods)
        lh = lh < 1e-300 ? std::numeric_limits<double>::signaling_NaN() : std::log(lh);

    if (debug)
        loginf << "after log:\n" << likelihoods;

    //determine max log value and subtract
    double max_val_log = std::numeric_limits<double>::lowest();
    for (auto& llh : likelihoods)
        if (std::isfinite(llh) && llh > max_val_log)
            max_val_log = llh;

    if (debug)
        loginf << "logmax: " << max_val_log;

    for (auto& llh : likelihoods)
        if (std::isfinite(llh))
            llh -= max_val_log;

    if (debug)
        loginf << "after log renorm:\n" << likelihoods;

    //convert back to likelihoods
    for (auto& lh : likelihoods)
        lh = !std::isfinite(lh) || lh < -700.0 ? 0.0 : std::exp(lh);

    if (debug)
        loginf << "after exp:\n" << likelihoods;

    //check sum
    const double sum = likelihoods.sum();
    if (sum < 1e-12)
    {
        normalizeLikelihoodWinnerTakesAll(likelihoods, debug);
        return;
    }

    //assure minimum prob
    for (auto& lh : likelihoods)
        lh = std::max(lh, std::numeric_limits<double>::epsilon());

    //normalize via sum
    likelihoods /= likelihoods.sum();

    if (debug)
        loginf << "renormed log:\n" << likelihoods;

    //final check
    for (auto& lh : likelihoods)
        assert(std::isfinite(lh));

    //likelihoods = likelihoods.array().log();
    //likelihoods.array() -= likelihoods.maxCoeff();
    //likelihoods = likelihoods.array().exp();
    //likelihoods /= likelihoods.sum();
}

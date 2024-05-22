
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

#include "kalman.h"
#include "logger.h"

#include <Eigen/Dense>

namespace kalman
{

/**
*/
KalmanFilter::KalmanFilter(size_t dim_x, 
                           size_t dim_z,
                           size_t dim_u)
    :   dim_x_(dim_x)
    ,   dim_z_(dim_z)
    ,   dim_u_(dim_u)
{
    x_.setZero(dim_x);
    P_.setIdentity(dim_x, dim_x);
    Q_.setIdentity(dim_x, dim_x);
    F_.setIdentity(dim_x, dim_x);
    H_.setZero(dim_z, dim_x);
    R_.setIdentity(dim_z, dim_z);
    M_.setZero(dim_x, dim_z);

    K_.setZero(dim_x, dim_z);
    y_.setZero(dim_z);
    S_.setZero(dim_z, dim_z);
    SI_.setZero(dim_z, dim_z);

    I_.setIdentity(dim_x, dim_x);

    x_backup_ = x_;
    P_backup_ = P_;

    x_prior_ = x_;
    P_prior_ = P_;

    x_post_ = x_;
    P_post_ = P_;
}

/**
*/
KalmanFilter::~KalmanFilter() = default;

/**
*/
kalman::KalmanState KalmanFilter::state() const
{
    kalman::KalmanState state;
    state.x = x_;
    state.P = P_;
    state.Q = Q_;
    state.F = F_;

    return state;
}

/**
*/
void KalmanFilter::revert()
{
    x_ = x_backup_;
    P_ = P_backup_;
}

/**
*/
void KalmanFilter::predict(const OMatrix& F,
                           const OMatrix& Q,
                           const OMatrix& B,
                           const OVector& u)
{
    const OMatrix& B__ = B.has_value() ? B         : B_;
    const Matrix&  F__ = F.has_value() ? F.value() : F_;
    const Matrix&  Q__ = Q.has_value() ? Q.value() : Q_;

    // save backup state
    x_backup_ = x_;
    P_backup_ = P_;

    // x = Fx + Bu
    x_ = F__ * x_;
    if (B__.has_value() && u.has_value())
        x_ += B__.value() * u.value();
    
    // P = FPF' + Q
    P_ = alpha_sq_ * (F__ * P_ * F__.transpose()) + Q__;

    // save prior
    x_prior_ = x_;
    P_prior_ = P_;
}

/**
 * @param external_state If true the current state and covariance used are provided in x and P.
*/
void KalmanFilter::predictState(Vector& x,
                                Matrix& P,
                                const Matrix& F,
                                const Matrix& Q,
                                const OMatrix& B,
                                const OVector& u) const
{
    // x = Fx + Bu
    x = F * x_;
    if (B.has_value() && u.has_value())
        x += B.value() * u.value();

    // P = FPF' + Q
    P = alpha_sq_ * (F * P_ * F.transpose()) + Q;
}

/**
*/
bool KalmanFilter::update(const Vector& z,
                          const OMatrix& R,
                          const OMatrix& H)
{
    // reset to force recompute
    log_likelihood_.reset();
    likelihood_.reset();
    mahalanobis_.reset();

    z_.reset();

    const Matrix& R__ = R.has_value() ? R.value() : R_;
    const Matrix& H__ = H.has_value() ? H.value() : H_;

    // y = z - Hx
    // error (residual) between measurement and prediction
    y_ = z - H__ * x_;

    // common subexpression for speed
    Matrix PHT = P_ * H__.transpose();

    // S = HPH' + R
    // project system uncertainty into measurement space
    S_  = (H__ * PHT) + R__;

    if (!Eigen::FullPivLU<Eigen::MatrixXd>(S_).isInvertible())
    {
        x_post_ = x_;
        P_post_ = P_;
        y_.setZero(dim_z_);
        return false;
    }

    SI_ = S_.inverse();

    // K = PH'inv(S)
    // map system uncertainty into kalman gain
    K_ = PHT * SI_;

    // x = x + Ky
    // predict new x with residual scaled by the kalman gain
    x_ += K_ * y_;

    // P = (I-KH)P(I-KH)' + KRK'
    // This is more numerically stable
    // and works for non-optimal K vs the equation
    // P = (I-KH)P usually seen in the literature.

    Matrix I_KH = I_ - K_ * H__;
    P_ = I_KH * P_ * I_KH.transpose() + K_ * R__ * K_.transpose();

    // save measurement and posterior state
    z_      = z;
    x_post_ = x_;
    P_post_ = P_;

    return true;
}

/**
*/
void KalmanFilter::printState() const
{
    loginf << "x_:";
    loginf << x_;
    loginf << "P_:";
    loginf << P_;
    loginf << "S_:";
    loginf << S_;
    loginf << "PivLU(S_):";
    loginf << Eigen::FullPivLU<Eigen::MatrixXd>(S_).matrixLU();
    if (z_.has_value())
    {
        loginf << "z_:";
        loginf << z_.value();
    }
}

/**
*/
void KalmanFilter::continuousWhiteNoise(Matrix& Q_noise, 
                                        size_t dim, 
                                        double dt, 
                                        double spectral_density, 
                                        size_t block_size)
{
    if (dim < 2 || dim > 4)
        throw std::runtime_error("KalmanFilter::continuousWhiteNoise(): dim must be between 2 and 4");
    if (block_size < 1)
        throw std::runtime_error("KalmanFilter::continuousWhiteNoise(): block size must be > 0");

    size_t full_size = dim * block_size;

    Q_noise.setZero(full_size, full_size);

    const double dt2 = dt * dt;

    if (dim == 2)
    {
        Matrix Q;
        Q.setZero(2, 2);
        Q(0, 0) = dt2 * dt / 3.0;  Q(0, 1) = dt2 / 2.0;
        Q(1, 0) = dt2      / 2.0;  Q(1, 1) = dt;

        if (block_size == 1)
        {
            Q_noise = Q;
        }
        else
        {
            for (size_t i = 0; i < block_size; ++i)
                Q_noise.block<2, 2>(i * 2, i * 2) = Q;
        }
    }
    else if (dim == 3)
    {
        const double dt3 = dt2 * dt;
        const double dt4 = dt2 * dt2;

        Matrix Q;
        Q.setZero(3, 3);
        Q(0, 0) = dt3 * dt2 / 20.0;  Q(0, 1) = dt4 / 8.0; Q(0, 2) = dt3 / 6.0;
        Q(1, 0) = dt4       / 8.0 ;  Q(1, 1) = dt3 / 3.0; Q(1, 2) = dt2 / 2.0;
        Q(2, 0) = dt3       / 6.0 ;  Q(2, 1) = dt2 / 2.0; Q(2, 2) = dt;

        if (block_size == 1)
        {
            Q_noise = Q;
        }
        else
        {
            for (size_t i = 0; i < block_size; ++i)
                Q_noise.block<3, 3>(i * 3, i * 3) = Q;
        }
    }
    else // dim == 4
    {
        const double dt3 = dt2 * dt;
        const double dt4 = dt2 * dt2;
        const double dt5 = dt3 * dt2;
        const double dt6 = dt2 * dt2 * dt2;

        Matrix Q;
        Q.setZero(4, 4);
        Q(0, 0) = dt6 * dt / 252.0;  Q(0, 1) = dt6 / 72.0; Q(0, 2) = dt5 / 30.0; Q(0, 3) = dt4 / 24.0;
        Q(1, 0) = dt6      / 72.0;   Q(1, 1) = dt5 / 20.0; Q(1, 2) = dt4 / 8.0 ; Q(1, 3) = dt3 / 6.0;
        Q(2, 0) = dt5      / 30.0;   Q(2, 1) = dt4 / 8.0 ; Q(2, 2) = dt3 / 3.0 ; Q(2, 3) = dt2 / 2.0;
        Q(3, 0) = dt4      / 24.0;   Q(3, 1) = dt3 / 6.0 ; Q(3, 2) = dt2 / 2.0 ; Q(3, 3) = dt;

        if (block_size == 1)
        {
            Q_noise = Q;
        }
        else
        {
            for (size_t i = 0; i < block_size; ++i)
                Q_noise.block<4, 4>(i * 4, i * 4) = Q;
        }
    }

    Q_noise *= spectral_density;
}

/**
*/
Matrix KalmanFilter::continuousWhiteNoise(size_t dim,
                                          double dt,
                                          double spectral_density,
                                          size_t block_size)
{
    Matrix Q_noise;
    continuousWhiteNoise(Q_noise, dim, dt, spectral_density, block_size);
    return Q_noise;
}

/**
*/
bool KalmanFilter::rtsSmoother(std::vector<kalman::Vector>& x_smooth,
                               std::vector<kalman::Matrix>& P_smooth,
                               const std::vector<KalmanState>& states,
                               const XTransferFunc& x_tr)
{
    if (states.empty())
        return true;

    int n     = (int)states.size();
    int dim_x = (int)states[0].x.size();

    x_smooth.resize(n);
    P_smooth.resize(n);

    std::vector<kalman::Matrix> Pp(n);
    std::vector<kalman::Matrix> K (n);

    for (int i = 0; i < n; ++i)
    {
        x_smooth[ i ] = states[ i ].x;
        P_smooth[ i ] = states[ i ].P / 10.0; // TODO HACK
        Pp      [ i ] = states[ i ].P / 10.0;
        K       [ i ].setZero(dim_x, dim_x);
    }

    if (states.size() < 2)
        return true;

    kalman::Vector x_smooth_1_tr;

    for (int k = n - 2; k >= 0; --k)
    {
        const auto& F_1  = states[ k+1 ].F;
        const auto& Q_1  = states[ k+1 ].Q;
        auto        F_1t = F_1.transpose();

        Pp[ k ] = states[ k+1 ].F * P_smooth[ k ] * F_1t + Q_1;

        if (!Eigen::FullPivLU<Eigen::MatrixXd>(Pp[ k ]).isInvertible())
            return false;

        K[ k ] = P_smooth[ k ] * F_1t * Pp[ k ].inverse();

        //get last smoothed state vector
        if (x_tr) x_tr(x_smooth_1_tr, x_smooth[ k+1 ], k + 1, k);
        const kalman::Vector& x_smooth_1 = x_tr ? x_smooth_1_tr : x_smooth[ k+1 ];

        x_smooth[ k ] += K[ k ] * (x_smooth_1 - F_1 * x_smooth[ k ]);
        P_smooth[ k ] += K[ k ] * (P_smooth[ k+1 ] - Pp[ k ]) * ((const kalman::Matrix&)K[ k ]).transpose();
    }

    return true;
}

} // namespace kalman

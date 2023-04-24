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

#include <vector>

#include <Eigen/Core>

#include <boost/optional.hpp>

namespace kalman
{

/**
*/
class KalmanFilter
{
public:
    KalmanFilter(size_t dim_x, 
                 size_t dim_z, 
                 size_t dim_u);
    virtual ~KalmanFilter();

    void predict(const OMatrix& F = OMatrix(),
                 const OMatrix& Q = OMatrix(),
                 const OMatrix& B = OMatrix(),
                 const OVector& u = OVector());

    bool update(const Vector& z,
                const OMatrix& R = OMatrix(), 
                const OMatrix& H = OMatrix());

    static Matrix continuousWhiteNoise(size_t dim, double dt = 1.0, double spectral_density = 1.0, size_t block_size = 1);
    static bool rtsSmoother(std::vector<kalman::Vector>& x_smooth,
                            std::vector<kalman::Matrix>& P_smooth,
                            const std::vector<KalmanState>& states);

    kalman::KalmanState state() const;

    void setX(const Vector& x) { x_ = x; }
    void setP(const Matrix& P) { P_ = P; }
    void setQ(const Matrix& Q) { Q_ = Q; }
    void setB(const Matrix& B) { B_ = B; }
    void setF(const Matrix& F) { F_ = F; }
    void setH(const Matrix& H) { H_ = H; }
    void setR(const Matrix& R) { R_ = R; }
    void setM(const Matrix& M) { M_ = M; }

    void setStateVec(const Vector& x) { setX(x); }
    void setCovarianceMat(const Matrix& P) { setP(P); }
    void setProcessUncertMat(const Matrix& Q) { setQ(Q); }
    void setControlTransitionMat(const Matrix& B) { setB(B); }
    void setStateTransitionMat(const Matrix& F) { setF(F); }
    void setMeasurementMat(const Matrix& H) { setH(H); }
    void setMeasurementUncertMat(const Matrix& R) { setR(R); }
    void setProcessMMCrossCorrMat(const Matrix& M) { setM(M); }

    const Vector& getX() const { return x_; }
    const Matrix& getP() const { return P_; }
    const Matrix& getQ() const { return Q_; }
    const Matrix& getF() const { return F_; }
    const Matrix& getH() const { return H_; }
    const Matrix& getR() const { return R_; }
    const Matrix& getM() const { return M_; }

private:
    size_t dim_x_;
    size_t dim_z_;
    size_t dim_u_;

    Vector  x_; // state
    Matrix  P_; // uncertainty covariance
    Matrix  Q_; // process uncertainty
    OMatrix B_; // control transition matrix
    Matrix  F_; // state transition matrix
    Matrix  H_; // measurement function
    Matrix  R_; // measurement uncertainty
    Matrix  M_; // process-measurement cross correlation

    OVector z_;

    double alpha_sq_ = 1.0; // fading memory control

    Matrix K_;  // kalman gain
    Vector y_;
    Matrix S_;  // system uncertainty
    Matrix SI_; // inverse system uncertainty

    Matrix I_;  // identity matrix

    //after predict()
    Vector x_prior_;
    Matrix P_prior_;

    //after update()
    Vector x_post_;
    Matrix P_post_;

    boost::optional<double> log_likelihood_;
    boost::optional<double> likelihood_;
    boost::optional<double> mahalanobis_;
};

} // namespace kalman

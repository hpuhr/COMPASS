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

    size_t dimX() const { return dim_x_; }
    size_t dimZ() const { return dim_z_; }
    size_t dimU() const { return dim_u_; }

    bool predict(const OMatrix& F = OMatrix(),
                 const OMatrix& Q = OMatrix(),
                 const OMatrix& B = OMatrix(),
                 const OVector& u = OVector(),
                 bool fix_estimate = false,
                 bool* fixed = nullptr);
    bool predictState(Vector& x,
                      Matrix& P,
                      const Matrix& F,
                      const Matrix& Q,
                      const OMatrix& B = OMatrix(),
                      const OVector& u = OVector(),
                      bool fix_estimate = false,
                      bool* fixed = nullptr) const;

    bool update(const Vector& z,
                const OMatrix& R = OMatrix(), 
                const OMatrix& H = OMatrix());

    bool checkState() const;
    void revert();

    static Matrix continuousWhiteNoise(size_t dim, double dt = 1.0, double spectral_density = 1.0, size_t block_size = 1);
    static void continuousWhiteNoise(Matrix& Q_noise, size_t dim, double dt = 1.0, double spectral_density = 1.0, size_t block_size = 1);

    static bool rtsSmoother(std::vector<kalman::Vector>& x_smooth,
                            std::vector<kalman::Matrix>& P_smooth,
                            const std::vector<KalmanState>& states,
                            const XTransferFunc& x_tr = XTransferFunc());

    kalman::KalmanState state() const;

    void init(const Vector& x, const Matrix& P);

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
    Vector& getX() { return x_; }
    const Matrix& getP() const { return P_; }
    Matrix& getP() { return P_; }
    const Matrix& getQ() const { return Q_; }
    Matrix& getQ() { return Q_; }
    const Matrix& getF() const { return F_; }
    Matrix& getF() { return F_; }
    const Matrix& getH() const { return H_; }
    Matrix& getH() { return H_; }
    const Matrix& getR() const { return R_; }
    Matrix& getR() { return R_; }
    const Matrix& getM() const { return M_; }
    Matrix& getM() { return M_; }

    std::string asString(const std::string prefix = "") const;

private:
    bool checkState(const Vector& x, const Matrix& P) const;
    void postConditionP(Matrix& P) const;

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

    //before predict()
    Vector  x_backup_;
    Matrix  P_backup_;

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

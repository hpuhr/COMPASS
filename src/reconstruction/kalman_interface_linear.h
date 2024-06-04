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

#include <memory>

namespace kalman
{
    class KalmanFilter;
}

namespace reconstruction
{

struct Measurement;
struct Uncertainty;

/**
 * Provides needed data structures for kalman and means to retrieve data from kalman state. 
 * Derive for a certain kalman variant.
 */
class KalmanInterfaceLinear : public KalmanInterface
{
public:
    KalmanInterfaceLinear();
    virtual ~KalmanInterfaceLinear();

    virtual size_t dimX() const = 0; 
    virtual size_t dimZ() const = 0;
    virtual size_t dimU() const = 0;

    virtual bool init() override;

    virtual kalman::KalmanState currentState() const override final;

    void stateVecX(const kalman::Vector& x) override final;

    boost::optional<kalman::KalmanState> interpStep(const kalman::KalmanState& state0,
                                                    const kalman::KalmanState& state1,
                                                    double dt,
                                                    double Q_var) const override final;

    std::string asString(const std::string& prefix = "") const override final;

protected:
    void kalmanInit_impl(kalman::KalmanState& init_state,
                         const Measurement& mm, 
                         const reconstruction::Uncertainty& default_uncert,
                         double Q_var) override final;
    void kalmanInit_impl(const kalman::KalmanState& init_state) override final;
    void kalmanInit_impl(const kalman::Vector& x,
                         const kalman::Matrix& P) override final;

    bool kalmanStep_impl(kalman::KalmanState& new_state,
                         double dt, 
                         const Measurement& mm, 
                         const reconstruction::Uncertainty& default_uncert, 
                         double Q_var) override final;
        
    bool kalmanPrediction_impl(kalman::Vector& x,
                               kalman::Matrix& P,
                               double dt,
                               double Q_var) const override final;
    bool smoothUpdates_impl(std::vector<kalman::Vector>& x_smooth,
                            std::vector<kalman::Matrix>& P_smooth,
                            const std::vector<kalman::KalmanState>& states,
                            const kalman::XTransferFunc& x_tr) const override final;

    std::unique_ptr<kalman::KalmanFilter> kalman_filter_;
};

} // reconstruction

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

#include "reconstructor_kalman.h"

#include <memory>

namespace kalman
{
    class KalmanFilter;
}

namespace reconstruction
{

/**
*/
class Reconstructor_UMKalman2D : public ReconstructorKalman
{
public:
    struct Config
    {
        bool simple_init = true;
    };

    Reconstructor_UMKalman2D(bool track_velocities = false);
    virtual ~Reconstructor_UMKalman2D();

    Config& config() { return config_; }

protected:
    kalman::KalmanState kalmanState() const override final;
    boost::optional<kalman::KalmanState> kalmanStep(double dt, const Measurement& mm) override final;
    kalman::Vector xVec(const Measurement& mm) const override final;
    kalman::Matrix pMat(const Measurement& mm) const override final;
    kalman::Vector zVec(const Measurement& mm) const override final;
    void storeState_impl(Reference& ref, const kalman::KalmanState& state) const override final;
    void init_impl(const Measurement& mm) const override final;

private:
    Config config_;
    bool   track_velocities_ = false;

    std::unique_ptr<kalman::KalmanFilter> kalman_;
};

} // namespace reconstruction

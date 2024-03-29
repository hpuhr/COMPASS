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

#include "reconstructor_defs.h"
#include "kalman_estimator.h"

#include <memory>

namespace reconstruction
{

class KalmanInterface;
class KalmanProjectionHandler;

/**
*/
class KalmanOnlineTracker
{
public:
    KalmanOnlineTracker();
    virtual ~KalmanOnlineTracker();

    void reset();

    bool isInit() const;
    void init(std::unique_ptr<KalmanInterface>&& interface);
    void init(kalman::KalmanType ktype);

    void kalmanInit(Measurement& mm);
    void kalmanInit(const kalman::KalmanUpdate& update);
    bool kalmanStep(Measurement& mm);
    bool kalmanPrediction(Measurement& mm,
                          double dt) const;

    bool isTracking() const;

    KalmanEstimator::Settings& settings();
    const boost::optional<kalman::KalmanUpdate>& currentState() const;
    const KalmanEstimator& estimator() const;

private:
    std::unique_ptr<KalmanEstimator>      estimator_;
    boost::optional<kalman::KalmanUpdate> current_update_;
};

} // reconstruction

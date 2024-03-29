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

#include "kalman_online_tracker.h"

namespace reconstruction
{

/**
*/
KalmanOnlineTracker::KalmanOnlineTracker()
:   estimator_(new KalmanEstimator)
{
}

/**
*/
KalmanOnlineTracker::~KalmanOnlineTracker() = default;

/**
*/
void KalmanOnlineTracker::reset()
{
    current_update_.reset();
}

/**
*/
bool KalmanOnlineTracker::isInit() const
{
    return estimator_->isInit();
}

/**
*/
void KalmanOnlineTracker::init(std::unique_ptr<KalmanInterface>&& interface)
{
    estimator_->init(std::move(interface));
}

/**
*/
void KalmanOnlineTracker::init(kalman::KalmanType ktype)
{
    estimator_->init(ktype);
}

/**
*/
void KalmanOnlineTracker::kalmanInit(Measurement& mm)
{
    assert(isInit());

    if (!current_update_.has_value())
        current_update_ = kalman::KalmanUpdate();

    estimator_->kalmanInit(current_update_.value(), mm);
}

/**
*/
void KalmanOnlineTracker::kalmanInit(const kalman::KalmanUpdate& update)
{
    assert(isInit());

    current_update_ = update;

    estimator_->kalmanInit(current_update_.value());
}

/**
*/
bool KalmanOnlineTracker::kalmanStep(Measurement& mm)
{
    assert(isInit());
    assert(isTracking());

    return (estimator_->kalmanStep(current_update_.value(), mm) == KalmanEstimator::StepResult::Success);
}

/**
*/
bool KalmanOnlineTracker::kalmanPrediction(Measurement& mm,
                                           double dt) const
{
    assert(isInit());
    assert(isTracking());

    return estimator_->kalmanPrediction(mm, dt);
}

/**
*/
bool KalmanOnlineTracker::isTracking() const
{
    return current_update_.has_value();
}

/**
*/
KalmanEstimator::Settings& KalmanOnlineTracker::settings()
{
    return estimator_->settings();
}

/**
*/
const boost::optional<kalman::KalmanUpdate>& KalmanOnlineTracker::currentState() const
{
    return current_update_;
}

/**
*/
const KalmanEstimator& KalmanOnlineTracker::estimator() const
{
    return *estimator_;
}

} // reconstruction
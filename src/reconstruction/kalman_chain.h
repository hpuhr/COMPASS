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
#include <map>

namespace reconstruction
{

class KalmanOnlineTracker;
class KalmanInterface;

/**
*/
class KalmanChain
{
public:
    struct Settings
    {
        boost::posix_time::time_duration max_reestim_duration;
        int                              max_reestim_updates;
        double                           reestim_residual;

        int verbosity = 0;
    };

    struct Update
    {
        Update() {}
        Update(const Measurement& mm, const kalman::KalmanUpdate& update = kalman::KalmanUpdate()) : measurement(mm), kalman_update(update) {}

        Measurement          measurement;
        kalman::KalmanUpdate kalman_update;
        bool                 init = false;
    };

    typedef std::pair<int, int> Interval;

    KalmanChain();
    virtual ~KalmanChain();

    void reset();

    bool isInit() const;
    void init(std::unique_ptr<KalmanInterface>&& interface);
    void init(kalman::KalmanType ktype);

    bool add(const Measurement& mm, bool reestim = false);
    void add(const std::vector<Measurement>& mms);
    void insert(const Measurement& mm);
    void insert(const std::vector<Measurement>& mms);

    bool needsReestimate() const;
    bool reestimate();

    bool predict(Measurement& mm_predicted,
                 const boost::posix_time::ptime& ts) const;
    bool predictFromLastState(Measurement& mm_predicted,
                              const boost::posix_time::ptime& ts) const;
    
    size_t size() const;
    int count() const;

    Settings& settings();
    KalmanEstimator::Settings& estimatorSettings();

private:
    void insert(int idx, const Measurement& mm);
    void addReesimationIndex(int idx);
    void resetReestimationIndices();
    bool reinit(int idx) const;
    bool reestimate(int idx);
    bool reestimate(int idx, double& dp);

    int lastIndex() const;

    Interval interval(const boost::posix_time::ptime& ts) const;
    int insertionIndex(const boost::posix_time::ptime& ts) const;
    int predictionRefIndex(const boost::posix_time::ptime& ts) const;
    
    Settings settings_;

    mutable std::unique_ptr<KalmanOnlineTracker> tracker_;
    mutable int                                  tracked_update_ = -1;
    int                                          reestim_min_    = -1;
    int                                          reestim_max_    = -1;
    std::vector<Update>                          updates_;
};

} // reconstruction

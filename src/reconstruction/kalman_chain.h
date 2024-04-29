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
 * Chain of kalman updates in which new measurements can be inserted and 
 * a reestimation of kalman states can be triggered.
 * - Utilizes a KalmanOnlineTracker which is reinitialized to certain chain updates as needed
 * - The number of states reestimated following the insertion of a new measurement can be specified using multiple criteria
 */
class KalmanChain
{
public:
    struct Settings
    {
        typedef boost::posix_time::time_duration TD;

        TD     max_reestim_duration       = boost::posix_time::seconds(30); // maximum timeframe reestimated after a new mm has been inserted
        int    max_reestim_updates        = 500;                            // maximum updates reestimated after a new mm has been inserted
        double reestim_residual_state_sqr = 100;                            // 10  * 10  - reestimation stop criterion based on state change residual
        double reestim_residual_cov_sqr   = 10000;                          // 100 * 100 - reestimation stop criterion based on cov mat change residual
        TD     max_prediction_tdiff       = boost::posix_time::seconds(30); // maximum difference in time which can be predicted

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

    bool add(const Measurement& mm, bool reestim);
    bool add(const std::vector<Measurement>& mms, bool reestim);
    bool insert(const Measurement& mm, bool reestim);
    bool insert(const std::vector<Measurement>& mms, bool reestim);

    bool needsReestimate() const;
    bool reestimate();

    bool canPredict(const boost::posix_time::ptime& ts) const;
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
    bool reestimate(int idx, double& d_state_sqr, double& d_cov_sqr);

    int lastIndex() const;

    Interval interval(const boost::posix_time::ptime& ts) const;
    int insertionIndex(const boost::posix_time::ptime& ts) const;
    int predictionRefIndex(const boost::posix_time::ptime& ts) const;
    
    Settings settings_;

    mutable std::unique_ptr<KalmanOnlineTracker> tracker_;
    mutable int                                  tracked_update_ = -1;
    std::vector<int>                             fresh_indices_;
    std::vector<Update>                          updates_;
};

} // reconstruction

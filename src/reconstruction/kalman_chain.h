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
class KalmanEstimator;
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

        enum class Mode
        {
            DynamicInserts = 0,  // allows dynamic inserts and reestimation, but needs to keep track of all states (more memory)
            StaticAdd            // allows adding to the end of the chain, and predictions at the end, only the current state is kept
        };

        enum class PredictionMode
        {
            NearestBefore = 0,
            Interpolate,
            LastUpdate
        };

        Mode           mode            = Mode::DynamicInserts;
        PredictionMode prediction_mode = PredictionMode::NearestBefore;

        TD     reestim_max_duration       = boost::posix_time::seconds(30); // maximum timeframe reestimated after a new mm has been inserted
        int    reestim_max_updates        = 500;                            // maximum updates reestimated after a new mm has been inserted
        double reestim_residual_state_sqr = 100;                            // 10  * 10  - reestimation stop criterion based on state change residual
        double reestim_residual_cov_sqr   = 10000;                          // 100 * 100 - reestimation stop criterion based on cov mat change residual

        TD     prediction_max_tdiff       = boost::posix_time::seconds(10); // maximum difference in time which can be predicted

        int    verbosity = 0;
    };

    struct Update
    {
        Update() {}
        Update(const Measurement& mm, 
               int id, 
               const kalman::KalmanUpdateMinimal& update = kalman::KalmanUpdateMinimal()) 
        :   measurement  (mm)
        ,   kalman_update(update)
        ,   mm_id        (id) {}

        Measurement                 measurement;
        kalman::KalmanUpdateMinimal kalman_update;
        int                         mm_id = -1;
        bool                        init  = false;
    };

    typedef std::pair<int, int>                  Interval;
    typedef std::unique_ptr<KalmanOnlineTracker> TrackerPtr;
    typedef std::unique_ptr<KalmanEstimator>     EstimatorPtr;

    KalmanChain(int max_prediction_threads = 1);
    virtual ~KalmanChain();

    void reset();
    bool hasData() const;

    bool isInit() const;
    void init(std::unique_ptr<KalmanInterface>&& interface);
    void init(kalman::KalmanType ktype);

    void configureEstimator(const KalmanEstimator::Settings& settings);

    bool add(const Measurement& mm, bool reestim);
    bool add(const std::vector<Measurement>& mms, bool reestim);
    bool insert(const Measurement& mm, bool reestim);
    bool insert(const std::vector<Measurement>& mms, bool reestim);

    void removeUpdatesBefore(const boost::posix_time::ptime& ts);

    kalman::KalmanUpdateMinimal lastUpdate() const;
    const kalman::KalmanUpdateMinimal& getUpdate(size_t idx) const;
    const Measurement& getMeasurement(size_t idx) const;

    bool canReestimate() const;
    bool needsReestimate() const;
    bool reestimate();

    bool canPredict(const boost::posix_time::ptime& ts) const;
    bool predict(Measurement& mm_predicted,
                 const boost::posix_time::ptime& ts,
                 int thread_id = 0) const;
    
    size_t size() const;
    int count() const;

    Settings& settings();

private:
    struct Tracker
    {
        void reset();

        TrackerPtr   tracker_ptr;
        int tracked_mm_id = -1;
    };

    struct Predictor
    {
        void reset();

        EstimatorPtr estimator_ptr;
        int ref_mm_id = -1;
    };

    bool checkIntegrity() const;

    void insert(int idx, const Measurement& mm);
    void addReesimationIndex(int idx);
    void addReesimationIndexRange(int idx0, int idx1);
    void resetReestimationIndices();
    bool reinit(int idx) const;
    bool reestimate(int idx);
    bool reestimate(int idx, double& d_state_sqr, double& d_cov_sqr);

    int lastIndex() const;

    Interval interval(const boost::posix_time::ptime& ts) const;
    int insertionIndex(const boost::posix_time::ptime& ts) const;
    int predictionRefIndex(const boost::posix_time::ptime& ts) const;
    Interval predictionRefInterval(const boost::posix_time::ptime& ts) const;

    Settings settings_;

    mutable Tracker                tracker_;
    mutable std::vector<Predictor> predictors_;
    std::vector<int>               fresh_indices_;
    std::vector<Update>            updates_;

    int mm_ids_ = 0;
};

} // reconstruction

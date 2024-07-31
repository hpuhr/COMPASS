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

#include "kalman_chain.h"
#include "kalman_online_tracker.h"
#include "kalman_interface.h"

#include "logger.h"
#include "timeconv.h"

namespace reconstruction
{

/**
*/
size_t KalmanChainPredictors::size() const
{
    return predictors.size();
}

/**
*/
bool KalmanChainPredictors::isInit() const
{
    if (size() == 0)
        return false;

    for (const auto& p : predictors)
        if (!p->isInit())
            return false;

    return true;
}

/**
*/
KalmanEstimator& KalmanChainPredictors::predictor(size_t idx)
{
    auto& p = predictors.at(idx);
    assert(p);

    return *p;
}

/**
*/
void KalmanChainPredictors::init(std::unique_ptr<KalmanInterface>&& interface,
                                 const KalmanEstimator::Settings& settings,
                                 unsigned int max_threads)
{
    assert (max_threads > 0);

    predictors.clear();
    predictors.resize(max_threads);

    for (unsigned int i = 0; i < max_threads; ++i)
    {
        predictors[ i ].reset(new KalmanEstimator);
        predictors[ i ]->settings() = settings;
        predictors[ i ]->init(std::unique_ptr<KalmanInterface>(interface->clone()));
    }
}

/**
*/
void KalmanChainPredictors::init(kalman::KalmanType ktype,
                                 const KalmanEstimator::Settings& settings,
                                 unsigned int max_threads)
{
    assert (max_threads > 0);

    predictors.clear();
    predictors.resize(max_threads);

    for (unsigned int i = 0; i < max_threads; ++i)
    {
        predictors[ i ].reset(new KalmanEstimator);
        predictors[ i ]->settings() = settings;
        predictors[ i ]->init(ktype);
    }
}

/**
*/
void KalmanChain::Tracker::reset()
{
    assert(tracker_ptr);
    tracker_ptr->reset();

    tracked_mm_id.reset();
}

/**
*/
void KalmanChain::Predictor::reset()
{
    ref_mm_id.reset();
}

/**
*/
KalmanChain::KalmanChain()
{
    predictor_.estimator_ptr.reset(new KalmanEstimator);
    tracker_.tracker_ptr.reset(new KalmanOnlineTracker);

    reset();
}

/**
*/
KalmanChain::~KalmanChain() = default;

/**
*/
bool KalmanChain::checkIntegrity() const
{
    for (size_t i = 1; i < size(); ++i)
        if (updates_[ i ].t < updates_[ i - 1 ].t)
            return false;

    return true;
}

/**
 * Resets the chain.
*/
void KalmanChain::reset()
{
    updates_.clear();
    tracker_.reset();
    predictor_.reset();

    resetReestimationIndices();
}

/**
*/
bool KalmanChain::hasData() const
{
    return (canReestimate() ? size() > 0 : tracker_.tracker_ptr->isTracking());
}

/**
 * Checks if the chain has been initialized.
*/
bool KalmanChain::isInit() const
{
    if (!tracker_.tracker_ptr->isInit())
        return false;
    
    if (!predictor_.estimator_ptr->isInit())
        return false;
    
    return true;
}

/**
 * Initializes the chain by passing a kalman interface.
*/
void KalmanChain::init(std::unique_ptr<KalmanInterface>&& interface)
{
    assert(interface);

    predictor_.estimator_ptr->init(std::unique_ptr<KalmanInterface>(interface->clone()));
    tracker_.tracker_ptr->init(std::move(interface));
}

/**
 * Initializes the chain to the given kalman type.
*/
void KalmanChain::init(kalman::KalmanType ktype)
{
    predictor_.estimator_ptr->init(ktype);
    tracker_.tracker_ptr->init(ktype);
}

/**
 * Returns the current number of chain elements.
*/
size_t KalmanChain::size() const
{
    return updates_.size();
}

/**
*/
int KalmanChain::count() const
{
    return (int)updates_.size();
}

/**
*/
int KalmanChain::lastIndex() const
{
    return count() - 1;
}

/**
*/
KalmanChain::Settings& KalmanChain::settings()
{
    return settings_;
}

/**
*/
void KalmanChain::configureEstimator(const KalmanEstimator::Settings& settings)
{
    predictor_.estimator_ptr->settings() = settings;
    tracker_.tracker_ptr->settings()     = settings;
}

/**
*/
void KalmanChain::setMeasurementGetFunc(const MeasurementGetFunc& get_func)
{
    assert(get_func);
    get_func_ = get_func;
}

/**
*/
void KalmanChain::setMeasurementAssignFunc(const MeasurementAssignFunc& assign_func)
{
    assert(assign_func);
    assign_func_ = assign_func;
}

/**
*/
const Measurement& KalmanChain::getMeasurement(unsigned long mm_id) const
{
    assert(get_func_ || assign_func_);

    if (get_func_)
        return get_func_(mm_id);

    assign_func_(mm_tmp_, mm_id);
    return mm_tmp_;
}

/**
 * Returns an interval containing the given timestamp.
 * (Note: in case of multiple equal timestamps the datum is located at the upper bound - as in std::multimap)
*/
KalmanChain::Interval KalmanChain::interval(const boost::posix_time::ptime& ts) const
{
    //empty case?
    if (updates_.empty())
        return { -1, -1 };

    //begin of range?
    if (ts < updates_.begin()->t)
        return { -1, 0 };

    //end of range?
    if (ts >= updates_.rbegin()->t)
        return { lastIndex(), -1 };

    //find interval via upper bound
    auto it = std::upper_bound(updates_.begin(), updates_.end(), ts, 
        [ & ] (const boost::posix_time::ptime& t,
               const Update& u) { return t < u.t; } );

    //checked beforehand
    assert (it != updates_.begin());
    
    int idx = it - updates_.begin();

    //        <= ts    > ts = insert index
    return { idx - 1, idx };
}

/**
 * Returns the index at which the given timestamp should be inserted, 
 * or -1 if the index is to be added at the end of the chain.
 */
int KalmanChain::insertionIndex(const boost::posix_time::ptime& ts) const
{
    auto iv = interval(ts);

    if (iv.first < 0 && iv.second < 0)
        return -1;        // empty - add to end of chain
    else if (iv.first < 0 && iv.second == 0)
        return 0;         // add at begin of chain
    else if (iv.first >= 0 && iv.second < 0)
        return -1;        // add to end of chain
    else if (iv.first >= 0 && iv.second >= 0)
        return iv.second; // insert into the chain
    else
        assert(false);    // should-never-happen-case
    
    return -1;
}

/**
 * Returns the reference index of the interval in which the timestamp should be interpolated,
 * or -1 if no interpolation is possible. 
 */
int KalmanChain::predictionRefIndex(const boost::posix_time::ptime& ts) const
{
    auto iv = interval(ts);

    if (iv.first < 0 && iv.second < 0)
        return -1;                       // empty
    else if (iv.first < 0 && iv.second == 0)
        return 0;                        // begin
    else if (iv.first >= 0 && iv.second < 0)
        return iv.first;                 // end
    else if (iv.first >= 0 && iv.second >= 0)
        return iv.first;                 // mid
    else
        assert(false);                   // should-never-happen-case
    
    return -1;
}

/**
*/
KalmanChain::Interval KalmanChain::predictionRefInterval(const boost::posix_time::ptime& ts) const
{
    auto iv = interval(ts);

    if (iv.first < 0 && iv.second < 0)
        return Interval(-1, -1);          // empty
    else if (iv.first < 0 && iv.second == 0)
        return Interval(-1, 0);           // begin
    else if (iv.first >= 0 && iv.second < 0)
        return Interval(lastIndex(), -1); // end
    else if (iv.first >= 0 && iv.second >= 0)
        return iv;                        // mid
    else
        assert(false); // should-never-happen-case
    
    return Interval(-1, -1);
}

/**
 * Internally used.
*/
bool KalmanChain::addToTracker(unsigned long mm_id,
                               const boost::posix_time::ptime& ts,
                               UpdateStats* stats)
{
    assert (!canReestimate());

    //just track measurement
    KalmanEstimator::StepResult res;
    bool ok = tracker_.tracker_ptr->track(getMeasurement(mm_id), true, &res);

    if (stats)
    {
        stats->set = true;

        ++stats->num_fresh;
        ++stats->num_updated;

        if (ok)
            ++stats->num_valid;
        else if (res == KalmanEstimator::StepResult::FailStepTooSmall)
            ++stats->num_skipped;
        else
            ++stats->num_failed;
    }

    return ok;
}

/**
 * Internally used.
*/
void KalmanChain::addToEnd(unsigned long mm_id,
                           const boost::posix_time::ptime& ts)
{
    assert (canReestimate());

    assert(updates_.empty() || ts >= updates_.rbegin()->t);
        
    addReesimationIndex(count());
    
    updates_.emplace_back(mm_id, ts);
}

/**
 * Adds a new measurement to the end of the chain.
*/
bool KalmanChain::add(unsigned long mm_id,
                      const boost::posix_time::ptime& ts,
                      bool reestim,
                      UpdateStats* stats)
{
    if (stats)
        *stats = {};

    if (canReestimate())
    {
        addToEnd(mm_id, ts);

        if (reestim && !reestimate(stats))
            return false;
    }
    else
    {
        if (!addToTracker(mm_id, ts, stats))
            return false;
    }

    return true;
}

/**
 * Adds new measurements to the end of the chain.
*/
bool KalmanChain::add(const std::vector<std::pair<unsigned long, boost::posix_time::ptime>>& mms,
                      bool reestim,
                      UpdateStats* stats)
{
    if (stats)
        *stats = {};

    if (mms.empty())
        return true;

    if (canReestimate())
    {
        //presort for safety
        typedef std::pair<unsigned long, boost::posix_time::ptime> MM;
        std::vector<MM> mms_sorted = mms;
        std::sort(mms_sorted.begin(), mms_sorted.end(), [ & ] (const MM& mm0, const MM& mm1) { return mm0.second < mm1.second; });

        //check correct time ordering
        if (!updates_.empty())
            assert(mms_sorted.begin()->second >= updates_.rbegin()->t);

        int idx_min = count();

        size_t n  = updates_.size();
        size_t ni = mms_sorted.size();

        updates_.resize(n + ni);

        for (size_t idx = 0, idx0 = n; idx < ni; ++idx, ++idx0)
        {
            updates_[ idx0 ].mm_id = mms_sorted[ idx ].first;
            updates_[ idx0 ].t     = mms_sorted[ idx ].second;
        }

        int idx_max = lastIndex();

        addReesimationIndexRange(idx_min, idx_max);

        if (reestim && !reestimate(stats))
            return false;
    }
    else
    {
        //just track measurements one after another
        bool has_failed_updates = false;
        for (const auto& mm : mms)
            if (!addToTracker(mm.first, mm.second, stats))
                has_failed_updates = true;

        if (has_failed_updates)
            return false;
    }

    return true;
}

/**
 * Inserts a new measurement at the given index.
*/
void KalmanChain::insertAt(int idx, 
                           unsigned long mm_id,
                           const boost::posix_time::ptime& ts)
{
    assert(canReestimate());

    // if (settings_.verbosity >= 2 && idx >= 0)
    // {
    //     boost::posix_time::ptime t0, t1;
    //     if (count() > 0)
    //     {
    //         t0 = idx < 0 ? updates_.rbegin()->measurement.t : updates_.at(idx).t;
    //         t1 = (idx < 0 || idx == lastIndex()) ? boost::posix_time::ptime() : updates_.at(idx + 1).t;
    //     }

    //     loginf << "KalmanChain: insert: Inserting t = " << Utils::Time::toString(ts)
    //           << " @idx " << idx << ", last_index = " << lastIndex() << " [" 
    //           << Utils::Time::toString(t0) << "," 
    //           << Utils::Time::toString(t1) << "]";
    // }

    if (idx < 0)
    {
        //just add to end
        addToEnd(mm_id, ts);
    }
    else
    {
        //insert
        assert(idx <= count());
        updates_.insert(updates_.begin() + idx, Update(mm_id, ts));

        addReesimationIndex(idx);
    }

    assert(checkIntegrity());
}

/**
 * Inserts a new measurement into the chain.
*/
bool KalmanChain::insert(unsigned long mm_id,
                         const boost::posix_time::ptime& ts, 
                         bool reestim,
                         UpdateStats* stats)
{
    //mode does not support inserts? => add instead
    if (!canReestimate())
        return add(mm_id, ts, reestim, stats);

    int idx = insertionIndex(ts);
    insertAt(idx, mm_id, ts);

    if (reestim)
        return reestimate(stats);

    return true;
}

/**
 * Inserts new measurements into the chain.
*/
bool KalmanChain::insert(const std::vector<std::pair<unsigned long, boost::posix_time::ptime>>& mms,
                         bool reestim,
                         UpdateStats* stats)
{
    //mode does not support inserts? => add instead
    if (!canReestimate())
        return add(mms, reestim, stats);
    
    for (const auto& mm : mms)
    {
        int idx = insertionIndex(mm.second);
        insertAt(idx, mm.first, mm.second);
    }

    if (reestim)
        return reestimate(stats);

    return true;
}

/**
*/
void KalmanChain::removeUpdatesBefore(const boost::posix_time::ptime& ts)
{
    assert(!needsReestimate());

    if (size() == 0)
        return;

    auto it = std::remove_if(updates_.begin(), updates_.end(), [ & ] (const Update& u) { return u.t < ts; });
    updates_.erase(it, updates_.end());
    updates_.shrink_to_fit();

    //current mm ids might be outdated now => reset
    tracker_.tracked_mm_id.reset();
    predictor_.ref_mm_id.reset();
}

/**
*/
const kalman::KalmanUpdateMinimal& KalmanChain::getKalmanUpdate(size_t idx) const
{
    assert(canReestimate());
    return updates_.at(idx).kalman_update;
}

/**
*/
kalman::KalmanUpdateMinimal KalmanChain::lastKalmanUpdate() const
{
    assert(hasData());
    return (canReestimate() ? updates_.rbegin()->kalman_update : tracker_.tracker_ptr->currentState().value().minimalInfo());
}

/**
*/
const KalmanChain::Update& KalmanChain::getUpdate(size_t idx)
{
    assert(canReestimate());
    return updates_.at(idx);
}

/**
 * Checks if a prediction is possible at the given timestamp.
*/
bool KalmanChain::canPredict(const boost::posix_time::ptime& ts) const
{
    //static mode? => just ask tracker
    if (!canReestimate())
        return tracker_.tracker_ptr->canPredict(ts, settings_.prediction_max_tdiff);

    //no updates no prediction
    if (updates_.empty())
        return false;

    if (settings_.prediction_mode == Settings::PredictionMode::LastUpdate ||
        settings_.prediction_mode == Settings::PredictionMode::NearestBefore)
    {
        //find reference update
        int idx = (settings_.prediction_mode == Settings::PredictionMode::LastUpdate ? lastIndex() : predictionRefIndex(ts));
        assert(idx >= 0);

        //check time diff
        auto t0 = updates_[ idx ].kalman_update.t;

        boost::posix_time::time_duration diff = (ts >= t0 ? ts - t0 : t0 - ts);
        
        if (diff > settings_.prediction_max_tdiff)
            return false;

        return true;
    }
    else if (settings_.prediction_mode == Settings::PredictionMode::Interpolate)
    {
        //find reference interval
        auto iv = predictionRefInterval(ts);
        assert(iv.first >= 0 || iv.second >= 0); //!updates not empty => non-empty interval needs to exist!

        bool first_ok  = false;
        bool second_ok = false;

        if (iv.first >= 0 && (ts - updates_[ iv.first ].t) <= settings_.prediction_max_tdiff)
            first_ok = true;
        if (iv.second >= 0 && (updates_[ iv.second ].t - ts) <= settings_.prediction_max_tdiff)
            second_ok = true;
        
        //not close enough to at least one interval border?
        if (!first_ok && !second_ok)
            return false;

        return true;
    }

    return false;
}

/**
*/
bool KalmanChain::predictMT(Measurement& mm_predicted,
                            const boost::posix_time::ptime& ts,
                            KalmanChainPredictors& predictors,
                            unsigned int thread_id,
                            PredictionStats* stats) const
{
    assert (thread_id < predictors.size());

    return predictInternal(mm_predicted, ts, &predictors, (int)thread_id, stats);
}

/**
*/
bool KalmanChain::predict(Measurement& mm_predicted,
                          const boost::posix_time::ptime& ts,
                          PredictionStats* stats) const
{
    return predictInternal(mm_predicted, ts, nullptr, 0, stats);
}

/**
 * Predicts the given timestamp from the nearest existing update in the chain.
*/
bool KalmanChain::predictInternal(Measurement& mm_predicted,
                                  const boost::posix_time::ptime& ts,
                                  KalmanChainPredictors* predictors,
                                  unsigned int thread_id,
                                  PredictionStats* stats) const
{
    //static mode? => just ask tracker
    if (!canReestimate())
    {
        assert (!predictors);

        bool fixed;
        bool ok = tracker_.tracker_ptr->predict(mm_predicted, ts, &fixed);

        if (stats)
        {
            stats->num_predictions = 1;
            stats->num_failed      = !ok   ? 1 : 0;
            stats->num_fixed       = fixed ? 1 : 0;
        }
        
        return ok;
    }

    assert(!predictors || predictors->isInit()); //!predictors must be init!
    assert(!predictors || thread_id < predictors->size()); //!thread id must be in range!
    assert(!needsReestimate()); //!no predictions if chain is out of date!
    assert(!updates_.empty());  //!no prediction from empty chains!

    //predicts from a single index
    auto predictFromIndex = [ & ] (int idx)
    {
        assert(idx >= 0);

        const auto& update = updates_[ idx ];

        assert(update.mm_id >= 0);
        assert(update.init);

        auto& p = predictors ? predictors->predictor(thread_id) : *predictor_.estimator_ptr;
        
        bool ref_changed = true;
        
        if (!predictors)
            ref_changed = !predictor_.ref_mm_id.has_value() || update.mm_id != predictor_.ref_mm_id.value();

        //predict
        bool fixed;
        bool ok = ref_changed ? p.kalmanPrediction(mm_predicted, update.kalman_update, ts, &fixed) :
                                p.kalmanPrediction(mm_predicted, ts, &fixed);

        if (stats)
        {
            stats->num_predictions = 1;
            stats->num_failed      = !ok   ? 1 : 0;
            stats->num_fixed       = fixed ? 1 : 0;
        }

        //remember current mm id?
        if (!predictors)
            predictor_.ref_mm_id = update.mm_id;

        return ok;
    };

    //predicts from an interval
    auto predictFromInterval =  [ & ] (int idx0, int idx1)
    {
        assert(idx0 >= 0 && idx1 >= 0);

        const auto& update0 = updates_[ idx0 ];
        const auto& update1 = updates_[ idx1 ];

        assert(update0.mm_id >= 0);
        assert(update1.mm_id >= 0);
        assert(update0.init);
        assert(update1.init);

        auto& p = predictors ? predictors->predictor(thread_id) : *predictor_.estimator_ptr;

        //@TODO: interpolate
        size_t num_fixed;
        bool ok = p.kalmanPrediction(mm_predicted, update0.kalman_update, update1.kalman_update, ts, &num_fixed);

        if (stats)
        {
            stats->num_predictions = 2;
            stats->num_failed      = !ok ? 2 : 0;
            stats->num_fixed       = num_fixed;
        }

        if (!predictors)
            predictor_.ref_mm_id.reset();

        return ok;
    };

    if (settings_.prediction_mode == Settings::PredictionMode::LastUpdate ||
        settings_.prediction_mode == Settings::PredictionMode::NearestBefore)
    {
        //find reference update
        int idx =  (settings_.prediction_mode == Settings::PredictionMode::LastUpdate ? lastIndex() : predictionRefIndex(ts));

        //predict
        return predictFromIndex(idx);
    }
    else if (settings_.prediction_mode == Settings::PredictionMode::Interpolate)
    {
        //find reference interval
        auto iv = predictionRefInterval(ts);
        assert(iv.first >= 0 || iv.second >= 0); //!updates not empty => non-empty interval needs to exist!

        //check distance to interval borders
        bool first_ok  = false;
        bool second_ok = false;

        if (iv.first >= 0 && (ts - updates_[ iv.first ].t) <= settings_.prediction_max_tdiff)
            first_ok = true;
        if (iv.second >= 0 && (updates_[ iv.second ].t - ts) <= settings_.prediction_max_tdiff)
            second_ok = true;

        //!assured beforehand by canPredict()!
        assert(first_ok || second_ok);

        //predict
        if (first_ok && second_ok)
            return predictFromInterval(iv.first, iv.second);
        else if (first_ok)
            return predictFromIndex(iv.first);
        else if (second_ok)
            return predictFromIndex(iv.second);
    }

    return false;
}

/**
*/
bool KalmanChain::predictPosClose(boost::posix_time::ptime timestamp, 
                                  double max_lat_lon_dist) const
{
    //@TODO
    return true;
}

/**
 * Reinits the tracker to the given stored update.
*/
bool KalmanChain::reinit(int idx) const
{
    assert(canReestimate());
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].init); //!no freshly added measurements!
    assert(updates_[ idx ].mm_id >= 0);

    const auto& update = updates_[ idx ];

    //update is the currently tracked update => nothing to do
    if (!tracker_.tracked_mm_id.has_value() || update.mm_id != tracker_.tracked_mm_id.value())
    {
        //if (settings_.verbosity > 0)
        //    loginf << "KalmanChain: reinit: Reinit at idx=" << idx << " t=" << Utils::Time::toString(updates_[ idx ].kalman_update.t);

        //reinit tracker
        tracker_.tracker_ptr->reset();
        if (!tracker_.tracker_ptr->track(update.kalman_update))
            return false; //should not happen

        tracker_.tracked_mm_id = update.mm_id;

        assert(tracker_.tracker_ptr->currentTime() == update.t);
    }

    assert(tracker_.tracker_ptr->currentTime() == update.t);

    return true;
}

/**
 * Removes the update.
*/
void KalmanChain::removeUpdate(int idx)
{
    const auto& update_tbr = updates_.at(idx);

    //reset now outdated measurement id
    if (tracker_.tracked_mm_id.has_value() && update_tbr.mm_id == tracker_.tracked_mm_id.value())
        tracker_.tracked_mm_id.reset();

    if (predictor_.ref_mm_id.has_value() && update_tbr.mm_id == predictor_.ref_mm_id.value())
        predictor_.ref_mm_id.reset();

    //erase update
    updates_.erase(updates_.begin() + idx);
}

/**
 * Keeps track of the range of inserted yet uninitialized indices. 
*/
void KalmanChain::addReesimationIndex(int idx)
{
    assert(idx >= 0);
    assert(canReestimate());

    fresh_indices_.push_back(idx);
}

/**
 * Keeps track of the range of inserted yet uninitialized indices. 
*/
void KalmanChain::addReesimationIndexRange(int idx0, int idx1)
{
    assert(idx0 >= 0 && idx1 >= idx0);
    assert(canReestimate());

    if (idx0 == idx1)
    {
        fresh_indices_.push_back(idx0);
        return;
    }

    size_t n_old = fresh_indices_.size();
    size_t n     = idx1 - idx0 + 1;
    size_t n_new = n_old + n;

    fresh_indices_.resize(n_new);

    for (size_t i = n_old, idx = idx0; i < n_new; ++i, ++idx)
        fresh_indices_[ i ] = idx;
}

/**
*/
void KalmanChain::resetReestimationIndices()
{
    fresh_indices_.clear();
}

/**
*/
bool KalmanChain::canReestimate() const
{
    return (settings_.mode == Settings::Mode::DynamicInserts);
}

/**
 * Checks if new measurements have been added and a reestimation is needed.
*/
bool KalmanChain::needsReestimate() const
{
    return !fresh_indices_.empty();
}

/**
 * Reestimates the kalman state of a measurement based on the current tracker state.
*/
bool KalmanChain::reestimate(int idx, 
                             KalmanEstimator::StepResult* res)
{
    assert(canReestimate());
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].mm_id >= 0);

    auto&       update = updates_[ idx ];
    const auto& mm     = getMeasurement(update.mm_id);

    //check fetched mm's time against update
    assert(update.t == mm.t);

    if (!tracker_.tracker_ptr->track(mm, true, res))
        return false;

    tracker_.tracker_ptr->currentState().value().minimalInfo(update.kalman_update);
    update.init = true;

    tracker_.tracked_mm_id = update.mm_id;

    assert(tracker_.tracker_ptr->currentTime() == update.t);

    return true;
}

/**
 * Reestimates the kalman state of a measurement based on the current tracker state.
 * Additionally returns a residual as norm of difference in covariance matrices before and after the reestimate.
*/
bool KalmanChain::reestimate(int idx, 
                             double& d_state_sqr, 
                             double& d_cov_sqr, 
                             KalmanEstimator::StepResult* res)
{
    assert(canReestimate());
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].mm_id >= 0);
    assert(updates_[ idx ].init); //!no freshly added measurements!

    auto x0 = updates_[ idx ].kalman_update.x;
    auto P0 = updates_[ idx ].kalman_update.P;

    if (!reestimate(idx, res))
        return false;

    const auto& x1 = updates_[ idx ].kalman_update.x;
    const auto& P1 = updates_[ idx ].kalman_update.P;

    d_state_sqr = (x0 - x1).squaredNorm();
    d_cov_sqr   = (P0.diagonal() - P1.diagonal()).squaredNorm(); // = squared frobenius norm

    return true;
}

/**
 * Reestimates the chain after new measurements have been added, based on various criteria.
*/
bool KalmanChain::reestimate(UpdateStats* stats)
{
    if (stats)
    {
        *stats = {};
        stats->set = true;
    }

    if (!needsReestimate() || !canReestimate())
        return true;

    int n       = count();
    int n_fresh = (int)fresh_indices_.size();

    if (n_fresh > 1)
        std::sort(fresh_indices_.begin(), fresh_indices_.end());

    bool is_last = (n_fresh == 1 && fresh_indices_[ 0 ] == lastIndex());

    std::vector<int> tbr;

    auto reestimateRange = [ & ] (int idx_start, int idx_end)
    {
        if (stats)
            ++stats->num_fresh;

        if (idx_end < 0)
            idx_end = n;

        int  idx_cutoff = std::min(idx_end, idx_start + 1 + settings_.reestim_max_updates); // cutoff range based on settings
        auto tstart     = updates_[ idx_start ].t;

        if (!is_last && settings_.verbosity >= 2)
            loginf << "KalmanChain: reestimate: reestimating range [" << idx_start << "," << idx_end << "): "
                   << "idx cutoff = " << idx_cutoff; 

        //reinit tracker
        if (idx_start == 0)
        {
            //first uninit index is start of chain => start from scratch
            tracker_.reset();
        }
        else
        {
            //reinit to last valid update
            bool ok = reinit(idx_start - 1);
            assert(ok); // !reinit should always succeed! 
        }

        //iterate over indices, reestimate, and stop if one of the criteria hits
        int reestimations = 0;
        double d_state_sqr, d_cov_sqr;
        for (int idx = idx_start; idx < idx_cutoff; ++idx)
        {
            auto& u = updates_[ idx ];

            //max reestimation duration hit?
            assert(u.t >= tstart);
            if (u.t - tstart > settings_.reestim_max_duration)
                break;

            //we check the norm starting from the extended range
            bool check_norm = (idx > idx_start);

            //reestim mm
            KalmanEstimator::StepResult res;
            bool ok = check_norm ? reestimate(idx, d_state_sqr, d_cov_sqr, &res) : reestimate(idx, &res);

            if (stats)
            {
                ++stats->num_updated;

                if (ok)
                    ++stats->num_valid;
                else if (res == KalmanEstimator::StepResult::FailStepTooSmall)
                    ++stats->num_skipped;
                else
                    ++stats->num_failed;
            }

            if (!ok)
            {
                //reestimate (=kalman step) failed => collect update for removal
                tbr.push_back(idx);
                continue;
            }

            ++reestimations;

            if (settings_.verbosity >= 2 && check_norm)
               loginf << "KalmanChain: reestimate:   " 
                      << " idx_start = " << idx_start << " idx = " << idx
                      << " ds = " << std::sqrt(d_state_sqr) << " / " << std::sqrt(settings_.reestim_residual_state_sqr)
                      << " dc = " << std::sqrt(d_cov_sqr  ) << " / " << std::sqrt(settings_.reestim_residual_cov_sqr  );

            //check residuals, small changes? => stop reestimation 
            if (check_norm && 
                d_state_sqr <= settings_.reestim_residual_state_sqr && 
                d_cov_sqr   <= settings_.reestim_residual_cov_sqr)
                break;
        }

        return reestimations;
    };

    int reestimations = 0;
    
    //reestimate from each fresh index up to the next one at max
    for (int i = 0; i < n_fresh; ++i)
    {
        int idx      = fresh_indices_[ i ];
        int idx_next = i < n_fresh - 1 ? fresh_indices_[ i + 1 ] : n;

        int ret = reestimateRange(idx, idx_next);
        assert(ret >= 0);

        reestimations += ret;
    }

    resetReestimationIndices();

    //remove any failed (=skipped) updates
    for (auto itr = tbr.rbegin(); itr != tbr.rend(); ++itr)
    {
        if (settings_.verbosity >= 2)
            logwrn << "KalmanChain: reestimate: removing update " << *itr;

        removeUpdate(*itr);
    }

    if (settings_.verbosity >= 2 && !is_last)
        loginf << "KalmanChain: reestimate: Refreshed " << reestimations << " measurement(s)";

    //had to remove error-step updates?
    bool ok = tbr.empty();

    return ok;
}


} // reconstruction

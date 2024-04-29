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

#include "logger.h"
#include "timeconv.h"

namespace reconstruction
{

/**
*/
KalmanChain::KalmanChain()
:   tracker_(new KalmanOnlineTracker)
{
    reset();
}

/**
*/
KalmanChain::~KalmanChain() = default;

/**
 * Resets the chain.
*/
void KalmanChain::reset()
{
    updates_.clear();
    tracker_->reset();

    resetReestimationIndices();

    tracked_update_ = -1;
}

/**
 * Checks if the chain has been initialized.
*/
bool KalmanChain::isInit() const
{
    return tracker_->isInit();
}

/**
 * Initializes the chain by passing a kalman interface.
*/
void KalmanChain::init(std::unique_ptr<KalmanInterface>&& interface)
{
    tracker_->init(std::move(interface));
}

/**
 * Initializes the chain to the given kalman type.
*/
void KalmanChain::init(kalman::KalmanType ktype)
{
    tracker_->init(ktype);
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
KalmanEstimator::Settings& KalmanChain::estimatorSettings()
{
    return tracker_->settings();
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
    if (ts < updates_.begin()->measurement.t)
        return { -1, 0 };

    //end of range?
    if (ts >= updates_.rbegin()->measurement.t)
        return { lastIndex(), -1 };

    //find interval via upper bound
    auto it = std::upper_bound(updates_.begin(), updates_.end(), ts, 
        [ & ] (const boost::posix_time::ptime& t,
               const Update& u) { return t < u.measurement.t; } );

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
 * Adds a new measurement to the end of the chain.
*/
bool KalmanChain::add(const Measurement& mm, bool reestim)
{
    assert(updates_.empty() || mm.t >= updates_.rbegin()->measurement.t);
    
    addReesimationIndex(count());
    updates_.emplace_back(mm);

    if (reestim)
        return reestimate();

    return true;
}

/**
 * Adds new measurements to the end of the chain.
*/
bool KalmanChain::add(const std::vector<Measurement>& mms, bool reestim)
{
    if (mms.empty())
        return true;

    //presort for safety
    std::vector<Measurement> mms_sorted = mms;
    std::sort(mms_sorted.begin(), mms_sorted.end(), [ & ] (const Measurement& mm0, const Measurement& mm1) { return mm0.t < mm1.t; });

    //check correct time ordering
    if (!updates_.empty())
        assert(mms.begin()->t >= updates_.rbegin()->measurement.t);

    int idx_min = count();
    updates_.insert(updates_.end(), mms_sorted.begin(), mms_sorted.end());
    int idx_max = lastIndex();

    addReesimationIndex(idx_min);
    addReesimationIndex(idx_max);

    if (reestim)
        return reestimate();

    return true;
}

/**
 * Inserts a new measurement at the given index.
*/
void KalmanChain::insert(int idx, const Measurement& mm)
{
    if (idx < 0)
    {
        //just add to end
        add(mm, false);
        return;
    }

    //insert
    assert(idx <= count());
    updates_.insert(updates_.begin() + idx, Update(mm));

    addReesimationIndex(idx);
}

/**
 * Inserts a new measurement into the chain.
*/
bool KalmanChain::insert(const Measurement& mm, bool reestim)
{
    int idx = insertionIndex(mm.t);
    insert(idx, mm);

    if (reestim)
        return reestimate();

    return true;
}

/**
 * Inserts new measurements into the chain.
*/
bool KalmanChain::insert(const std::vector<Measurement>& mms, bool reestim)
{
    for (const auto& mm : mms)
        insert(mm, false);

    if (reestim)
        return reestimate();

    return true;
}

/**
 * Checks if a prediction is possible at the given timestamp.
*/
bool KalmanChain::canPredict(const boost::posix_time::ptime& ts) const
{
    if (updates_.empty())
        return false;

    //find reference update
    int idx = predictionRefIndex(ts);
    assert(idx >= 0);

    //check time diff
    auto t0 = updates_[ idx ].kalman_update.t;

    boost::posix_time::time_duration diff = (ts >= t0 ? ts - t0 : t0 - ts);
    
    if (diff > settings_.max_prediction_tdiff)
        return false;

    return true;
}

/**
 * Predicts the given timestamp from the nearest existing update in the chain.
*/
bool KalmanChain::predict(Measurement& mm_predicted,
                          const boost::posix_time::ptime& ts) const
{
    assert(!needsReestimate()); //!no predictions if chain is out of date!
    assert(!updates_.empty());  //!no prediction from empty chains!

    //find reference update
    int idx = predictionRefIndex(ts);
    assert(idx >= 0);

    //reinit tracker to ref update
    if (!reinit(idx))
        return false;

    //predict
    return tracker_->predict(mm_predicted, ts);
}

/**
 * Predicts the given timestamp from the currently tracked update in the chain.
*/
bool KalmanChain::predictFromLastState(Measurement& mm_predicted,
                                       const boost::posix_time::ptime& ts) const
{
    assert(!needsReestimate()); //!no predictions if chain is out of date!
    assert(!updates_.empty());  //!no prediction from empty chains!

    //reinit tracker to last update
    if (!reinit(lastIndex()))
        return false;

    //predict
    return tracker_->predict(mm_predicted, ts);
}

/**
 * Reinits the tracker to the given stored update.
*/
bool KalmanChain::reinit(int idx) const
{
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].init); //!no freshly added measurements!

    //update is the currently tracked update => nothing to do
    if (idx == tracked_update_)
        return true;

    if (settings_.verbosity > 0)
        loginf << "KalmanChain: reinit: Reinit at idx=" << idx << " t=" << Utils::Time::toString(updates_[ idx ].kalman_update.t);

    //reinit tracker
    tracker_->reset();
    if (!tracker_->track(updates_[ idx ].kalman_update))
        return false;

    tracked_update_ = idx;

    return true;
}

/**
 * Keeps track of the range of inserted yet uninitialized indices. 
*/
void KalmanChain::addReesimationIndex(int idx)
{
    assert(idx >= 0);

    fresh_indices_.push_back(idx);
}

/**
*/
void KalmanChain::resetReestimationIndices()
{
    fresh_indices_.clear();
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
bool KalmanChain::reestimate(int idx)
{
    assert(idx >= 0 && idx < count());

    if (!tracker_->track(updates_[ idx ].measurement))
        return false;

    updates_[ idx ].kalman_update = tracker_->currentState().value();
    updates_[ idx ].init = true;
    tracked_update_ = idx;

    return true;
}

/**
 * Reestimates the kalman state of a measurement based on the current tracker state.
 * Additionally returns a residual as norm of difference in covariance matrices before and after the reestimate.
*/
bool KalmanChain::reestimate(int idx, double& d_state_sqr, double& d_cov_sqr)
{
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].init); //!no freshly added measurements!

    auto x0 = updates_[ idx ].kalman_update.state.x;
    auto P0 = updates_[ idx ].kalman_update.state.P;

    if (!reestimate(idx))
        return false;

    const auto& x1 = updates_[ idx ].kalman_update.state.x;
    const auto& P1 = updates_[ idx ].kalman_update.state.P;

    d_state_sqr = (x0 - x1).squaredNorm();
    d_cov_sqr   = (P0.diagonal() - P1.diagonal()).squaredNorm(); // = squared frobenius norm

    return true;
}

/**
 * Reestimates the chain after new measurements have been added, based on various criteria.
*/
bool KalmanChain::reestimate()
{
    if (!needsReestimate())
        return true;

    int n       = count();
    int n_fresh = (int)fresh_indices_.size();

    if (n_fresh > 1)
        std::sort(fresh_indices_.begin(), fresh_indices_.end());

    auto reestimateRange = [ & ] (int idx_start, int idx_end)
    {
        if (idx_end < 0)
            idx_end = n;

        int  idx_cutoff = std::min(idx_end, idx_start + 1 + settings_.max_reestim_updates); // cutoff range based on settings
        auto tstart     = updates_[ idx_start ].measurement.t;

        //reinit tracker
        if (idx_start == 0)
        {
            //first uninit index is start of chain => start from scratch
            tracker_->reset();
        }
        else
        {
            //reinit to last valid update
            if (!reinit(idx_start - 1))
                return -1;
        }

        //iterate over indices, reestimate, and stop if one of the criteria hits
        int reestimations = 0;
        double d_state_sqr, d_cov_sqr;
        for (int idx = idx_start; idx < idx_cutoff; ++idx)
        {
            auto& u = updates_[ idx ];

            //max reestimation duration hit?
            assert(u.measurement.t >= tstart);
            if (u.measurement.t - tstart > settings_.max_reestim_duration)
                break;

            //we check the norm starting from the extended range
            bool check_norm = (idx >= idx_start + 1);

            //reestim mm
            bool ok = check_norm ? reestimate(idx, d_state_sqr, d_cov_sqr) : reestimate(idx);
            if (!ok)
                return -1;

            ++reestimations;

            if (settings_.verbosity > 1 && check_norm)
                loginf << "KalmanChain: reestimate:   " 
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
        if (ret < 0)
            return false;

        reestimations += ret;
    }

    resetReestimationIndices();

    if (settings_.verbosity > 0)
        loginf << "KalmanChain: reestimate: Refreshed " << reestimations << " measurement(s)";

    return true;
}

} // reconstruction

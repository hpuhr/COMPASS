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
*/
void KalmanChain::reset()
{
    updates_.clear();
    tracker_->reset();

    resetReestimationIndices();

    tracked_update_ = -1;
}

/**
*/
bool KalmanChain::isInit() const
{
    return tracker_->isInit();
}

/**
*/
void KalmanChain::init(std::unique_ptr<KalmanInterface>&& interface)
{
    tracker_->init(std::move(interface));
}

/**
*/
void KalmanChain::init(kalman::KalmanType ktype)
{
    tracker_->init(ktype);
}

/**
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
void KalmanChain::add(const std::vector<Measurement>& mms)
{
    if (mms.empty())
        return;

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
}

/**
 * Inserts a new measurement at the given index.
*/
void KalmanChain::insert(int idx, const Measurement& mm)
{
    if (idx < 0)
    {
        //just add to end
        add(mm);
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
void KalmanChain::insert(const Measurement& mm)
{
    int idx = insertionIndex(mm.t);
    insert(idx, mm);
}

/**
 * Inserts new measurements into the chain.
*/
void KalmanChain::insert(const std::vector<Measurement>& mms)
{
    for (const auto& mm : mms)
        insert(mm);
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

    if (idx < reestim_min_)
        reestim_min_ = idx;
    if (idx > reestim_max_)
        reestim_max_ = idx;
}

/**
*/
void KalmanChain::resetReestimationIndices()
{
    reestim_min_ = std::numeric_limits<int>::max();
    reestim_max_ = std::numeric_limits<int>::min();
}

/**
 * Checks if new measurements have been added and a reestimation is needed.
*/
bool KalmanChain::needsReestimate() const
{
    return (reestim_min_ <= reestim_max_);
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
bool KalmanChain::reestimate(int idx, double& dp)
{
    assert(idx >= 0 && idx < count());
    assert(updates_[ idx ].init); //!no freshly added measurements!

    auto P0 = updates_[ idx ].kalman_update.state.P;

    if (!reestimate(idx))
        return false;

    const auto& P1 = updates_[ idx ].kalman_update.state.P;

    dp = (P0 - P1).norm(); // = frobenius norm

    return true;
}

/**
*/
bool KalmanChain::reestimate()
{
    if (!needsReestimate())
        return true;

    int n = count();

    assert(reestim_min_ >= 0 && 
           reestim_max_ >= 0 && 
           reestim_min_ < n &&
           reestim_max_ < n);

    int idx_start   = reestim_min_;
    int idx_end_min = reestim_max_ + 1;
    int idx_end_max = std::min(n, idx_end_min + settings_.max_reestim_updates); // extended range based on settings

    auto tstart = updates_[ reestim_max_ ].measurement.t;

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
            return false;
    }

    //iterate over indices and reestimate
    size_t reestimations = 0;
    for (int idx = idx_start; idx < idx_end_max; ++idx)
    {
        auto& u = updates_[ idx ];

        bool in_extended_range = idx >= idx_end_min;

        //max reestimation duration hit?
        if (in_extended_range)
        {
            assert(u.measurement.t >= tstart);
            if (u.measurement.t - tstart > settings_.max_reestim_duration)
                break;
        }

        //we check the norm starting from the extended range
        bool check_norm = in_extended_range;

        //reestim mm
        double dp;
        bool ok = check_norm ? reestimate(idx, dp) : reestimate(idx);

        ++reestimations;

        if (!ok)
            return false;

        //check residuals, small changes? => stop reestimation 
        if (check_norm && dp <= settings_.reestim_residual)
            break;
    }

    resetReestimationIndices();

    if (settings_.verbosity)
        loginf << "KalmanChain: reestimate: Refreshed " << reestimations << " measurement(s)";

    return true;
}

} // reconstruction

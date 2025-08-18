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

#include "jpda.h"
#include "logger.h"

namespace reconstruction
{

/********************************************************************************************
 * JPDABase
 ********************************************************************************************/

/**
 */
JPDABase::JPDABase() = default;

/**
 */
JPDABase::~JPDABase() = default;

/**
 */
void JPDABase::setGatingFunc(const GatingFunc& func)
{
    gating_func_ = func;
}

/**
 */
void JPDABase::addTrack(Track::Estimator& track_estimator,
                    unsigned int utn)
{
    Track t;
    t.estimator = track_estimator;
    t.track_idx = tracks_.size();
    t.utn       = utn;

    tracks_.push_back(t);
}

/**
 */
size_t JPDABase::numTracks() const
{
    return tracks_.size();
}

/**
 */
boost::optional<SingleHypothesis> JPDABase::checkHypothesis(const Track& track, 
                                                            const std::vector<Measurement>& measurements,
                                                            unsigned int measurement_idx) const
{
    assert(track.isValid());

    const auto& mm = measurements.at(measurement_idx);

    //apply external gating
    if (gating_func_ && !gating_func_(track.utn, mm.source_id.value()))
        return {};

    boost::optional<double> mahalanobis;
    boost::optional<double> likelihood;

    Measurement mm_ref;
    bool ok = track.estimator(&mm_ref, nullptr, mm.t);
    if (!ok)
        return {};

    mahalanobis = mm_ref.mahalanobisDistance(mm);
    if (!mahalanobis.has_value())
        return {};

    //apply gating
    if (mahalanobis.value() > settings_.gate_threshold)
        return {};

    likelihood = mm_ref.likelihood(mm);
    if (!likelihood.has_value())
        return {};

    //return valid hypothesis
    SingleHypothesis hyp;
    hyp.track_idx       = track.track_idx;
    hyp.measurement_idx = measurement_idx;
    hyp.probability     = likelihood.value() * settings_.prob_detect;

    return hyp;
}

/**
 */
std::vector<SingleHypothesis> JPDABase::createSingleHypotheses(const std::vector<Measurement>& measurements) const
{
    std::vector<SingleHypothesis> hypotheses;
    
    // For each track-measurement pair, calculate Mahalanobis distance
    // and add to hypotheses if within gating threshold
    for (size_t track_idx = 0; track_idx < tracks_.size(); ++track_idx) 
    {
        const Track& t = tracks_[ track_idx ];
        for (size_t meas_idx = 0; meas_idx < measurements.size(); ++meas_idx) 
        {
            auto h = checkHypothesis(t, measurements, meas_idx);
            if (!h.has_value())
                continue;

            hypotheses.push_back(h.value());
        }
    }
    
    return hypotheses;
}

/**
 */
std::vector<SingleHypothesis> JPDABase::createTrackHypotheses(const Track& track, 
                                                              const std::vector<Measurement>& measurements) const
{
    std::vector<SingleHypothesis> joined;

    size_t hyp_valid = 0;
    
    //add missed hyp
    SingleHypothesis hyp_missed;
    hyp_missed.measurement_idx = -1;
    hyp_missed.track_idx       = track.track_idx;
    hyp_missed.probability     = 1.0 - settings_.prob_detect; //@TODO: gate prob?

    joined.push_back(hyp_missed);

    for (size_t meas_idx = 0; meas_idx < measurements.size(); ++meas_idx) 
    {
        //check if this is a valid hypothesis
        auto h = checkHypothesis(track, measurements, meas_idx);
        if (!h.has_value())
            continue;

        ++hyp_valid;
        joined.push_back(h.value());
    }

    double prob_sum = 0.0;
    
}

/********************************************************************************************
 * JPDAFull
 ********************************************************************************************/

/**
 */
JPDAFull::JPDAFull() = default;

/**
 */
JPDAFull::~JPDAFull() = default;

/**
 */
std::vector<std::vector<std::pair<int, double>>> JPDAFull::calculateMarginalProbabilities(const std::vector<SingleHypothesis>& hypotheses,
                                                                                          size_t num_tracks,
                                                                                          size_t num_measurements) const
{
    auto clutter_prob = 0.0;//settings_.clutter_prob_default;

    // Initialize result with missed detection probabilities
    std::vector<std::vector<std::pair<int, double>>> marginal_probs(num_tracks);
    for (size_t track_idx = 0; track_idx < num_tracks; ++track_idx) 
    {
        // -1 represents missed detection
        marginal_probs[track_idx].emplace_back(-1, clutter_prob);
    }
    
    // Group hypotheses by track
    std::unordered_map<int, std::vector<SingleHypothesis>> track_hypotheses;
    for (const auto& hypothesis : hypotheses)
        track_hypotheses[hypothesis.track_idx].push_back(hypothesis);
    
    // For each track, calculate marginal probabilities
    for (const auto& [track_idx, track_hyps] : track_hypotheses) 
    {
        double total_probability = clutter_prob;
        
        // Sum probabilities for normalization
        for (const auto& hyp : track_hyps) 
            total_probability += hyp.probability;
        
        // Normalize and add to result
        for (const auto& hyp : track_hyps) 
        {
            double normalized_prob = hyp.probability / total_probability;
            marginal_probs[track_idx].emplace_back((int)hyp.measurement_idx, normalized_prob);
        }
        
        // Normalize missed detection probability
        marginal_probs[track_idx][0].second = clutter_prob / total_probability;
    }
    
    return marginal_probs;
}

// std::vector<JointHypothesis> JPDAFull::createFullJointHypothesis(const std::vector<SingleHypothesis>& hypotheses) const
// {

// }

/**
 */
bool JPDAFull::associate(TrackAssignments& assignments,
                         std::vector<Measurement>& measurements)
{
    // assignments = {};
    // assignments.resize(measurements.size());

    // if (numTracks() == 0 || measurements.empty()) 
    //     return true;
    
    // // generate hypotheses
    // auto hypotheses = JPDABase::createSingleHypotheses(measurements);
    // if (hypotheses.empty()) 
    //     return true;

    // auto joint_hyp = createFullJointHypothesis(hypotheses);
    // if (joint_hyp.empty())
    //     return true;

    
    
    // // calculate marginal association probabilities
    // auto marg_probs = calculateMarginalProbabilities(hypotheses, tracks_.size(), measurements.size());
    // auto 




    return true;
}

} // namespace reconstruction

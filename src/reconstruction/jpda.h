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

#include "reconstruction_defs.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iostream>

namespace reconstruction
{

struct Measurement;
struct PredictionResult;

/**
 */
struct Track
{
    typedef std::function<bool(Measurement*, kalman::GeoProbState*, const boost::posix_time::ptime&)>  Estimator;
    typedef std::function<bool(PredictionComparison&, const kalman::GeoProbState&, int)>               Comparator;

    bool isValid() const 
    { 
        if (!estimator ) return false;
        if (!comparator) return false;
        return true;
    }

    unsigned int track_idx;
    unsigned int utn;
    Estimator    estimator;
    Comparator   comparator;
};

/**
 */
struct SingleHypothesis
{
public:
    int          track_idx       = -1;
    int          measurement_idx = -1;
    double       probability     = 0.0;
};

/**
 */
struct JPDASettings
{
    double gate_threshold = 9.0; // default threshold for mahalanobis based distance gating
    double prob_detect    = 0.9; // probability of detection
};

/**
 */
class JPDABase
{
public:
    typedef boost::optional<std::pair<unsigned int, double>> TrackAssignment;
    typedef std::vector<TrackAssignment>                     TrackAssignments;
    typedef std::function<bool(unsigned int, unsigned long)> GatingFunc;

    JPDABase();
    virtual ~JPDABase();

    JPDASettings& settings() { return settings_; }

    void setGatingFunc(const GatingFunc& func);

    void addTrack(Track::Estimator& track_estimator,
                  unsigned int utn);
    size_t numTracks() const;
    
    virtual bool associate(TrackAssignments& assignments,
                           std::vector<Measurement>& measurements) = 0;
protected:
    boost::optional<SingleHypothesis> checkHypothesis(const Track& track, 
                                                      const std::vector<Measurement>& measurements,
                                                      unsigned int measurement_idx) const;
    std::vector<SingleHypothesis> createTrackHypotheses(const Track& track, 
                                                        const std::vector<Measurement>& measurements) const;
    std::vector<SingleHypothesis> createSingleHypotheses(const std::vector<Measurement>& measurements) const;

    GatingFunc         gating_func_;
    std::vector<Track> tracks_;
    JPDASettings       settings_;
};

/**
 */
class JPDAFull : public JPDABase
{
public:
    JPDAFull();
    virtual ~JPDAFull();

    bool associate(TrackAssignments& assignments,
                   std::vector<Measurement>& measurements) override final;
private:
    /**
     * @brief Calculate marginal association probabilities from hypotheses
     * 
     * @param hypotheses Vector of hypotheses
     * @param num_tracks Number of tracks
     * @param num_measurements Number of measurements
     * @return std::vector<std::vector<std::pair<int, double>>> Marginal probabilities for each track
     */
    std::vector<std::vector<std::pair<int, double>>> calculateMarginalProbabilities(const std::vector<SingleHypothesis>& hypotheses,
                                                                                    size_t num_tracks,
                                                                                    size_t num_measurements) const;

    //std::vector<JointHypothesis> createFullJointHypothesis(const std::vector<SingleHypothesis>& hypotheses) const;
};

} // namespace reconstruction

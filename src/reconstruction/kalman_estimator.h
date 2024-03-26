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

#include <memory>

namespace reconstruction
{

class KalmanInterface;
class KalmanProjectionHandler;

/**
 * Provides needed data structures for kalman and means to retrieve data from kalman state. 
 * Derive for a certain kalman variant.
 */
class KalmanEstimator
{
public:
    struct Settings
    {
        enum ReinitFlags
        {
            ReinitCheckTime     = 1 << 0, //checks for reinit using a timestep threshold
            ReinitCheckDistance = 1 << 1  //checks for reinit using a metric distance-based threshold
        };

        double Q_var;                    // variance of kalman process

        size_t min_chain_size = 2;       // minimum number of consecutive kalman updates without reinit            (0 = do not check)
        double max_distance   = 50000.0; // maximum allowed distance of consecutive measurements in meters         (0 = do not check)
        double min_dt         = 0;       // minimum allowed time difference of consecutive measurements in seconds (0 = do not check)
        double max_dt         = 11.0;    // maximum allowed time difference of consecutive measurements in seconds (0 = do not check)

        int reinit_check_flags = ReinitCheckTime; // checks used for reinitialization of kalman

        double max_proj_distance_cart = 20000.0; // maximum distance from the current map projection origin in meters 
                                                 // before changing the projection center

        reconstruction::Uncertainty default_uncert;

        double          resample_dt          = 2.0;                       // resampling step size in seconds
        double          resample_Q_var       = 10.0;                      // resampling process noise
        StateInterpMode resample_interp_mode = StateInterpMode::BlendVar; // kalman state interpolation mode used during resampling

        int verbosity = 0;
    };

    enum class ReinitState
    {
        ReinitNotNeeded = 0,
        ReinitDistance,
        ReinitTime
    };

    enum class StepResult
    {
        Success = 0,
        FailStepTooSmall,
        FailKalmanError
    };

    typedef std::vector<kalman::KalmanUpdate> Updates;
    typedef std::function<void(Updates&, size_t, size_t)> ChainFunc;

    KalmanEstimator();
    virtual ~KalmanEstimator();

    void init(std::unique_ptr<KalmanInterface>&& interface);
    
    void kalmanInit(kalman::KalmanUpdate& update,
                    Measurement& mm);
    void kalmanInit(const kalman::KalmanUpdate& update);
    StepResult kalmanStep(kalman::KalmanUpdate& update,
                          Measurement& mm);

    void storeUpdates(std::vector<Reference>& refs,
                      const std::vector<kalman::KalmanUpdate>& updates) const;
    
    void smoothUpdates(std::vector<kalman::KalmanUpdate>& updates);
    void interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                       std::vector<kalman::KalmanUpdate>& updates);

    Settings& settings() { return settings_; }

private:
    void initMeasurement(kalman::KalmanUpdate& update,
                         Measurement& mm);
    ReinitState needsReinit(const Measurement& mm) const;
    void reinit(kalman::KalmanUpdate& update,
                const Measurement& mm);
    bool step(kalman::KalmanUpdate& update,
              const Measurement& mm);
    void checkProjection(kalman::KalmanUpdate& update);

    void executeChainFunc(Updates& updates, const ChainFunc& func);

    bool interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                       const std::vector<kalman::KalmanUpdate>& updates,
                       size_t idx0,
                       size_t idx1,
                       double dt_sec,
                       double min_dt_sec,
                       double Q_var,
                       StateInterpMode interp_mode,
                       KalmanProjectionHandler& proj_handler) const;
    
    void storeUpdate(Reference& ref, 
                     const kalman::KalmanUpdate& update,
                     KalmanProjectionHandler& phandler) const;

    std::unique_ptr<KalmanInterface>         kalman_interface_;
    std::unique_ptr<KalmanProjectionHandler> proj_handler_;

    Settings settings_;

    double max_distance_sqr_;
};

} // reconstruction

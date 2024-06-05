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

#include <memory>

namespace reconstruction
{

class KalmanInterface;
class KalmanProjectionHandler;

/**
 * Kalman-filter-based state estimator.
 * - operates on a KalmanInterface to interact with a certain kalman variant
 * - provides kalman features such as
 *     - initialization of kalman filter
 *     - integration of measurements into kalman filter
 *     - smoothing of kalman states
 *     - interpolation of kalman states
 *     - predictions based on a certain kalman state
 *     - handling projection into local map crs
 *     - etc.
 * - holds a current state - generally not multithreading safe
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

        enum class StepFailStrategy
        {
            Assert = 0,      //assert if step fails
            Reinit,          //reinit kalman if step fails       
            ReturnInvalid    //return error and set update to invalid
        };

        double Q_var       = 900.0;   // variance of kalman process (30*30)
        double R_var_undef = HighVar; // high variance for undefined values (1000*1000)

        size_t min_chain_size = 2;       // minimum number of consecutive kalman updates without reinit            (0 = do not check)
        double max_distance   = 50000.0; // maximum allowed distance of consecutive measurements in meters         (0 = do not check)
        double min_dt         = 0.0;     // minimum allowed time difference of consecutive measurements in seconds (0 = do not check)
        double max_dt         = 11.0;    // maximum allowed time difference of consecutive measurements in seconds (0 = do not check)

        double min_pred_dt = 0.0;

        int reinit_check_flags = ReinitCheckTime; // checks used for reinitialization of kalman

        double max_proj_distance_cart = 20000.0; // maximum distance from the current map projection origin in meters 
                                                 // before changing the projection center

        double          resample_dt          = 2.0;                       // resampling step size in seconds
        double          resample_Q_var       = 100.0;                     // resampling process noise (10*10)
        StateInterpMode resample_interp_mode = StateInterpMode::BlendVar; // kalman state interpolation mode used during resampling

        StepFailStrategy step_fail_strategy = StepFailStrategy::ReturnInvalid;

        bool track_velocities      = true;
        bool track_accelerations   = true;

        int verbosity = 0;

        reconstruction::Uncertainty default_uncert; //default uncertainties used if none are provided in the measurement
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

    bool isInit() const;

    void init(std::unique_ptr<KalmanInterface>&& interface);
    void init(kalman::KalmanType ktype);
    
    void kalmanInit(kalman::KalmanUpdate& update,
                    const Measurement& mm);
    void kalmanInit(const kalman::KalmanUpdate& update);
    void kalmanInit(const kalman::KalmanUpdateMinimal& update);

    StepResult kalmanStep(kalman::KalmanUpdate& update,
                          const Measurement& mm);
    
    bool kalmanPrediction(Measurement& mm,
                          double dt) const;
    bool kalmanPrediction(Measurement& mm,
                          const boost::posix_time::ptime& ts) const;
    bool kalmanPrediction(Measurement& mm,
                          const kalman::KalmanUpdate& ref_update,
                          const boost::posix_time::ptime& ts);
    bool kalmanPrediction(Measurement& mm,
                          const kalman::KalmanUpdateMinimal& ref_update,
                          const boost::posix_time::ptime& ts);
    bool kalmanPrediction(Measurement& mm,
                          const kalman::KalmanUpdate& ref_update0,
                          const kalman::KalmanUpdate& ref_update1,
                          const boost::posix_time::ptime& ts);
    bool kalmanPrediction(Measurement& mm,
                          const kalman::KalmanUpdateMinimal& ref_update0,
                          const kalman::KalmanUpdateMinimal& ref_update1,
                          const boost::posix_time::ptime& ts);

    void storeUpdates(std::vector<Reference>& refs,
                      const std::vector<kalman::KalmanUpdate>& updates) const;
    
    bool smoothUpdates(std::vector<kalman::KalmanUpdate>& updates) const;
    bool interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                       std::vector<kalman::KalmanUpdate>& updates,
                       size_t* num_steps_failed = nullptr) const;

    kalman::KalmanState currentState() const;
    const boost::posix_time::ptime& currentTime() const;
    Measurement currentStateAsMeasurement() const;

    std::string asString(const std::string& prefix = "") const;

    static std::unique_ptr<KalmanInterface> createInterface(kalman::KalmanType ktype, 
                                                            bool track_velocity = true, 
                                                            bool track_accel = true);
    Settings& settings() { return settings_; }

    static const double HighStdDev;
    static const double HighVar;

private:
    void initMeasurement(kalman::KalmanUpdate& update,
                         const Measurement& mm);
    ReinitState needsReinit(const Measurement& mm) const;
    void reinit(kalman::KalmanUpdate& update,
                const Measurement& mm);
    bool step(kalman::KalmanUpdate& update,
              const Measurement& mm);
    void checkProjection(kalman::KalmanUpdate& update);

    void executeChainFunc(Updates& updates, const ChainFunc& func) const;

    bool interpUpdates(std::vector<kalman::KalmanUpdate>& interp_updates,
                       const std::vector<kalman::KalmanUpdate>& updates,
                       size_t idx0,
                       size_t idx1,
                       double dt_sec,
                       double min_dt_sec,
                       double Q_var,
                       StateInterpMode interp_mode,
                       KalmanProjectionHandler& proj_handler,
                       size_t* num_steps_failed = nullptr) const;
    bool interpUpdates(kalman::KalmanUpdateMinimal& update_interp,
                       const kalman::KalmanUpdateMinimal& update0,
                       const kalman::KalmanUpdateMinimal& update1,
                       const boost::posix_time::ptime& ts,
                       double min_dt_sec,
                       double Q_var,
                       StateInterpMode interp_mode,
                       KalmanProjectionHandler& proj_handler) const;
    
    void storeUpdate(Reference& ref, 
                     const kalman::KalmanUpdate& update,
                     KalmanProjectionHandler& phandler) const;

    reconstruction::Uncertainty defaultUncert(const Measurement& mm) const;

    bool checkPrediction(const Measurement& mm) const;

    std::unique_ptr<KalmanInterface>         kalman_interface_;
    std::unique_ptr<KalmanProjectionHandler> proj_handler_;

    Settings settings_;

    double max_distance_sqr_;
};

} // reconstruction

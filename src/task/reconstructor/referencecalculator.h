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
#include "kalman_estimator.h"

/**
*/
struct ReferenceCalculatorSettings
{
    reconstruction::KalmanEstimator::Settings kalmanEstimatorSettings() const
    {
        reconstruction::KalmanEstimator::Settings settings;

        settings.min_chain_size = min_chain_size;
        settings.min_dt         = min_dt;
        settings.max_dt         = max_dt;
        settings.max_distance   = max_distance;

        settings.Q_var       = Q_std       * Q_std;
        settings.R_var_undef = R_std_undef * R_std_undef;
        
        settings.resample_dt          = resample_dt;
        settings.resample_Q_var       = resample_Q_std * resample_Q_std;
        settings.resample_interp_mode = resample_blend_mode;

        settings.max_proj_distance_cart = max_proj_distance_cart;

        settings.verbosity = activeVerbosity() >= 2 ? activeVerbosity() - 1 : 0;

        settings.step_fail_strategy = allow_invalid_updates ? reconstruction::KalmanEstimator::Settings::StepFailStrategy::ReturnInvalid :
                                                              reconstruction::KalmanEstimator::Settings::StepFailStrategy::Assert;

        settings.fix_predictions        = fix_predictions;
        settings.fix_predictions_interp = fix_predictions_interp;

        return settings;
    }

    int activeVerbosity() const
    {
        return (multithreading ? 0 : verbosity);
    }

    int verbosity = 0;

    kalman::KalmanType kalman_type = kalman::KalmanType::UMKalman2D;

    //default noise
    double Q_std       = 30.0;
    double R_std_undef = reconstruction::KalmanEstimator::HighStdDev;

    //reinit related
    int    min_chain_size = 2;
    double min_dt         = 0.01;
    double max_dt         = 11.0;
    double max_distance   = 50000.0;

    bool smooth_rts = true;

    //result resampling related
    bool                            resample_result     = true;
    double                          resample_Q_std      = 10.0;
    double                          resample_dt         = 2.0;
    reconstruction::StateInterpMode resample_blend_mode = reconstruction::StateInterpMode::BlendVar;

    //dynamic projection change
    double max_proj_distance_cart = 20000.0;

    //systrack resampling related
    bool   resample_systracks        = true; // resample system tracks using spline interpolation
    double resample_systracks_dt     = 1.0;  // resample interval in seconds
    double resample_systracks_max_dt = 30.0; // maximum timestep to interpolate in seconds

    bool multithreading        = true;
    bool allow_invalid_updates = true;

    bool fix_predictions        = true;
    bool fix_predictions_interp = true;

    //debug options
    bool compat_mode     = false;
    int  max_slice_index = -1;
};

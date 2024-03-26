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

#include <vector>

class SimpleReconstructor;

namespace dbContent 
{
    class ReconstructorTarget;
}

/**
*/
class SimpleReferenceCalculator
{
public:
    struct Settings
    {
        enum ReconstructorType
        {
            Rec_UMKalman2D = 0,
            Rec_AMKalman2D
        };

        int verbosity = 0;

        ReconstructorType rec_type = ReconstructorType::Rec_UMKalman2D;

        //default noise
        double Q_std = 30.0;

        //reinit related
        int    min_chain_size = 2;
        double min_dt         = 0.0;
        double max_dt         = 11.0;
        double max_distance   = 50000.0;

        bool smooth_rts = true;

        //result resampling related
        bool   resample_result = false;
        double resample_Q_std  = 10.0;
        double resample_dt     = 2.0;

        //dynamic projection change
        double max_proj_distance_cart = 20000.0;

        //systrack resampling related
        bool   resample_systracks        = true; // resample system tracks using spline interpolation
        double resample_systracks_dt     = 1.0;  // resample interval in seconds
        double resample_systracks_max_dt = 30.0; // maximum timestep to interpolate in seconds

        bool multithreading = true;
    };

    typedef std::vector<reconstruction::Measurement>                       Measurements;
    typedef std::multimap<boost::posix_time::ptime, unsigned long>         TargetReports;
    typedef std::map<unsigned int, std::vector<reconstruction::Reference>> References;

    SimpleReferenceCalculator(SimpleReconstructor& reconstructor);
    virtual ~SimpleReferenceCalculator();

    void prepareForNextSlice();
    bool computeReferences();

    Settings& settings() { return settings_; }
    
    static std::vector<std::vector<reconstruction::Measurement>> splitMeasurements(const Measurements& measurements,
                                                                                   double max_dt);
private:
    struct TargetReferences
    {
        unsigned int utn;

        std::vector<reconstruction::Measurement> measurements;
        std::vector<kalman::KalmanUpdate>        updates;
        std::vector<reconstruction::Reference>   references;

        boost::optional<kalman::KalmanUpdate> init_update;
        boost::optional<size_t>               start_index;
    };

    enum class InitRecResult
    {
        NoMeasurements = 0,
        NoStartIndex,
        Success
    };

    void reset();
    void updateInterpOptions();

    void generateMeasurements();
    void generateTargetMeasurements(const dbContent::ReconstructorTarget& target);
    void generateLineMeasurements(const dbContent::ReconstructorTarget& target,
                                  unsigned int dbcontent_id,
                                  unsigned int sensor_id,
                                  unsigned int line_id,
                                  const TargetReports& target_reports);
    
    void addMeasurements(unsigned int utn,
                         unsigned int dbcontent_id, 
                         Measurements& measurements);
    void preprocessMeasurements(unsigned int dbcontent_id, 
                                Measurements& measurements);
    void interpolateMeasurements(Measurements& measurements, 
                                 const reconstruction::InterpOptions& options) const;
    
    void reconstructMeasurements();
    InitRecResult initReconstruction(TargetReferences& refs);
    void reconstructMeasurements(TargetReferences& refs);

    void updateReferences();

    int verbosity() const;

    boost::posix_time::ptime getJoinThreshold() const;

    SimpleReconstructor& reconstructor_;

    Settings settings_;

    std::map<unsigned int, TargetReferences>              references_;
    std::map<unsigned int, reconstruction::InterpOptions> interp_options_;
};

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

#include "referencecalculator.h"

#include <vector>

class ReconstructorBase;

namespace dbContent 
{
    class ReconstructorTarget;
}

class ViewPointGenAnnotation;

namespace reconstruction
{
    class KalmanEstimator;
}

class QColor;

/**
*/
class SimpleReferenceCalculator
{
public:
    typedef ReferenceCalculatorSettings Settings;

    typedef std::vector<reconstruction::Measurement>                       Measurements;
    typedef std::multimap<boost::posix_time::ptime, unsigned long>         TargetReports;
    typedef std::map<unsigned int, std::vector<reconstruction::Reference>> References;

    SimpleReferenceCalculator(ReconstructorBase& reconstructor);
    virtual ~SimpleReferenceCalculator();

    void prepareForNextSlice();
    bool computeReferences();

    void reset();

    Settings& settings() { return settings_; }
    
    static std::vector<std::vector<reconstruction::Measurement>> splitMeasurements(const Measurements& measurements,
                                                                                   double max_dt);
private:
    struct TargetReferences
    {
        void reset();
        void resetCounts();

        unsigned int utn;

        std::vector<reconstruction::Measurement> measurements;
        std::vector<kalman::KalmanUpdate>        updates;
        std::vector<kalman::KalmanUpdate>        updates_smooth;
        std::vector<reconstruction::Reference>   references;

        boost::optional<kalman::KalmanUpdate> init_update;
        boost::optional<size_t>               start_index;

        size_t num_updates             = 0;
        size_t num_updates_valid       = 0;
        size_t num_updates_failed      = 0;
        size_t num_updates_skipped     = 0;
        size_t num_smooth_steps_failed = 0;
        size_t num_interp_steps_failed = 0;

        ViewPointGenAnnotation* annotation = nullptr;
    };

    enum class InitRecResult
    {
        NoMeasurements = 0,
        NoStartIndex,
        Success
    };

    void resetDataStructs();
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

    boost::posix_time::ptime getJoinThreshold() const;

    ViewPointGenAnnotation* addAnnotations(const reconstruction::KalmanEstimator& estimator, 
                                           ViewPointGenAnnotation& root,
                                           const std::string& name,
                                           const std::vector<kalman::KalmanUpdate>& updates,
                                           const std::vector<reconstruction::Measurement> measurements,
                                           const std::vector<unsigned int>& used_mms,
                                           size_t offs,
                                           const boost::optional<boost::posix_time::ptime>& t0,
                                           const boost::optional<boost::posix_time::ptime>& t1,
                                           const QColor& base_color,
                                           bool add_positions = true,
                                           bool add_accuracies = true,
                                           bool add_velocities = true,
                                           bool add_acceleration = true,
                                           bool add_input_connections = false) const;

    ReconstructorBase& reconstructor_;

    Settings settings_;

    int slice_idx_ = -1;

    std::map<unsigned int, TargetReferences>              references_;
    std::map<unsigned int, reconstruction::InterpOptions> interp_options_;
};

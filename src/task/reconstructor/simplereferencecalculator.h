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
        int verbosity = 0;

        std::map<unsigned int, reconstruction::InterpOptions> interp_options;
    };

    typedef std::vector<reconstruction::Measurement> Measurements;

    SimpleReferenceCalculator(SimpleReconstructor& reconstructor);
    virtual ~SimpleReferenceCalculator();

    bool computeReferences();

    Settings& settings() { return settings_; }
    
    static std::vector<std::vector<reconstruction::Measurement>> splitMeasurements(const Measurements& measurements,
                                                                                   double max_dt);
private:
    struct CalcRefJob
    {
        unsigned int utn;
        std::vector<reconstruction::Measurement>* measurements;
        std::vector<kalman::KalmanUpdate>         updates;
    };

    void reset();

    void generateMeasurements();
    void generateTargetMeasurements(const dbContent::ReconstructorTarget& target);
    void generateLineMeasurements(const dbContent::ReconstructorTarget& target,
                                  unsigned int dbcontent_id,
                                  unsigned int sensor_id,
                                  unsigned int line_id,
                                  const std::multimap<boost::posix_time::ptime, unsigned long>& target_reports);
    
    void addMeasurements(unsigned int utn,
                         unsigned int dbcontent_id, 
                         Measurements& measurements);
    void preprocessMeasurements(unsigned int dbcontent_id, 
                                Measurements& measurements);
    void interpolateMeasurements(Measurements& measurements, 
                                 const reconstruction::InterpOptions& options) const;
    
    void reconstructMeasurements();
    void reconstructMeasurements(CalcRefJob& job);

    SimpleReconstructor& reconstructor_;

    Settings settings_;

    std::map<unsigned int, std::vector<reconstruction::Measurement>> measurements_;
};

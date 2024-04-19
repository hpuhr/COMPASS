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

#include "dbcontentaccessor.h"
#include "accuracyestimatorbase.h"
#include "configurable.h"
#include "targetreportdefs.h"
#include "reconstructortarget.h"
#include "referencecalculator.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <map>
#include <string>
#include <memory>

namespace dbContent
{
class VariableSet;
class ReconstructorTarget;
}

namespace reconstruction
{
struct Measurement;
}

class Buffer;
class ReconstructorTask;

typedef std::pair<boost::posix_time::ptime, boost::posix_time::ptime> TimeWindow; // min, max

/**
 */
class ReconstructorBase : public Configurable
{
public:
    struct BaseSettings
    {
        boost::posix_time::time_duration sliceDuration() const
        {
            return boost::posix_time::minutes(slice_duration_in_minutes);
        }
        boost::posix_time::time_duration outdatedDuration() const
        {
            return boost::posix_time::minutes(outdated_duration_in_minutes);
        }

        // output
        std::string  ds_name {"CalcRef"};
        unsigned int ds_sac  {255};
        unsigned int ds_sic  {1};
        unsigned int ds_line {0};

        // slicing
        unsigned int slice_duration_in_minutes    {10};
        unsigned int outdated_duration_in_minutes {2};

        bool delete_all_calc_reftraj {false};
    };

    typedef std::map<std::string, std::shared_ptr<Buffer>> Buffers;

    ReconstructorBase(const std::string& class_id, 
                      const std::string& instance_id,
                      ReconstructorTask& task, 
                      std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator,
                      unsigned int default_line_id = 0);
    virtual ~ReconstructorBase();

    bool hasNextTimeSlice();
    TimeWindow getNextTimeSlice();

    int numSlices() const;

    bool processSlice(Buffers&& buffers);

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const = 0;

    virtual void reset();

    BaseSettings& baseSettings() { return base_settings_; }
    const BaseSettings& baseSettings() const { return base_settings_; }

    ReferenceCalculatorSettings& referenceCalculatorSettings() { return ref_calc_settings_; }
    const ReferenceCalculatorSettings& referenceCalculatorSettings() const { return ref_calc_settings_; }

    void createMeasurement(reconstruction::Measurement& mm, const dbContent::targetReport::ReconstructorInfo& ri);

    const dbContent::TargetReportAccessor& accessor(dbContent::targetReport::ReconstructorInfo& tr) const;

    virtual void updateWidgets() = 0;

    ReconstructorTask& task() const;

protected:
    friend class dbContent::ReconstructorTarget;
    friend class SimpleReferenceCalculator;
    friend class ComplexAccuracyEstimator;
    friend class ProbabilisticAssociator;
    friend class DataSourceAccuracyEstimator;
    friend class ADSBAccuracyEstimator;
    friend class MLATAccuracyEstimator;
    friend class RadarAccuracyEstimator;
    friend class TrackerAccuracyEstimator;

    ReconstructorTask& task_;

    std::map<unsigned int, dbContent::TargetReportAccessor> accessors_;

    std::unique_ptr<AccuracyEstimatorBase> acc_estimator_;

    Buffers buffers_;
    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    boost::posix_time::ptime current_slice_begin_;
    boost::posix_time::ptime next_slice_begin_;
    boost::posix_time::ptime timestamp_min_, timestamp_max_;
    bool first_slice_ {false};

    boost::posix_time::ptime remove_before_time_;
    boost::posix_time::ptime write_before_time_;

    std::map<unsigned long, dbContent::targetReport::ReconstructorInfo> target_reports_;
    // all sources, record_num -> base info
    std::multimap<boost::posix_time::ptime, unsigned long> tr_timestamps_;
    // all sources sorted by time, ts -> record_num
    std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::vector<unsigned long>>>> tr_ds_;
    // dbcontent id -> ds_id -> line id -> record_num, sorted by ts

    std::map<unsigned int, dbContent::ReconstructorTarget> targets_; // utn -> tgt

    BaseSettings base_settings_;

    void removeOldBufferData(); // remove all data before current_slice_begin_
    virtual bool processSlice_impl() = 0;

    void clearOldTargetReports();
    void createTargetReports();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> createAssociations();
    void saveAssociations(std::map<unsigned int, std::map<unsigned long, unsigned int>> associations);
    void saveReferences();
    void saveTargets();

private:
    ReferenceCalculatorSettings ref_calc_settings_;
};

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
class KalmanChainPredictors;
}

class Buffer;
class ReconstructorTask;

typedef std::pair<boost::posix_time::ptime, boost::posix_time::ptime> TimeWindow; // min, max

class ReconstructorBaseSettings
{
  public:
    ReconstructorBaseSettings() {};

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
    unsigned int ds_sac  {REC_DS_SAC};
    unsigned int ds_sic  {REC_DS_SIC};
    unsigned int ds_line {0};

    // slicing
    unsigned int slice_duration_in_minutes    {15};
    unsigned int outdated_duration_in_minutes {2};

    bool delete_all_calc_reftraj {true};
    bool ignore_calculated_references {true};

    // maximum time difference in target reports to do comparisons
    float max_time_diff_ {5}; // sec
    // maximum altitude difference to consider mode c the "same"
    float max_altitude_diff_ {300.0};
    // maximimum time difference between track updates, otherwise considered new track
    double track_max_time_diff_ {300.0};

    // compare targets related
    double target_prob_min_time_overlap_ {0.1};
    unsigned int target_min_updates_ {5};
    double target_max_positions_dubious_verified_rate_ {0.5};
    double target_max_positions_dubious_unknown_rate_ {0.3};

    static const unsigned int REC_DS_SAC = 255;
    static const unsigned int REC_DS_SIC = 1;
};

/**
 */
class ReconstructorBase : public Configurable
{
  public:

    struct DataSlice
    {
        unsigned int slice_count_;
        boost::posix_time::ptime slice_begin_;
        boost::posix_time::ptime next_slice_begin_;
        boost::posix_time::ptime timestamp_min_, timestamp_max_;
        bool first_slice_ {false};
        bool is_last_slice_ {false};

        boost::posix_time::ptime remove_before_time_;
        boost::posix_time::ptime write_before_time_;

        std::map<std::string, std::shared_ptr<Buffer>> data_;
        bool loading_done_ {false}; // set if data_ is set correctly and can be processed

        std::map<std::string, std::shared_ptr<Buffer>> assoc_data_;
        std::map<std::string, std::shared_ptr<Buffer>> reftraj_data_;

        bool processing_done_ {false}; // set if assoc_data_, reftraj_data_ are set correctly and can be written
        bool write_done_ {false}; // set if data has been written
    };

    typedef std::map<std::string, std::shared_ptr<Buffer>> Buffers;

    ReconstructorBase(const std::string& class_id, 
                      const std::string& instance_id,
                      ReconstructorTask& task, 
                      std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator,
                      ReconstructorBaseSettings& base_settings,
                      unsigned int default_line_id = 0);
    virtual ~ReconstructorBase();

    bool hasNextTimeSlice();
    std::unique_ptr<ReconstructorBase::DataSlice> getNextTimeSlice();

    int numSlices() const;

    void processSlice();
    ReconstructorBase::DataSlice& currentSlice();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const = 0;

    virtual void reset();

    virtual ReconstructorBaseSettings& settings() { return base_settings_; };

    ReferenceCalculatorSettings& referenceCalculatorSettings() { return ref_calc_settings_; }
    const ReferenceCalculatorSettings& referenceCalculatorSettings() const { return ref_calc_settings_; }

    void createMeasurement(reconstruction::Measurement& mm, const dbContent::targetReport::ReconstructorInfo& ri);
    void createMeasurement(reconstruction::Measurement& mm, unsigned long rec_num);

    const dbContent::TargetReportAccessor& accessor(dbContent::targetReport::ReconstructorInfo& tr) const;

    virtual void updateWidgets() = 0;

    ReconstructorTask& task() const;

    void cancel();
    bool isCancelled() { return cancelled_; };

    void saveTargets();

    // our data structures
    std::map<unsigned long, dbContent::targetReport::ReconstructorInfo> target_reports_;
    // all sources, record_num -> base info
    std::multimap<boost::posix_time::ptime, unsigned long> tr_timestamps_;
    // all sources sorted by time, ts -> record_num
    std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::vector<unsigned long>>>> tr_ds_;
    // dbcontent id -> ds_id -> line id -> record_num, sorted by ts

    std::map<unsigned int, dbContent::ReconstructorTarget> targets_; // utn -> tgt

    std::unique_ptr<AccuracyEstimatorBase> acc_estimator_;

    bool processing() const;

    virtual const std::map<unsigned int, std::map<unsigned int,
                                                  std::pair<unsigned int, unsigned int>>>& assocAounts() const = 0;
    // ds_id -> dbcont id -> (assoc, not assoc cnt)

    virtual void createAdditionalAnnotations() {}

    reconstruction::KalmanChainPredictors& chainPredictors();

protected:

    ReconstructorTask& task_;

    std::map<unsigned int, dbContent::TargetReportAccessor> accessors_;

    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    ReconstructorBaseSettings& base_settings_;

    void removeOldBufferData(); // remove all data before current_slice_begin_
    virtual void processSlice_impl() = 0;

    void clearOldTargetReports();
    void createTargetReports();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> createAssociations();
    std::map<std::string, std::shared_ptr<Buffer>> createAssociationBuffers(
        std::map<unsigned int, std::map<unsigned long, unsigned int>> associations);
    std::map<std::string, std::shared_ptr<Buffer>> createReferenceBuffers();

    bool cancelled_ {false};

  private:
    void initChainPredictors();

    ReferenceCalculatorSettings ref_calc_settings_;

    unsigned int slice_cnt_ {0};
    boost::posix_time::ptime current_slice_begin_;
    boost::posix_time::ptime next_slice_begin_;
    boost::posix_time::ptime timestamp_min_, timestamp_max_;
    bool first_slice_ {false};

    boost::posix_time::ptime remove_before_time_;
    boost::posix_time::ptime write_before_time_;

    bool processing_ {false};

    std::unique_ptr<reconstruction::KalmanChainPredictors> chain_predictors_;
};

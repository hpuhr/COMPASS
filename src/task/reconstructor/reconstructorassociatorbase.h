#pragma once

#include "targetreportdefs.h"
#include "reconstructortarget.h"
#include "reconstructorbase.h"

class ReconstructorAssociatorBase
{
  public:

    enum DistanceClassification
    {
        //Distance_Unknown=0, // check not possible
        Distance_Good=0,
        Distance_Acceptable,
        Distance_Dubious,
        Distance_NotOK
    };

    ReconstructorAssociatorBase();

    virtual void associateNewData();
    virtual void reset();

    const std::map<unsigned int, std::map<unsigned int,
                                          std::pair<unsigned int, unsigned int>>>& assocAounts() const;

    struct AssociationOption
    {
        AssociationOption(){}

        AssociationOption(bool usable, unsigned int other_utn, unsigned int num_updates,
                          bool associate, float avg_distance)
            : usable_(usable), other_utn_(other_utn), num_updates_(num_updates),
            associate_based_on_secondary_attributes_(associate), avg_distance_(avg_distance)
        {}

        bool usable_ {false};
        unsigned int other_utn_ {0};
        unsigned int num_updates_{0};

        bool associate_based_on_secondary_attributes_ {false};
        float avg_distance_{0};
    };

    virtual bool canGetPositionOffsetTR(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, bool use_max_distance=true) = 0;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffsetTR(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target,
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) = 0;

    virtual boost::optional<bool> isTargetAccuracyAcceptable(
        double tgt_est_std_dev, unsigned int utn, const boost::posix_time::ptime& ts, bool do_debug) = 0;

  protected:

    boost::posix_time::time_duration max_time_diff_;

    std::vector<unsigned long> unassoc_rec_nums_;

    unsigned int num_merges_ {0};

    boost::posix_time::time_duration time_assoc_trs_;
    boost::posix_time::time_duration time_assoc_new_utns_;
    boost::posix_time::time_duration time_retry_assoc_trs_;

    void associateTargetReports();
    void associateTargetReports(std::set<unsigned int> dbcont_ids);

    void selfAssociateNewUTNs();
    void retryAssociateTargetReports();
    void associate(dbContent::targetReport::ReconstructorInfo& tr, int utn);
    virtual void postAssociate(dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn) {};
    //void checkACADLookup();
    void countUnAssociated();

    int findUTNFor (dbContent::targetReport::ReconstructorInfo& tr);

            // tries to find existing utn for target report, based on mode a/c and position, -1 if failed
    int findUTNByModeACPos (const dbContent::targetReport::ReconstructorInfo& tr);

    std::vector<unsigned int> findUTNsForTarget (unsigned int utn, const std::set<unsigned int>& utns_to_ignore);

    //unsigned int createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr);



    virtual bool canGetPositionOffsetTargets(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1) = 0;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffsetTargets(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1,
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) = 0;

    virtual boost::optional<bool> checkTrackPositionOffsetAcceptable (
        dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn,
        bool secondary_verified, bool do_debug) = 0;
    // empty if not possible, else check passed or failed returned
    virtual void doOutlierDetection (
        dbContent::targetReport::ReconstructorInfo& tr,
        unsigned int utn, bool do_debug) {};

    virtual boost::optional<std::pair<bool, double>> calculatePositionOffsetScore (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) = 0;
    // empty if not possible, else check passed + score (smaller is better) returned
    virtual std::tuple<DistanceClassification, double> checkPositionOffsetScore
        (double distance_m, double sum_stddev_est, bool secondary_verified, bool target_acccuracy_acceptable) = 0;

    virtual bool isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified) = 0;

    virtual ReconstructorBase& reconstructor() = 0;

    std::map<unsigned int, std::map<unsigned int, std::pair<unsigned int, unsigned int>>> assoc_counts_;
    // ds_id -> dbcont id -> (assoc, unassoc cnt)
};


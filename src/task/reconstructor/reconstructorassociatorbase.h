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

  protected:

    boost::posix_time::time_duration max_time_diff_;

    std::vector<unsigned int> utn_vec_;
    std::map<unsigned int, unsigned int> acad_2_utn_; // acad dec -> utn
    std::map<std::string, unsigned int> acid_2_utn_; // acid trim -> utn

            // ds_id -> line id -> track num -> utn, last tod
    std::map<unsigned int, std::map<unsigned int,
                                    std::map<unsigned int,
                                             std::pair<unsigned int, boost::posix_time::ptime>>>> tn2utn_;

    std::vector<unsigned int> unassoc_rec_nums_;

    unsigned int num_merges_ {0};

    void associateTargetReports();
    void associateTargetReports(std::set<unsigned int> dbcont_ids);

    void selfAccociateNewUTNs();
    void retryAssociateTargetReports();
    void associate(dbContent::targetReport::ReconstructorInfo& tr, int utn);
    virtual void postAssociate(dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn) {};
    void checkACADLookup();
    void countUnAssociated();

    int findUTNFor (dbContent::targetReport::ReconstructorInfo& tr,
                   const std::set<unsigned long>& debug_rec_nums,
                   const std::set<unsigned int>& debug_utns);

            // tries to find existing utn for target report, based on mode a/c and position, -1 if failed
    int findUTNByModeACPos (const dbContent::targetReport::ReconstructorInfo& tr,
                           const std::vector<unsigned int>& utn_vec,
                           const std::set<unsigned long>& debug_rec_nums,
                           const std::set<unsigned int>& debug_utns);

    int findUTNForTarget (unsigned int utn,
                         const std::set<unsigned long>& debug_rec_nums,
                         const std::set<unsigned int>& debug_utns);

    unsigned int createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr);

    virtual bool canGetPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target) = 0;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, bool do_debug) = 0;
    virtual bool canGetPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1) = 0;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1,
        int thread_id,
        bool do_debug) = 0;

    virtual boost::optional<bool> checkPositionOffsetAcceptable (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn,
        bool secondary_verified, bool do_debug) = 0;
    // empty if not possible, else check passed or failed returned
    virtual boost::optional<std::pair<bool, double>> calculatePositionOffsetScore (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) = 0;
    // empty if not possible, else check passed + score (smaller is better) returned
    virtual std::tuple<DistanceClassification, double> checkPositionOffsetScore
        (double distance_m, double sum_stddev_est, bool secondary_verified) = 0;

    virtual boost::optional<bool> isTargetAccuracyAcceptable(
        double tgt_est_std_dev, unsigned int utn, const boost::posix_time::ptime& ts) = 0;
    virtual bool isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified) = 0;

    virtual ReconstructorBase& reconstructor() = 0;

    std::map<unsigned int, std::map<unsigned int, std::pair<unsigned int, unsigned int>>> assoc_counts_;
    // ds_id -> dbcont id -> (assoc, unassoc cnt)
};


#pragma once

#include "targetreportdefs.h"
#include "reconstructortarget.h"

class SimpleReconstructor;

class SimpleAssociator
{
  public:
    SimpleAssociator(SimpleReconstructor& reconstructor);

    void associateNewData();

  private:
    SimpleReconstructor& reconstructor_;

    void createReferenceUTNs();

    void createTrackerUTNs();
    void createNonTrackerUTNS();

    // creates tmp tracked targets to be added
    std::map<unsigned int, dbContent::ReconstructorTarget> createTrackedTargets(
        unsigned int dbcont_id, unsigned int ds_id);
    void selfAssociateTrackerUTNs();

    void addTrackerUTNs(const std::string& ds_name, std::map<unsigned int, dbContent::ReconstructorTarget> from_targets,
                        std::map<unsigned int, dbContent::ReconstructorTarget>& to_targets);

    int findContinuationUTNForTrackerUpdate (const dbContent::targetReport::ReconstructorInfo& tr,
                                            const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);
    // tries to find existing utn for tracker update, -1 if failed
    int findUTNForTrackerTarget (const dbContent::ReconstructorTarget& target,
                                const std::map<unsigned int, dbContent::ReconstructorTarget>& targets,
                                int max_utn=-100);
    // tries to find existing utn for target, -1 if failed
    int findUTNForTargetByTA (const dbContent::ReconstructorTarget& target,
                             const std::map<unsigned int, dbContent::ReconstructorTarget>& targets,
                             int max_utn);
    // tries to find existing utn for target by target address, -1 if failed

    std::map<unsigned int, unsigned int> getTALookupMap (
        const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);
};


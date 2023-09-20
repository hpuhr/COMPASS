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

#ifndef CREATEASSOCIATIONSJOB_H
#define CREATEASSOCIATIONSJOB_H

#include "job.h"
#include "dbcontent/dbcontentcache.h"
#include "assoc/targetreport.h"
#include "assoc/target.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time_duration.hpp"

class CreateAssociationsTask;
class DBInterface;
class Buffer;
class DBContent;

class CreateAssociationsJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal(QString status);

public:
    CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                          std::shared_ptr<dbContent::Cache> cache);

    virtual ~CreateAssociationsJob();

    virtual void run();

    std::map<std::string, std::pair<unsigned int, unsigned int> > associationCounts() const;

protected:
    //static bool in_appimage_;

    CreateAssociationsTask& task_;
    DBInterface& db_interface_;
    std::shared_ptr<dbContent::Cache> cache_;

    std::map<std::string, std::map<unsigned int, std::vector<Association::TargetReport>>> target_reports_;
    //dbo name->ds_id->trs

    std::map<std::string,
        std::map<unsigned long,
            std::tuple<unsigned int, std::vector<std::pair<std::string, unsigned long>>>>> associations_;
    // dbcontent -> rec_num -> <utn, src rec_nums (dbcontent, rec_num)>

    std::map<std::string, std::pair<unsigned int,unsigned int>> association_counts_; // dbcontent -> total, assoc cnt

    void createTargetReports();
    std::map<unsigned int, Association::Target> createReferenceUTNs();

    void createTrackerUTNs(std::map<unsigned int, Association::Target>& sum_targets);

    void createNonTrackerUTNS(std::map<unsigned int, Association::Target>& targets);
    void createAssociations();
    void saveAssociations();
    void saveTargets(std::map<unsigned int, Association::Target>& targets);

    void removePreviousAssociations();

    std::map<unsigned int, Association::Target> createTrackedTargets(const std::string& dbcontent_name, unsigned int ds_id);
    void cleanTrackerUTNs(std::map<unsigned int, Association::Target>& targets);
    std::map<unsigned int, Association::Target> selfAssociateTrackerUTNs(
            std::map<unsigned int, Association::Target>& targets);
    // tries to associate each utn to all others, returns new target list

    void markDubiousUTNs(std::map<unsigned int, Association::Target>& targets);
    // marks weird utns as, only for final utns, must have calculated speeds

    void addTrackerUTNs(const std::string& ds_name, std::map<unsigned int, Association::Target> from_targets,
                        std::map<unsigned int, Association::Target>& to_targets);

    int findContinuationUTNForTrackerUpdate (const Association::TargetReport& tr,
                                             const std::map<unsigned int, Association::Target>& targets);
    // tries to find existing utn for tracker update, -1 if failed
    int findUTNForTrackerTarget (const Association::Target& target,
                                 const std::map<unsigned int, Association::Target>& targets);
    // tries to find existing utn for target, -1 if failed
    int findUTNForTargetByTA (const Association::Target& target,
                              const std::map<unsigned int, Association::Target>& targets);
    // tries to find existing utn for target by target address, -1 if failed

    std::map<unsigned int, unsigned int> getTALookupMap (
            const std::map<unsigned int, Association::Target>& targets);
};

#endif // CREATEASSOCIATIONSJOB_H

#ifndef CALCULATEREFERENCESJOB_H
#define CALCULATEREFERENCESJOB_H

#include "job.h"
#include "dbcontent/dbcontentcache.h"
#include "calculatereferencestarget.h"

//#include "json.h"

//#include "boost/date_time/posix_time/ptime.hpp"
//#include "boost/date_time/posix_time/posix_time_duration.hpp"

class CalculateReferencesTask;
class DBContent;

class CalculateReferencesJob : public Job
{
    Q_OBJECT

public slots:
    void insertDoneSlot();

signals:
    void statusSignal(QString status);

public:
    CalculateReferencesJob(CalculateReferencesTask& task, 
                           std::shared_ptr<dbContent::Cache> cache);
    virtual ~CalculateReferencesJob();

    nlohmann::json viewPointsJSON() const { return viewpoint_json_; }

    virtual void run();

protected:
    CalculateReferencesTask& task_;
    std::shared_ptr<dbContent::Cache> cache_;

    std::vector<std::unique_ptr<CalculateReferences::Target>> targets_;

    std::shared_ptr<Buffer> result_;

    std::map<unsigned int, unsigned int> reftraj_counts_; // utn -> cnt

    bool insert_done_ {false};

    nlohmann::json viewpoint_json_;

    void createTargets();
    void filterPositionUsage(std::map<unsigned int, std::unique_ptr<CalculateReferences::Target>>& target_map);
    void finalizeTargets();
    void calculateReferences();
    void writeReferences();
};

#endif // CALCULATEREFERENCESJOB_H

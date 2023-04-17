#ifndef CALCULATEREFERENCESJOB_H
#define CALCULATEREFERENCESJOB_H

#include "job.h"
#include "dbcontent/dbcontentcache.h"
#include "calculatereferencestarget.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time_duration.hpp"

class CalculateReferencesTask;
class DBContent;

class CalculateReferencesJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal(QString status);

public:
    CalculateReferencesJob(CalculateReferencesTask& task, std::shared_ptr<dbContent::Cache> cache);
    virtual ~CalculateReferencesJob();

    virtual void run();

protected:
    CalculateReferencesTask& task_;
    std::shared_ptr<dbContent::Cache> cache_;

    std::vector<std::unique_ptr<CalculateReferences::Target>> targets_;

    void createTargets();
    void finalizeTargets();
    void calculateReferences();
};

#endif // CALCULATEREFERENCESJOB_H

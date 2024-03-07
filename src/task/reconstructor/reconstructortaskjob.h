#pragma once

#include "job.h"
//#include "dbcontent/dbcontentcache.h"

class ReconstructorTask;
class DBInterface;
class Buffer;
class DBContent;

class ReconstructorTaskJob: public Job
{
    Q_OBJECT

  signals:
//    void statusSignal(QString status);


  public:
    ReconstructorTaskJob(ReconstructorTask& task); //DBInterface& db_interface,
                         //std::shared_ptr<dbContent::Cache> cache);
    virtual ~ReconstructorTaskJob();

    virtual void run();

  protected:
    ReconstructorTask& task_;
    //DBInterface& db_interface_;
    //std::shared_ptr<dbContent::Cache> cache_;
};


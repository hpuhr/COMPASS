#include "reconstructortaskjob.h"

ReconstructorTaskJob::ReconstructorTaskJob(ReconstructorTask& task) //DBInterface& db_interface,
                                           //std::shared_ptr<dbContent::Cache> cache)
    : Job("ReconstructorTaskJob"), task_(task)//, //db_interface_(db_interface),
      //cache_(cache)
{

}

ReconstructorTaskJob::~ReconstructorTaskJob()
{
    logdbg << "ReconstructorTaskJob: dtor";

}

void ReconstructorTaskJob::run()
{
    logdbg << "ReconstructorTaskJob: run: start";

    started_ = true;

    done_ = true;
}

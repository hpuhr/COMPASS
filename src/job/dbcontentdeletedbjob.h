#ifndef DBCONTENTDELETEDBJOB_H
#define DBCONTENTDELETEDBJOB_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include "job.h"

class DBInterface;

class DBContentDeleteDBJob : public Job
{
    Q_OBJECT

public:
    DBContentDeleteDBJob(DBInterface& db_interface, boost::posix_time::ptime before_timestamp);

    virtual ~DBContentDeleteDBJob();

    virtual void run();

protected:
    DBInterface& db_interface_;
    boost::posix_time::ptime before_timestamp_;
};

#endif // DBCONTENTDELETEDBJOB_H

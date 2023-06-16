#ifndef DBCONTENTDELETEDBJOB_H
#define DBCONTENTDELETEDBJOB_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include "job.h"

class DBInterface;

class DBContentDeleteDBJob : public Job
{
    Q_OBJECT

public:
    DBContentDeleteDBJob(DBInterface& db_interface);

    virtual ~DBContentDeleteDBJob();

    void setBeforeTimestamp(boost::posix_time::ptime before_timestamp); // delete everything before ts
    void setSpecificDBContent(const std::string& specific_dbcontent); // delete everything in dbcontent

    virtual void run();

    bool useSpecificDBContent() const;
    bool useBeforeTimestamp() const;

protected:
    DBInterface& db_interface_;

    bool use_before_timestamp_ {false};
    boost::posix_time::ptime before_timestamp_;

    bool use_specific_dbcontent_ {false};
    std::string specific_dbcontent_ {false};
};

#endif // DBCONTENTDELETEDBJOB_H

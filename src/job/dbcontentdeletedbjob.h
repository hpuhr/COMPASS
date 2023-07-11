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
    void setSpecificSacSic(unsigned int sac, unsigned int sic);
    void setSpecificLineId(unsigned int line_id);

    virtual void run();

    bool useSpecificDBContent() const;
    bool useBeforeTimestamp() const;

protected:
    DBInterface& db_interface_;

    bool use_before_timestamp_ {false};
    boost::posix_time::ptime before_timestamp_;

    bool use_specific_dbcontent_ {false};
    std::string specific_dbcontent_ {false};

    bool use_specific_sac_sic_ {false};
    unsigned int specific_sac_ {0};
    unsigned int specific_sic_ {0};

    bool use_specific_line_id_ {false};
    unsigned int specific_line_id_ {0};
};

#endif // DBCONTENTDELETEDBJOB_H

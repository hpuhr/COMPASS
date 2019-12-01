#ifndef DBOREADASSOCIATIONSJOB_H
#define DBOREADASSOCIATIONSJOB_H

#include "job.h"

class DBObject;

class DBOReadAssociationsJob : public Job
{
public:
    DBOReadAssociationsJob(DBObject& dbobject);
    virtual ~DBOReadAssociationsJob();

    virtual void run ();

protected:
    DBObject& dbobject_;
};

#endif // DBOREADASSOCIATIONSJOB_H

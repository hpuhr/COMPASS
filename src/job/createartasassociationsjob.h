#ifndef CREATEARTASASSOCIATIONSJOB_H
#define CREATEARTASASSOCIATIONSJOB_H

#include "job.h"

class DBInterface;

class CreateARTASAssociationsJob : public Job
{
    Q_OBJECT

public:
    CreateARTASAssociationsJob(DBInterface& db_interface);

    virtual ~CreateARTASAssociationsJob();

    virtual void run ();

protected:
    DBInterface& db_interface_;
};

#endif // CREATEARTASASSOCIATIONSJOB_H

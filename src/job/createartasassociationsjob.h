#ifndef CREATEARTASASSOCIATIONSJOB_H
#define CREATEARTASASSOCIATIONSJOB_H

#include "job.h"

class DBInterface;
class Buffer;

class CreateARTASAssociationsJob : public Job
{
    Q_OBJECT

public:
    CreateARTASAssociationsJob(DBInterface& db_interface, std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateARTASAssociationsJob();

    virtual void run ();

protected:
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
};

#endif // CREATEARTASASSOCIATIONSJOB_H

#ifndef CREATEARTASASSOCIATIONSJOB_H
#define CREATEARTASASSOCIATIONSJOB_H

#include "job.h"

class CreateARTASAssociationsTask;
class DBInterface;
class Buffer;

class CreateARTASAssociationsJob : public Job
{
    Q_OBJECT

public:
    CreateARTASAssociationsJob(CreateARTASAssociationsTask& task, DBInterface& db_interface,
                               std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateARTASAssociationsJob();

    virtual void run ();

protected:
    CreateARTASAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    const std::string tracker_dbo_name_{"Tracker"};

    void createUTNS ();

    std::map<unsigned int, unsigned int> track_rec_num_utns_; // track rec num -> utn
};

#endif // CREATEARTASASSOCIATIONSJOB_H

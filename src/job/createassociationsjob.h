#ifndef CREATEASSOCIATIONSJOB_H
#define CREATEASSOCIATIONSJOB_H

#include "job.h"

class CreateAssociationsTask;
class DBInterface;
class Buffer;
class DBObject;

class CreateAssociationsJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal(QString status);

public:
    CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                          std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateAssociationsJob();

    virtual void run();

protected:
    CreateAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::map<unsigned int, unsigned int> ta_2_utn_;
    unsigned int utn_cnt_ {0};

    void createUTNS();
};

#endif // CREATEASSOCIATIONSJOB_H

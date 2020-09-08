#include "createassociationsjob.h"
#include "atsdb.h"
#include "buffer.h"
#include "createassociationstask.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"

using namespace Utils;

CreateAssociationsJob::CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                                             std::map<std::string, std::shared_ptr<Buffer>> buffers)
    : Job("CreateAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{
}

CreateAssociationsJob::~CreateAssociationsJob() {}

void CreateAssociationsJob::run()
{
    logdbg << "CreateAssociationsJob: run: start";

    started_ = true;

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    loginf << "CreateARTASAssociationsJob: run: clearing associations";

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    object_man.removeAssociations();

    // create utns
    emit statusSignal("Creating UTNs");
    createUTNS();

    // save associations
    emit statusSignal("Saving Associations");
    for (auto& dbo_it : object_man)
    {
        loginf << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first
               << " associated " << dbo_it.second->associations().size() << " of "
               << dbo_it.second->count();
        dbo_it.second->saveAssociations();
    }

    object_man.setAssociationsByAll(); // no specific dbo or data source

    stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CreateARTASAssociationsJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";
    done_ = true;
}

void CreateAssociationsJob::createUTNS()
{
    loginf << "CreateAssociationsJob: createUTNS";
}

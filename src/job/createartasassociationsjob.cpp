#include "createartasassociationsjob.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "atsdb.h"

#include "stringconv.h"

using namespace Utils::String;

CreateARTASAssociationsJob::CreateARTASAssociationsJob(DBInterface& db_interface)
: Job("CreateARTASAssociationsJob"), db_interface_(db_interface)
{

}

CreateARTASAssociationsJob::~CreateARTASAssociationsJob()
{

}

void CreateARTASAssociationsJob::run ()
{
    logdbg  << "CreateARTASAssociationsJob: run: start";

    started_ = true;

    boost::posix_time::ptime loading_start_time;
    boost::posix_time::ptime loading_stop_time;

    loading_start_time = boost::posix_time::microsec_clock::local_time();

//    loginf  << "CreateARTASAssociationsJob: run: writing object " << dbobject_.name() << " size " << buffer_->size();
//    assert (buffer_->size());

    for (auto& dbo_it : ATSDB::instance().objectManager())
    {
        logdbg  << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first;

        DBObject* object = dbo_it.second;

        size_t size = object->count();

        for (unsigned int cnt=0; cnt < size; ++cnt)
            object->addAssociation(cnt, cnt);

        object->saveAssociations();
    }


//    db_interface_.insertBuffer(dbobject_.currentMetaTable(), buffer_);
    loading_stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "CreateARTASAssociationsJob: run: done (" << doubleToStringPrecision(load_time, 2) << " s).";
    done_=true;
}

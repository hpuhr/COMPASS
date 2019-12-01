#include "dboreadassociationsjob.h"
#include "dbobject.h"

DBOReadAssociationsJob::DBOReadAssociationsJob(DBObject& dbobject)
    : Job("DBOReadAssociationsJob"), dbobject_(dbobject)
{
    assert (dbobject_.existsInDB());
}

DBOReadAssociationsJob::~DBOReadAssociationsJob()
{

}

void DBOReadAssociationsJob::run ()
{
    loginf << "DBOReadAssociationsJob: run: " << dbobject_.name() << ": start";
    started_ = true;

    dbobject_.loadAssociations ();

    done_=true;

    return;
}

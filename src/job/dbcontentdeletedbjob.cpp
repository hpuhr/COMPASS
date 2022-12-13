#include "dbcontentdeletedbjob.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "logger.h"
#include "compass.h"
#include "util/timeconv.h"

using namespace std;
using namespace Utils;

DBContentDeleteDBJob::DBContentDeleteDBJob(
        DBInterface& db_interface, boost::posix_time::ptime before_timestamp)
    : Job("DBContentDeleteDBJob"), db_interface_(db_interface),
      before_timestamp_(before_timestamp)
{
}

DBContentDeleteDBJob::~DBContentDeleteDBJob() {}

void DBContentDeleteDBJob::run()
{
    logdbg << "DBContentDeleteDBJob: run: start";
    started_ = true;

    if (obsolete_)
    {
        logdbg << "DBContentDeleteDBJob: run: obsolete before prepared";
        done_ = true;
        return;
    }

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& dbcont_it : dbcont_man)
    {
        if (!dbcont_it.second->existsInDB())
            continue;

        logdbg << "DBContentDeleteDBJob: run: deleting dbcontent for " << dbcont_it.first;
        db_interface_.deleteBefore(*dbcont_it.second, before_timestamp_);
    }

    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

    logdbg << "DBContentDeleteDBJob: run: done after " << Time::toString(diff);

    done_ = true;

    return;

}

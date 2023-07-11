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

DBContentDeleteDBJob::DBContentDeleteDBJob(DBInterface& db_interface)
    : Job("DBContentDeleteDBJob"), db_interface_(db_interface)

{
}

DBContentDeleteDBJob::~DBContentDeleteDBJob() {}

void DBContentDeleteDBJob::setBeforeTimestamp(boost::posix_time::ptime before_timestamp)
{
    assert (!use_specific_dbcontent_);
    use_before_timestamp_ = true;
    before_timestamp_ = before_timestamp;
}

void DBContentDeleteDBJob::setSpecificDBContent(const std::string& specific_dbcontent)
{
    assert (!use_before_timestamp_);
    use_specific_dbcontent_ = true;
    specific_dbcontent_ = specific_dbcontent;
}

void DBContentDeleteDBJob::setSpecificSacSic(unsigned int sac, unsigned int sic)
{
    assert (!use_before_timestamp_);
    assert (use_specific_dbcontent_);

    use_specific_sac_sic_ = true;
    specific_sac_ = sac;
    specific_sic_ = sic;
}

void DBContentDeleteDBJob::setSpecificLineId(unsigned int line_id)
{
    assert (!use_before_timestamp_);
    assert (use_specific_dbcontent_);
    assert (use_specific_sac_sic_);

    use_specific_line_id_ = true;
    specific_line_id_ = line_id;
}

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

    if (!(use_before_timestamp_ || use_specific_dbcontent_))
    {
        logerr << "DBContentDeleteDBJob: run: neither before time or dbcontent defined";
        done_ = true;
        return;
    }

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (use_before_timestamp_)
    {
        for (auto& dbcont_it : dbcont_man)
        {
            if (!dbcont_it.second->existsInDB())
                continue;

            logdbg << "DBContentDeleteDBJob: run: deleting dbcontent for " << dbcont_it.first;
            db_interface_.deleteBefore(*dbcont_it.second, before_timestamp_);
        }
    }
    else if (use_specific_dbcontent_)
    {
        if (use_specific_line_id_)
        {
            loginf << "DBContentDeleteDBJob: run: deleting dbcontent for " << specific_dbcontent_
                   << " for specific sac/sic + line";
            assert (dbcont_man.existsDBContent(specific_dbcontent_));
            assert (use_specific_sac_sic_);

            db_interface_.deleteContent(dbcont_man.dbContent(specific_dbcontent_),
                                        specific_sac_, specific_sic_, specific_line_id_);
        }
        else if (use_specific_sac_sic_)
        {
            loginf << "DBContentDeleteDBJob: run: deleting dbcontent for " << specific_dbcontent_
                   << " for specific sac/sic";
            assert (dbcont_man.existsDBContent(specific_dbcontent_));

            db_interface_.deleteContent(dbcont_man.dbContent(specific_dbcontent_),
                                        specific_sac_, specific_sic_);
        }
        else // all
        {
            logdbg << "DBContentDeleteDBJob: run: deleting dbcontent for " << specific_dbcontent_;
            assert (dbcont_man.existsDBContent(specific_dbcontent_));
            db_interface_.deleteAll(dbcont_man.dbContent(specific_dbcontent_));
        }
    }
    else
        assert (false);


    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

    logdbg << "DBContentDeleteDBJob: run: done after " << Time::toString(diff);

    done_ = true;

    return;

}

bool DBContentDeleteDBJob::useSpecificDBContent() const
{
    return use_specific_dbcontent_;
}

bool DBContentDeleteDBJob::useBeforeTimestamp() const
{
    return use_before_timestamp_;
}

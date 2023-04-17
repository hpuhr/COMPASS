#include "calculatereferencestarget.h"
#include "timeconv.h"

using namespace std;
using namespace Utils;
using namespace dbContent;
using namespace dbContent::TargetReport;

namespace CalculateReferences {

Target::Target(unsigned int utn, std::shared_ptr<dbContent::Cache> cache)
    : utn_(utn), cache_(cache)
{

}

void Target::addTargetReport(const std::string& dbcontent_name, unsigned int ds_id,
                             unsigned int line_id, boost::posix_time::ptime timestamp, unsigned int index)
{
    TargetKey key = TargetKey{dbcontent_name, ds_id, line_id};

    if (!chains_.count(key))
        chains_[key].reset(new Chain(cache_, dbcontent_name));

    chains_.at(key)->addIndex(timestamp, index);
}

void Target::finalizeChains()
{
    for (auto& chain_it : chains_)
        chain_it.second->finalize();
}

void Target::calculateReference()
{
    boost::posix_time::ptime ts_begin, ts_end;

    for (auto& chain_it : chains_)
    {
        if (chain_it.second->hasData())
        {
            if (ts_begin.is_not_a_date_time())
            {
                ts_begin = chain_it.second->timeBegin();
                ts_end = chain_it.second->timeEnd();

                assert (!ts_begin.is_not_a_date_time());
                assert (!ts_end.is_not_a_date_time());
            }
            else
            {
                ts_begin = min(ts_begin, chain_it.second->timeBegin());
                ts_end = max(ts_end, chain_it.second->timeEnd());
            }
        }
    }

    DataMapping mapping;

    if (!ts_begin.is_not_a_date_time())
    {
        boost::posix_time::time_duration update_interval = Time::partialSeconds(1.0);

        for (boost::posix_time::ptime ts_current = ts_begin; ts_current <= ts_end; ts_current += update_interval)
        {
            for (auto& chain_it : chains_)
            {
                mapping = chain_it.second->calculateDataMapping(ts_current);
            }

        }
    }

}

unsigned int Target::utn() const
{
    return utn_;
}

} // namespace CalculateReferences

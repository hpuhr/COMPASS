#include "calculatereferencestarget.h"

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

} // namespace CalculateReferences

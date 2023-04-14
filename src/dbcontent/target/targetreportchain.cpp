#include "targetreportchain.h"

namespace dbContent {

namespace TargetReport {

Chain::Chain(std::shared_ptr<dbContent::Cache> cache)
    : cache_(cache)
{

}

Chain::~Chain()
{

}

} // namespace TargetReport

} // namespace dbContent

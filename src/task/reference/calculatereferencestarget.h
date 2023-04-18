#ifndef CALCULATEREFERENCES_TARGET_H
#define CALCULATEREFERENCES_TARGET_H

#include "dbcontent/target/targetreportchain.h"
#include "buffer.h"

#include <tuple>

namespace CalculateReferences {

class Target
{
public:
    typedef std::tuple<std::string, unsigned int, unsigned int> TargetKey;

    Target(unsigned int utn, std::shared_ptr<dbContent::Cache> cache);

    void addTargetReport(const std::string& dbcontent_name, unsigned int ds_id, unsigned int line_id,
                         boost::posix_time::ptime timestamp, unsigned int index);

    void finalizeChains();

    std::shared_ptr<Buffer> calculateReference();

    unsigned int utn() const;

protected:
    unsigned int utn_;
    std::shared_ptr<dbContent::Cache> cache_;

    std::map<TargetKey, std::unique_ptr<dbContent::TargetReport::Chain>> chains_;
};

} // namespace CalculateReferences

#endif // CALCULATEREFERENCES_TARGET_H

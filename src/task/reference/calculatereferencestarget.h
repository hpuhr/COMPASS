#ifndef CALCULATEREFERENCES_TARGET_H
#define CALCULATEREFERENCES_TARGET_H

#include "dbcontent/target/targetreportchain.h"
#include "buffer.h"
#include "json.h"

#include <tuple>

namespace CalculateReferences {

class Target
{
public:
    typedef std::tuple<std::string, unsigned int, unsigned int> TargetKey; // dbcontent_name, ds_id, line_id

    Target(unsigned int utn, 
           std::shared_ptr<dbContent::Cache> cache,
           bool generate_viewpoint = false);

    void addTargetReport(const std::string& dbcontent_name, unsigned int ds_id, unsigned int line_id,
                         boost::posix_time::ptime timestamp, unsigned int index);

    void finalizeChains();

    std::shared_ptr<Buffer> calculateReference();
    nlohmann::json viewpointJSON() const { return viewpoint_json_; }

    unsigned int utn() const;

protected:
    unsigned int                      utn_;
    std::shared_ptr<dbContent::Cache> cache_;
    bool                              generate_viewpoint_;
    nlohmann::json                    viewpoint_json_;
    std::map<TargetKey, std::unique_ptr<dbContent::TargetReport::Chain>> chains_;
};

} // namespace CalculateReferences

#endif // CALCULATEREFERENCES_TARGET_H

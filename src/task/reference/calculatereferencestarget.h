#ifndef CALCULATEREFERENCES_TARGET_H
#define CALCULATEREFERENCES_TARGET_H

#include "dbcontent/target/targetreportchain.h"
#include "buffer.h"

#include <tuple>

class ViewPointGenVP;
class CalculateReferencesTaskSettings;

namespace CalculateReferences {

class Target
{
public:
    typedef std::tuple<std::string, unsigned int, unsigned int> TargetKey; // dbcontent_name, ds_id, line_id

    Target(unsigned int utn, 
           std::shared_ptr<dbContent::DBContentAccessor> accessor);

    void addTargetReport(const std::string& dbcontent_name, 
                         unsigned int ds_id, 
                         unsigned int line_id,
                         boost::posix_time::ptime timestamp, 
                         unsigned int index);
    void finalizeChains();

    std::shared_ptr<Buffer> calculateReference(const CalculateReferencesTaskSettings& settings, 
                                               ViewPointGenVP* gen_view_point = nullptr);

    unsigned int utn() const;

    std::map<TargetKey, std::unique_ptr<dbContent::TargetReport::Chain>>& chains () { return chains_; }

protected:
    unsigned int utn_;
    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    std::map<TargetKey, std::unique_ptr<dbContent::TargetReport::Chain>> chains_;
};

} // namespace CalculateReferences

#endif // CALCULATEREFERENCES_TARGET_H

#include "assoc/targetreport.h"
#include "assoc/target.h"
#include "stringconv.h"

#include <cassert>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace Utils;

namespace Association
{

TargetReport::TargetReport()
{
}

void TargetReport::addAssociated (Target* tr)
{
    assert (tr);
    assoc_targets_.push_back(tr);
}

void TargetReport::removeAssociated (Target* tr)
{
    assert (tr);
    auto tr_it = find(assoc_targets_.begin(), assoc_targets_.end(), tr);
    assert (tr_it != assoc_targets_.end());
    assoc_targets_.erase(tr_it);
}

std::string TargetReport::asStr()
{
    stringstream ss;

    ss << "dbo " << dbo_name_ << " ds_id " << ds_id_ << " rec_num " << rec_num_
       << " tod " << String::timeStringFromDouble(tod_);

    if (has_ta_)
        ss << " ta " << String::hexStringFromInt(ta_, 6, '0');

    if (has_ti_)
        ss << " ti '" << ti_ << "'";

    if (has_tn_)
        ss << " tn " << tn_;

    if (has_ma_)
        ss << " m3a " << String::octStringFromInt(ma_, 4, '0');

    return ss.str();
}

}

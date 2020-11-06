#include "assoc/target.h"
#include "assoc/targetreport.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>
#include <sstream>


using namespace std;
using namespace Utils;

namespace Association
{
    Target::Target(unsigned int utn, bool tmp)
        : utn_(utn), tmp_(tmp)
    {
    }

    Target::~Target()
    {
        if (!tmp_)
            for (auto& tr_it : assoc_trs_)
                tr_it->removeAssociated(this);
    }

    void Target::addAssociated (TargetReport* tr)
    {
        assert (tr);

        assoc_trs_.push_back(tr);

        if (!has_ta_ && tr->has_ta_)
        {
            has_ta_ = true;
            ta_ = tr->ta_;
        }
        else if (has_ta_ && tr->has_ta_ && ta_ != tr->ta_)
        {
            logwrn << "Target: addAssociated: ta mismatch, target " << asStr()
                   << " tr " << tr->asStr();
        }

        if (!tmp_)
            tr->addAssociated(this);
    }

    void Target::addAssociated (vector<TargetReport*> trs)
    {
        for (auto& tr_it : trs)
            addAssociated(tr_it);
    }

    std::string Target::asStr()
    {
        stringstream ss;

        ss << "utn " << utn_ << " tmp " << tmp_;

        if (has_ta_)
            ss << " ta " << String::hexStringFromInt(ta_, 6, '0');

        return ss.str();
    }
}

/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "adsbqualityfilter.h"
#include "adsbqualityfilterwidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;

ADSBQualityFilter::ADSBQualityFilter(const std::string& class_id, const std::string& instance_id,
                                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("use_v0", &use_v0_, true);
    registerParameter("use_v1", &use_v1_, true);
    registerParameter("use_v2", &use_v2_, true);

    registerParameter("use_min_nucp", &use_min_nucp_, true);
    registerParameter("min_nucp", &min_nucp_, 4);

    registerParameter("use_min_nic", &use_min_nic_, true);
    registerParameter("min_nic", &min_nic_, 5);

    registerParameter("use_min_nacp", &use_min_nacp_, true);
    registerParameter("min_nacp", &min_nacp_, 5);

    registerParameter("use_min_sil_v1", &use_min_sil_v1_, true);
    registerParameter("min_sil_v1", &min_sil_v1_, 2);

    registerParameter("use_min_sil_v2", &use_min_sil_v2_, true);
    registerParameter("min_sil_v2", &min_sil_v2_, 4);

    name_ = "ADSB Quality";

    createSubConfigurables();

    assert(widget_);
}

ADSBQualityFilter::~ADSBQualityFilter() {}

bool ADSBQualityFilter::filters(const std::string& dbo_type)
{
    return dbo_type == "ADSB";
}

std::string ADSBQualityFilter::getConditionString(const std::string& dbo_name, bool& first,
                                          std::vector<DBOVariable*>& filtered_variables)
{
    logdbg << "ADSBQualityFilter: getConditionString: dbo " << dbo_name << " active " << active_;

    if (dbo_name != "ADSB")
        return "";

    stringstream ss;

    if (active_)
    {
        first = false;

        ss << "MOPS_VERSION IN (";

        if (use_v0_)
            ss << "0";

        if (use_v1_)
        {
            if (use_v0_)
                ss << ",1";
            else
                ss << "1";
        }

        if (use_v2_)
        {
            if (use_v0_ || use_v0_)
                ss << ",2";
            else
                ss << "2";
        }

        ss << ")";

        if (use_min_nucp_)
            ss << " AND ((NUCP_NIC >= " << min_nucp_ << " AND MOPS_VERSION=0) OR MOPS_VERSION IN (1,2))";

        if (use_min_nic_)
            ss << " AND ((NUCP_NIC >= " << min_nic_ << " AND MOPS_VERSION IN (1,2)) OR MOPS_VERSION = 0)";

        if (use_min_nacp_)
            ss << " AND ((NAC_P >= " << min_nacp_ << " AND MOPS_VERSION IN (1,2)) OR MOPS_VERSION = 0)";

        if (use_min_sil_v1_)
            ss << " AND ((SIL >= " << min_sil_v1_ << " AND MOPS_VERSION=1) OR MOPS_VERSION IN (0,2))";

        if (use_min_sil_v2_)
            ss << " AND ((SIL >= " << min_sil_v2_ << " AND MOPS_VERSION=2) OR MOPS_VERSION IN (0,1))";

    }

    loginf << "ADSBQualityFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}


void ADSBQualityFilter::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    logdbg << "ADSBQualityFilter: generateSubConfigurable: class_id " << class_id;

    if (class_id.compare("ADSBQualityFilterWidget") == 0)
    {
        assert(!widget_);
        widget_ = new ADSBQualityFilterWidget(*this, class_id, instance_id);
    }
    else
        throw std::runtime_error("ADSBQualityFilter: generateSubConfigurable: unknown class_id " +
                                 class_id);
}


void ADSBQualityFilter::checkSubConfigurables()
{
    logdbg << "ADSBQualityFilter: checkSubConfigurables";

    if (!widget_)
    {
        logdbg << "ADSBQualityFilter: checkSubConfigurables: generating filter widget";
        widget_ = new ADSBQualityFilterWidget(*this, "ADSBQualityFilterWidget", instanceId() + "Widget0");
    }
    assert(widget_);
}

void ADSBQualityFilter::reset()
{
//    for (auto& it : data_sources_)
//        it.second.setActive(true);

    widget_->update();
}

void ADSBQualityFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    //filter["utns"] = utns_str_;
}

void ADSBQualityFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

//    assert (filter.contains("utns"));
//    utns_str_ = filter.at("utns");

//    updateUTNSFromStr(utns_str_);

    if (widget())
        widget()->update();
}

bool ADSBQualityFilter::useV0() const
{
    return use_v0_;
}

void ADSBQualityFilter::useV0(bool value)
{
    loginf << "ADSBQualityFilter: useV0: value " << value;
    use_v0_ = value;
}

bool ADSBQualityFilter::useV1() const
{
    return use_v1_;
}

void ADSBQualityFilter::useV1(bool value)
{
    loginf << "ADSBQualityFilter: useV1: value " << value;
    use_v1_ = value;
}

bool ADSBQualityFilter::useV2() const
{
    return use_v2_;
}

void ADSBQualityFilter::useV2(bool value)
{
    loginf << "ADSBQualityFilter: useV2: value " << value;
    use_v2_ = value;
}

bool ADSBQualityFilter::useMinNUCP() const
{
    return use_min_nucp_;
}

void ADSBQualityFilter::useMinNUCP(bool value)
{
    loginf << "ADSBQualityFilter: useMinNUCP: value " << value;
    use_min_nucp_ = value;
}

unsigned int ADSBQualityFilter::minNUCP() const
{
    return min_nucp_;
}

void ADSBQualityFilter::minNUCP(unsigned int value)
{
    loginf << "ADSBQualityFilter: minNUCP: value " << value;
    min_nucp_ = value;
}

bool ADSBQualityFilter::useMinNIC() const
{
    return use_min_nic_;
}

void ADSBQualityFilter::useMinNIC(bool value)
{
    loginf << "ADSBQualityFilter: useMinNIC: value " << value;
    use_min_nic_ = value;
}

unsigned int ADSBQualityFilter::minNIC() const
{
    return min_nic_;
}

void ADSBQualityFilter::minNIC(unsigned int value)
{
    loginf << "ADSBQualityFilter: minNIC: value " << value;
    min_nic_ = value;
}

bool ADSBQualityFilter::useMinNACp() const
{
    return use_min_nacp_;
}

void ADSBQualityFilter::useMinNACp(bool value)
{
    loginf << "ADSBQualityFilter: useMinNACp: value " << value;
    use_min_nacp_ = value;
}

unsigned int ADSBQualityFilter::minNACp() const
{
    return min_nacp_;
}

void ADSBQualityFilter::minNACp(unsigned int value)
{
    loginf << "ADSBQualityFilter: minNACp: value " << value;
    min_nacp_ = value;
}

bool ADSBQualityFilter::useMinSILv1() const
{
    return use_min_sil_v1_;
}

void ADSBQualityFilter::useMinSILv1(bool value)
{
    loginf << "ADSBQualityFilter: useMinSILv1: value " << value;
    use_min_sil_v1_ = value;
}

unsigned int ADSBQualityFilter::minSILv1() const
{
    return min_sil_v1_;
}

void ADSBQualityFilter::minSILv1(unsigned int value)
{
    loginf << "ADSBQualityFilter: minSILv1: value " << value;
    min_sil_v1_ = value;
}

bool ADSBQualityFilter::useMinSILv2() const
{
    return use_min_sil_v2_;
}

void ADSBQualityFilter::useMinSILv2(bool value)
{
    loginf << "ADSBQualityFilter: useMinSILv2: value " << value;
    use_min_sil_v2_ = value;
}

unsigned int ADSBQualityFilter::minSILv2() const
{
    return min_sil_v2_;
}

void ADSBQualityFilter::minSILv2(unsigned int value)
{
    loginf << "ADSBQualityFilter: minSILv2: value " << value;
    min_sil_v2_ = value;
}

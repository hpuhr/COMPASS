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

#include "eval/results/base/base.h"
#include "eval/requirement/base/base.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/section_id.h"

#include "sectorlayer.h"
//#include "logger.h"
#include "evaluationmanager.h"

#include "viewpoint.h"

#include <sstream>
#include <cassert>

using namespace std;

namespace EvaluationRequirementResult
{

const std::string Base::req_overview_table_name_ {"Results Overview"};

/**
*/
Base::Base(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement, 
           const SectorLayer& sector_layer,
           EvaluationManager& eval_man)
:   type_        (type)
,   result_id_   (result_id)
,   requirement_ (requirement)
,   sector_layer_(sector_layer)
,   eval_man_    (eval_man)
{
    assert (requirement_);

    req_grp_id_ = EvaluationResultsReport::SectionID::requirementGroupResultID(*this);
}

/**
*/
Base::~Base() = default;

/**
*/
bool Base::isSingle() const
{
    return (baseType() == BaseType::Single);
}

/**
*/
bool Base::isJoined() const
{
    return (baseType() == BaseType::Joined);
}

/**
*/
std::shared_ptr<EvaluationRequirement::Base> Base::requirement() const
{
    return requirement_;
}

/**
*/
std::string Base::type() const
{
    return type_;
}

/**
*/
std::string Base::resultId() const
{
    return result_id_;
}

/**
*/
std::string Base::reqGrpId() const
{
    return req_grp_id_;
}

/**
*/
bool Base::use() const
{
    return use_;
}

/**
*/
void Base::use(bool use)
{
    use_ = use;
}

/**
*/
const boost::optional<double>& Base::result() const
{
    return result_;
}

/**
*/
void Base::updateResult()
{
    result_.reset();

    auto result = computeResult();
    if (result.has_value())
        result_ = result.value();
}

/**
*/
bool Base::resultUsable() const
{
    return (result_.has_value() && !ignore_);
}

/**
*/
bool Base::hasFailed() const
{
    return (resultUsable() && !requirement_->conditionPassed(result_.value()));
}

/**
*/
bool Base::hasIssues() const
{
    return (resultUsable() && numIssues() > 0);
}

/**
*/
QVariant Base::resultValue() const
{
    return resultValueOptional(result_);
}

/**
*/
QVariant Base::resultValueOptional(const boost::optional<double>& value) const
{
    if (!value.has_value())
        return QVariant();

    return resultValue(value.value());
}

/**
*/
QVariant Base::resultValue(double value) const
{
    return requirement_->getResultValueString(value).c_str();
}

/**
*/
std::string Base::conditionResultString() const
{
    if (!resultUsable())
        return "";

    return requirement_->getConditionResultStr(result_.value());
}

/**
*/
void Base::setIgnored()
{
    ignore_ = true;
}

/**
*/
bool Base::isIgnored() const
{
    return ignore_;
}

/**
*/
boost::optional<double> Base::computeResult() const
{
    return computeResult_impl();
}

/**
*/
EvaluationResultsReport::SectionContentTable& Base::getReqOverviewTable (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& ov_sec = root_item->getSection("Overview:Results");

    if (!ov_sec.hasTable(req_overview_table_name_))
        ov_sec.addTable(req_overview_table_name_, 8,
        {"Sector Layer", "Group", "Req.", "Id", "#Updates", "Value", "Condition", "Result"});

    return ov_sec.getTable(req_overview_table_name_);
}

/**
*/
std::string Base::getRequirementSectionID() const
{
    return EvaluationResultsReport::SectionID::requirementResultID(*this);
}

/**
*/
std::string Base::getRequirementSumSectionID() const
{
    return EvaluationResultsReport::SectionID::requirementResultSumID(*this);
}

/**
*/
EvaluationResultsReport::Section& Base::getRequirementSection (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    return root_item->getSection(getRequirementSectionID());
}

/**
*/
size_t Base::numDetails() const
{
    return details_.size();
}

/**
*/
const Base::EvaluationDetails& Base::getDetails() const
{
    return details_;
}

/**
*/
const EvaluationDetail& Base::getDetail(int idx) const
{
    return getDetails().at(idx);
}

/**
*/
const EvaluationDetail& Base::getDetail(const DetailIndex& index) const
{
    const auto& d = getDetail(index[ 0 ]);

    if (index[ 1 ] < 0)
        return d;

    return d.details().at(index[ 1 ]);
}

/**
*/
bool Base::detailIndexValid(const DetailIndex& index) const
{
    if (index[ 0 ] < 0 || index[ 0 ] >= (int)details_.size())
        return false;
    if (index[ 1 ] >= 0 && index[ 1 ] >= (int)details_[ index[ 0 ] ].numDetails())
        return false;

    return true;
}

/**
*/
void Base::clearDetails()
{
    details_ = {};
}

/**
*/
void Base::setDetails(const EvaluationDetails& details)
{
    details_ = details;
}

/**
*/
void Base::addDetails(const EvaluationDetails& details)
{
    details_.insert(details_.end(), details.begin(), details.end());
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Base::createViewable(const AnnotationOptions& options) const
{
    auto viewable_ptr = createBaseViewable();        // create basic viewable
    auto info         = createViewableInfo(options); // create viewable info (data bounds etc.)

    //configure viewable depending on type
    if (info.viewable_type == ViewableType::Overview)
    {
        //overview => set region of interest
        if (!info.bounds.isEmpty())
        {
            (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY] = info.bounds.center().x();
            (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY] = info.bounds.center().y();

            double lat_w = info.bounds.width();
            double lon_w = info.bounds.height();

            if (lat_w < eval_man_.settings().result_detail_zoom_)
                lat_w = eval_man_.settings().result_detail_zoom_;

            if (lon_w < eval_man_.settings().result_detail_zoom_)
                lon_w = eval_man_.settings().result_detail_zoom_;

            (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = lat_w;
            (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = lon_w;
        }
    }
    else
    {
        //detail => set position of interest
        (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY    ] = info.bounds.x();
        (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY    ] = info.bounds.y();
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_TIMESTAMP_KEY  ] = Utils::Time::toString(info.timestamp);
    }

    //init annotation array
    (*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY] = nlohmann::json::array();

    //add annotations
    createAnnotations((*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY], options);

    return viewable_ptr;
}

/**
*/
QString Base::formatValue(double v, int precision) const
{
    return QString::fromStdString(Utils::String::doubleToStringPrecision(v, precision));
}

}

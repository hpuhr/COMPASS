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
#include "eval/results/base/result_t.h"
#include "eval/results/base/featuredefinition.h"

#include "task/result/report/report.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontenttable.h"

#include "eval/results/report/section_id.h"

#include "eval/requirement/base/base.h"
#include "number.h"

#include "sectorlayer.h"
//#include "logger.h"
#include "evaluationmanager.h"

#include "viewpoint.h"
#include "viewpointgenerator.h"
#include "histograminitializer.h"
#include "grid2d_defs.h"
#include "grid2d.h"
#include "grid2dlayer.h"

#include <sstream>
#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

const std::string Base::req_overview_table_name_ {"Results Overview"};

const QColor Base::HistogramColorDefault = QColor(0, 0, 255);

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
void Base::updateResult(const boost::optional<double>& value)
{
    result_ = value;
}

/**
*/
nlohmann::json Base::resultValue() const
{
    return resultValueOptional(result_);
}

/**
*/
nlohmann::json Base::resultValueOptional(const boost::optional<double>& value) const
{
    if (!value.has_value())
        return nlohmann::json();

    return resultValue(value.value());
}

/**
*/
nlohmann::json Base::resultValue(double value) const
{
    return requirement_->getResultValueString(value);
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
ResultReport::SectionContentTable& Base::getReqOverviewTable (std::shared_ptr<ResultReport::Report> report)
{
    auto& ov_sec = report->getSection("Overview:Results");

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
std::string Base::getRequirementAnnotationID() const
{
    return "Evaluation:" + getRequirementAnnotationID_impl();
}

/**
*/
ResultReport::Section& Base::getRequirementSection (std::shared_ptr<ResultReport::Report> report)
{
    return report->getSection(getRequirementSectionID());
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
    auto& vp_anno_array = (*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY];

    //add root annotation and retrieve its annotation array
    std::string root_anno_name = getRequirementAnnotationID();
    assert(!root_anno_name.empty());

    ViewPointGenAnnotation root_anno(root_anno_name);

    nlohmann::json root_anno_json;
    root_anno.toJSON(root_anno_json);

    vp_anno_array.push_back(root_anno_json);

    auto& result_anno_array = ViewPointGenAnnotation::getChildrenJSON(vp_anno_array.at(0));

    //add annotations
    createAnnotations(result_anno_array, options);

    return viewable_ptr;
}

/**
*/
double Base::formatValue(double v, int precision) const
{
    //return QString::fromStd(Utils::String::doubleToStringPrecision(v, precision));
    return Number::round(v, precision);
}

/**
*/
FeatureDefinitions Base::getCustomAnnotationDefinitions() const
{
    //by default return empty definitions => no custom annotations will be generated
    return FeatureDefinitions();
}

/**
 * Called by Single and Joined to add custom annotations to the viewable.
*/
void Base::addCustomAnnotations(nlohmann::json& annotations_json) const
{
    assert(annotations_json.is_array());

    //get custom annotations
    auto defs = getCustomAnnotationDefinitions();

    std::string group_name = getRequirementAnnotationID();

    for (const auto& value_defs : defs.definitions())
    {
        loginf << "Base: addCustomAnnotations: Adding annotation for value '" << value_defs.first << "'"; 

        //create annotation for value features
        ViewPointGenAnnotation value_annotation(value_defs.first);

        for (const auto& def : value_defs.second)
        {
            assert(def);

            //create feature and add to annotation
            auto f = def->createFeature(this);
            if (!f)
            {
                loginf << "Base: addCustomAnnotations: Skipping empty feature of definition type '" << def->type() << "'";
                continue;
            }

            loginf << "Base: addCustomAnnotations: Adding feature '" << f->name() << "' of definition type '" << def->type() << "'";

            PlotMetadata metadata;
            metadata.plot_group_   = group_name;
            metadata.title_        = value_defs.first;
            metadata.subtitle_     = def->featureDescription();
            metadata.x_axis_label_ = def->xAxisLabel();
            metadata.y_axis_label_ = def->yAxisLabel();

            f->setPlotMetadata(metadata);
            f->writeBinaryIfPossible(true);

            ViewPointGenAnnotation* feat_annotation = new ViewPointGenAnnotation(def->featureDescription());
            feat_annotation->addFeature(std::move(f));

            value_annotation.addAnnotation(std::unique_ptr<ViewPointGenAnnotation>(feat_annotation));
        }

        loginf << "Base: addCustomAnnotations: Added " << value_annotation.numFeatures() << " feature(s) to annotation";

        //convert to json and collect
        nlohmann::json anno_json;
        value_annotation.toJSON(anno_json);

        annotations_json.push_back(anno_json);
    }
}

/**
*/
size_t Base::totalNumDetails() const
{
    size_t num_details = 0;

    //scan for estimated max num values
    auto funcScan = [ & ] (const EvaluationDetail& detail, 
                           const EvaluationDetail* parent_detail, 
                           int idx0, 
                           int idx1,
                           int evt_pos_idx, 
                           int evt_ref_pos_idx)
    {
        ++num_details;
    };

    iterateDetails(funcScan, {});

    return num_details;
}

/**
*/
size_t Base::totalNumPositions() const
{
    size_t num_positions = 0;

    //scan for estimated max num values
    auto funcScan = [ & ] (const EvaluationDetail& detail, 
                           const EvaluationDetail* parent_detail, 
                           int idx0, 
                           int idx1,
                           int evt_pos_idx, 
                           int evt_ref_pos_idx)
    {
        num_positions += detail.numPositions();
    };

    iterateDetails(funcScan, {});

    return num_positions;
}

}

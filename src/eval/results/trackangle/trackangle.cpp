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

#include "eval/results/trackangle/trackangle.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/trackangle/trackangle.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "util/number.h"
#include "viewpoint.h"
#include "compass.h"

#include <cassert>
#include <algorithm>
#include <fstream>

#include <QFileDialog>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/***************************************************************************************
 * TrackAngleBase
 ***************************************************************************************/

/**
*/
TrackAngleBase::TrackAngleBase() = default;

/**
*/
TrackAngleBase::TrackAngleBase(unsigned int num_pos, 
                               unsigned int num_no_ref,
                               unsigned int num_pos_outside, 
                               unsigned int num_pos_inside, 
                               unsigned int num_no_tst_value,
                               unsigned int num_comp_failed, 
                               unsigned int num_comp_passed)
:   num_pos_         (num_pos)
,   num_no_ref_      (num_no_ref)
,   num_pos_outside_ (num_pos_outside)
,   num_pos_inside_  (num_pos_inside)
,   num_no_tst_value_(num_no_tst_value)
,   num_comp_failed_ (num_comp_failed)
,   num_comp_passed_ (num_comp_passed)
{
}

/**
*/
unsigned int TrackAngleBase::numCompFailed() const
{
    return num_comp_failed_;
}

/**
*/
unsigned int TrackAngleBase::numCompPassed() const
{
    return num_comp_passed_;
}

/**
*/
unsigned int TrackAngleBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int TrackAngleBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int TrackAngleBase::numNoTstValues() const
{
    return num_no_tst_value_;
}

/**
*/
unsigned int TrackAngleBase::numPos() const
{
    return num_pos_;
}

/**
*/
unsigned int TrackAngleBase::numNoRef() const
{
    return num_no_ref_;
}

/***************************************************************************************
 * SingleTrackAngle
 ***************************************************************************************/

/**
*/
SingleTrackAngle::SingleTrackAngle(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer,
                                   unsigned int utn,
                                   const EvaluationTargetData* target,
                                   EvaluationManager& eval_man,
                                   const EvaluationDetails& details,
                                   unsigned int num_pos,
                                   unsigned int num_no_ref,
                                   unsigned int num_pos_outside,
                                   unsigned int num_pos_inside,
                                   unsigned int num_no_tst_value,
                                   unsigned int num_comp_failed,
                                   unsigned int num_comp_passed)
:   TrackAngleBase(num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_no_tst_value, num_comp_failed, num_comp_passed)
,   SingleProbabilityBase("SingleTrackAngle", result_id, requirement, sector_layer, utn, target, eval_man, details)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SingleTrackAngle::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedTrackAngle> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
boost::optional<double> SingleTrackAngle::computeResult_impl() const
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    accumulator_.reset();

    auto values = getValues(DetailKey::Offset);

    assert (values.size() == num_comp_failed_ + num_comp_passed_);

    unsigned int num_trackangles = values.size();

    if (num_trackangles == 0)
        return {};

    accumulator_.accumulate(values, true);

    assert (num_comp_failed_ <= num_trackangles);

    return (double)num_comp_passed_ / (double)num_trackangles;
}

/**
*/
unsigned int SingleTrackAngle::numIssues() const
{
    return num_comp_failed_;
}

/**
*/
std::vector<std::string> SingleTrackAngle::targetTableHeadersCustom() const
{
    return { "OMin", "OMax", "OAvg", "OSDev", "#CF", "#CP" };
}

/**
*/
std::vector<QVariant> SingleTrackAngle::targetTableValuesCustom() const
{
    return { formatValue(accumulator_.min()),    // "DMin"
             formatValue(accumulator_.max()),    // "DMax"
             formatValue(accumulator_.mean()),   // "DAvg"
             formatValue(accumulator_.stddev()), // "DSDev"
             num_comp_failed_,                   // "#DOK"
             num_comp_passed_ };                 // "#DNOK"
}

/**
*/
std::vector<Single::TargetInfo> SingleTrackAngle::targetInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                            , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference trackangles"  , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"              , num_pos_inside_                    }, 
             { "#PosOutside [1]", "Number of updates outside sector"             , num_pos_outside_                   },
             { "#NoTstData [1]" , "Number of updates without tst trackangle data", num_no_tst_value_                  }, 
             { "OMin [m/s]"     , "Minimum of trackangle offset"                 , formatValue(accumulator_.min())    }, 
             { "OMax [m/s]"     , "Maximum of trackangle offset"                 , formatValue(accumulator_.max())    }, 
             { "OAvg [m/s]"     , "Average of trackangle offset"                 , formatValue(accumulator_.mean())   }, 
             { "OSDev [m/s]"    , "Standard Deviation of trackangle offset"      , formatValue(accumulator_.stddev()) },
             { "OVar [m^2/s^2]" , "Variance of trackangle offset"                , formatValue(accumulator_.var())    },
             { "#CF [1]"        , "Number of updates with failed comparison"     , num_comp_failed_                   }, 
             { "#CP [1]"        , "Number of updates with passed comparison"     , num_comp_passed_                   } };
}

/**
*/
std::vector<std::string> SingleTrackAngle::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "Distance", "CP", "Value Ref", "Value Tst", "Speed Ref", "#CF", "#CP", "Comment" };
}

/**
*/
std::vector<QVariant> SingleTrackAngle::detailValues(const EvaluationDetail& detail,
                                                     const EvaluationDetail* parent_detail) const
{
    bool has_ref_pos = detail.numPositions() >= 2;

    return { Utils::Time::toString(detail.timestamp()).c_str(),
            !has_ref_pos,
             detail.getValue(DetailKey::PosInside),
             detail.getValue(DetailKey::Offset),           // "Distance"
             detail.getValue(DetailKey::CheckPassed),      // CP"
             detail.getValue(DetailKey::ValueRef),         // "Value Ref"
             detail.getValue(DetailKey::ValueTst),         // "Value Tst"
             detail.getValue(DetailKey::SpeedRef),         // "Speed Ref"
             detail.getValue(DetailKey::NumCheckFailed),   // "#CF",
             detail.getValue(DetailKey::NumCheckPassed),   // "#CP"
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SingleTrackAngle::detailIsOk(const EvaluationDetail& detail) const
{
    auto req = dynamic_cast<const EvaluationRequirement::TrackAngle*>(requirement_.get());
    assert(req);

    bool failed_values_of_interest = req->failedValuesOfInterest();

    auto check_passed = detail.getValueAs<bool>(DetailKey::CheckPassed);
    assert(check_passed.has_value());

    return (( failed_values_of_interest &&  check_passed.value()) ||
            (!failed_values_of_interest && !check_passed.value()));
}

/**
*/
void SingleTrackAngle::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                              const EvaluationDetail& detail, 
                                              TargetAnnotationType type,
                                              bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationPos(annotations_json, detail.position(0), AnnotationType::TypeHighlight);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationPos(annotations_json, detail.position(0), is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError);
    }

    // legacy code ahead
    //        for (const auto& line_it : det_it.lines())
    //        {
    //            const QColor& line_color_qt = get<2>(line_it);
    //            line_color = {(float)line_color_qt.redF(), (float)line_color_qt.greenF(), (float)line_color_qt.blueF(), 1.0f};
    //            anno_lines.push_back({get<0>(line_it), get<1>(line_it), line_color});
    //        }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleTrackAngle::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleTrackAngle::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, DetailKey::CheckPassed);
    }
}

/***************************************************************************************
 * JoinedTrackAngle
 ***************************************************************************************/

/**
*/
JoinedTrackAngle::JoinedTrackAngle(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer, 
                                   EvaluationManager& eval_man)
:   TrackAngleBase()
,   JoinedProbabilityBase("JoinedTrackAngle", result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
unsigned int JoinedTrackAngle::numIssues() const
{
    return num_comp_failed_;
}

/**
*/
unsigned int JoinedTrackAngle::numUpdates() const
{
    return num_comp_failed_ + num_comp_passed_;
}

/**
*/
void JoinedTrackAngle::clearResults_impl() 
{
    num_pos_          = 0;
    num_no_ref_       = 0;
    num_pos_outside_  = 0;
    num_pos_inside_   = 0;
    num_no_tst_value_ = 0;
    num_comp_failed_  = 0;
    num_comp_passed_  = 0;

    accumulator_.reset();
}

/**
*/
void JoinedTrackAngle::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SingleTrackAngle> single_ta = std::static_pointer_cast<SingleTrackAngle>(single_result);

    num_pos_          += single_ta->numPos();
    num_no_ref_       += single_ta->numNoRef();
    num_pos_outside_  += single_ta->numPosOutside();
    num_pos_inside_   += single_ta->numPosInside();
    num_no_tst_value_ += single_ta->numNoTstValues();
    num_comp_failed_  += single_ta->numCompFailed();
    num_comp_passed_  += single_ta->numCompPassed();

    auto values = single_ta->getValues(SingleTrackAngle::DetailKey::Offset);
    assert(values.size() == single_ta->numCompFailed() + single_ta->numCompPassed());

    accumulator_.accumulate(values, last);
}

/**
*/
boost::optional<double> JoinedTrackAngle::computeResult_impl() const
{
    loginf << "JoinedTrackAngle: computeResult_impl:"
            << " num_pos " << num_pos_
            << " num_no_ref " << num_no_ref_
            << " num_no_tst_value " << num_no_tst_value_
            << " num_comp_failed " << num_comp_failed_
            << " num_comp_passed " << num_comp_passed_;

    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    unsigned int total = num_comp_passed_ + num_comp_failed_;

    if (total == 0)
        return {};

    return (double)num_comp_passed_ / (double)total;
}

/**
*/
std::vector<Joined::SectorInfo> JoinedTrackAngle::sectorInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                            , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference trackangles"  , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"              , num_pos_inside_                    }, 
             { "#PosOutside [1]", "Number of updates outside sector"             , num_pos_outside_                   },
             { "#NoTstData [1]" , "Number of updates without tst trackangle data", num_no_tst_value_                  }, 
             { "OMin [m/s]"     , "Minimum of trackangle offset"                 , formatValue(accumulator_.min())    }, 
             { "OMax [m/s]"     , "Maximum of trackangle offset"                 , formatValue(accumulator_.max())    }, 
             { "OAvg [m/s]"     , "Average of trackangle offset"                 , formatValue(accumulator_.mean())   }, 
             { "OSDev [m/s]"    , "Standard Deviation of trackangle offset"      , formatValue(accumulator_.stddev()) },
             { "OVar [m^2/s^2]" , "Variance of trackangle offset"                , formatValue(accumulator_.var())    },
             { "#CF [1]"        , "Number of updates with failed comparison"     , num_comp_failed_                   }, 
             { "#CP [1]"        , "Number of updates with passed comparison"     , num_comp_passed_                   } };
}

/**
*/
bool JoinedTrackAngle::exportAsCSV(std::ofstream& strm) const
{
    loginf << "JoinedTrackAngle: exportAsCSV";

    strm << "trackangle_offset\n";

    auto values = getValues(SingleTrackAngle::DetailKey::Offset);

    for (auto v : values)
        strm << v << "\n";
       
    if (!strm)
        return false;

    return true;
}

}

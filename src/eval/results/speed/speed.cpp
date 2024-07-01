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

#include "eval/results/speed/speed.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/speed/speed.h"

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

/***********************************************************************************
 * SpeedBase
 ***********************************************************************************/

/**
*/
SpeedBase::SpeedBase() = default;

/**
*/
SpeedBase::SpeedBase(unsigned int num_pos, 
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
unsigned int SpeedBase::numCompFailed() const
{
    return num_comp_failed_;
}

/**
*/
unsigned int SpeedBase::numCompPassed() const
{
    return num_comp_passed_;
}

/**
*/
unsigned int SpeedBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int SpeedBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int SpeedBase::numNoTstValues() const
{
    return num_no_tst_value_;
}

/**
*/
unsigned int SpeedBase::numPos() const
{
    return num_pos_;
}

/**
*/
unsigned int SpeedBase::numNoRef() const
{
    return num_no_ref_;
}

/***********************************************************************************
 * SingleSpeed
 ***********************************************************************************/

/**
*/
SingleSpeed::SingleSpeed(const std::string& result_id, 
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
:   SpeedBase(num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_no_tst_value, num_comp_failed, num_comp_passed)
,   SingleProbabilityBase("SingleSpeed", result_id, requirement, sector_layer, utn, target, eval_man, details)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SingleSpeed::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedSpeed> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::vector<double> SingleSpeed::getValues() const
{
    std::vector<double> values;
    values.reserve(getDetails().size());

    auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int idx0, int idx1)
    {
        auto offset_valid = detail.getValueAs<bool>(OffsetValid);
        if (!offset_valid.has_value() || !offset_valid.value())
            return;

        auto offset = detail.getValueAsOrAssert<float>(Offset);
        values.push_back(offset);
    };

    iterateDetails(func);

    values.shrink_to_fit();

    return values;
}

/**
*/
boost::optional<double> SingleSpeed::computeResult_impl() const
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    accumulator_.reset();

    auto values = getValues();

    assert (values.size() == num_comp_failed_ + num_comp_passed_);

    unsigned int num_speeds = values.size();

    if (num_speeds == 0)
        return {};

    accumulator_.accumulate(values, true);

    assert (num_comp_failed_ <= num_speeds);
    
    return (double)num_comp_passed_ / (double)num_speeds;
}

/**
*/
unsigned int SingleSpeed::numIssues() const
{
    return num_comp_failed_;
}

/**
*/
std::vector<std::string> SingleSpeed::targetTableHeadersCustom() const
{
    return { "OMin", "OMax", "OAvg", "OSDev", "#CF", "#CP" };
}

/**
*/
std::vector<QVariant> SingleSpeed::targetTableValuesCustom() const
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
std::vector<Single::TargetInfo> SingleSpeed::targetInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                        , num_pos_                           },
             { "#NoRef [1]"     , "Number of updates w/o reference speeds"   , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"          , num_pos_inside_                    },
             { "#PosOutside [1]", "Number of updates outside sector"         , num_pos_outside_                   },
             { "#NoTstData [1]" , "Number of updates without tst speed data" , num_no_tst_value_                  },
             { "OMin [m/s]"     , "Minimum of speed offset"                  , formatValue(accumulator_.min())    },
             { "OMax [m/s]"     , "Maximum of speed offset"                  , formatValue(accumulator_.max())    },
             { "OAvg [m/s]"     , "Average of speed offset"                  , formatValue(accumulator_.mean())   }, 
             { "OSDev [m/s]"    , "Standard Deviation of speed offset"       , formatValue(accumulator_.stddev()) }, 
             { "OVar [m^2/s^2]" , "Variance of speed offset"                 , formatValue(accumulator_.var())    },
             { "#CF [1]"        , "Number of updates with failed comparison" , num_comp_failed_                   },
             { "#CP [1]"        , "Number of updates with  passed comparison", num_comp_passed_                   } };
}

/**
*/
std::vector<std::string> SingleSpeed::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "Distance", "CP", "#CF", "#CP", "Comment" };
}

/**
*/
std::vector<QVariant> SingleSpeed::detailValues(const EvaluationDetail& detail,
                                                const EvaluationDetail* parent_detail) const
{
    bool has_ref_pos = detail.numPositions() >= 2;

    return { Utils::Time::toString(detail.timestamp()).c_str(),
            !has_ref_pos,
             detail.getValue(DetailKey::PosInside),
             detail.getValue(DetailKey::Offset),           // "Distance"
             detail.getValue(DetailKey::CheckPassed),      // CP"
             detail.getValue(DetailKey::NumCheckFailed),   // "#CF",
             detail.getValue(DetailKey::NumCheckPassed),   // "#CP"
             detail.comments().generalComment().c_str() }; // "Comment"
}

/**
*/
bool SingleSpeed::detailIsOk(const EvaluationDetail& detail) const
{
    EvaluationRequirement::Speed* req = dynamic_cast<EvaluationRequirement::Speed*>(requirement_.get());
    assert(req);

    auto failed_values_of_interest = req->failedValuesOfInterest();

    auto check_passed = detail.getValueAs<bool>(DetailKey::CheckPassed);
    assert(check_passed.has_value());

    return (( failed_values_of_interest &&  check_passed.value()) ||
            (!failed_values_of_interest && !check_passed.value()));
}

/**
*/
void SingleSpeed::addAnnotationForDetail(nlohmann::json& annotations_json, 
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
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleSpeed::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleSpeed::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, DetailKey::CheckPassed);
    }
}

/***********************************************************************************
 * JoinedSpeed
 ***********************************************************************************/

/**
*/
JoinedSpeed::JoinedSpeed(const std::string& result_id, 
                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                         const SectorLayer& sector_layer, 
                         EvaluationManager& eval_man)
:   SpeedBase()
,   JoinedProbabilityBase("JoinedSpeed", result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
std::vector<double> JoinedSpeed::getValues() const
{
    std::vector<double> values;

    auto func = [ & ] (const std::shared_ptr<Single>& result)
    {
        std::shared_ptr<SingleSpeed> res = std::static_pointer_cast<SingleSpeed>(result);
        auto v = res->getValues();

        values.insert(values.end(), v.begin(), v.end());
    };

    iterateSingleResults({}, func, {});

    return values;
}

/**
*/
unsigned int JoinedSpeed::numIssues() const
{
    return num_comp_failed_;
}

/**
*/
unsigned int JoinedSpeed::numUpdates() const
{
    return num_comp_failed_ + num_comp_passed_;
}

/**
*/
void JoinedSpeed::clearResults_impl() 
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
void JoinedSpeed::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SingleSpeed> single = std::static_pointer_cast<SingleSpeed>(single_result);

    num_pos_          += single->numPos();
    num_no_ref_       += single->numNoRef();
    num_pos_outside_  += single->numPosOutside();
    num_pos_inside_   += single->numPosInside();
    num_no_tst_value_ += single->numNoTstValues();
    num_comp_failed_  += single->numCompFailed();
    num_comp_passed_  += single->numCompPassed();

    auto values = single->getValues();
    assert(values.size() == single->numCompFailed() + single->numCompPassed());

    accumulator_.accumulate(values, last);
}

/**
*/
boost::optional<double> JoinedSpeed::computeResult_impl() const
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
std::vector<Joined::SectorInfo> JoinedSpeed::sectorInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                       , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference speeds"  , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"         , num_pos_inside_                    }, 
             { "#PosOutside [1]", "Number of updates outside sector"        , num_pos_outside_                   },
             { "#NoTstData [1]" , "Number of updates without tst speed data", num_no_tst_value_                  }, 
             { "OMin [m/s]"     , "Minimum of speed offset"                 , formatValue(accumulator_.min())    }, 
             { "OMax [m/s]"     , "Maximum of speed offset"                 , formatValue(accumulator_.max())    }, 
             { "OAvg [m/s]"     , "Average of speed offset"                 , formatValue(accumulator_.mean())   }, 
             { "OSDev [m/s]"    , "Standard Deviation of speed offset"      , formatValue(accumulator_.stddev()) },
             { "OVar [m^2/s^2]" , "Variance of speed offset"                , formatValue(accumulator_.var())    },
             { "#CF [1]"        , "Number of updates with failed comparison", num_comp_failed_                   }, 
             { "#CP [1]"        , "Number of updates with passed comparison", num_comp_passed_                   } };
}

/**
*/
bool JoinedSpeed::exportAsCSV(std::ofstream& strm) const
{
    loginf << "JoinedSpeed: exportAsCSV";

    strm << "speed_offset\n";
    
    auto values = getValues();

    for (auto v : values)
        strm << v << "\n";

    if (!strm)
        return false;

    return true;
}

}

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

#include "eval/results/base/presentbase.h"

namespace EvaluationRequirementResult
{

/***************************************************************************
 * PresentBase
 ***************************************************************************/

/**
*/
PresentBase::PresentBase(const std::string& no_ref_value_name)
:   no_ref_value_name_(no_ref_value_name)
{
}

/**
*/
PresentBase::PresentBase(int num_updates, 
                         int num_no_ref_pos, 
                         int num_pos_outside, 
                         int num_pos_inside,
                         int num_no_ref_val, 
                         int num_present, 
                         int num_missing,
                         const std::string& no_ref_value_name)
:   num_updates_      (num_updates)
,   num_no_ref_pos_   (num_no_ref_pos)
,   num_pos_outside_  (num_pos_outside)
,   num_pos_inside_   (num_pos_inside)
,   num_no_ref_val_   (num_no_ref_val)
,   num_present_      (num_present)
,   num_missing_      (num_missing)
,   no_ref_value_name_(no_ref_value_name)
{
}

/**
*/
int PresentBase::numUpdates() const
{
    return num_updates_;
}

/**
*/
int PresentBase::numNoRefPos() const
{
    return num_no_ref_pos_;
}

/**
*/
int PresentBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
int PresentBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
int PresentBase::numNoRefValue() const
{
    return num_no_ref_val_;
}

/**
*/
int PresentBase::numPresent() const
{
    return num_present_;
}

/**
*/
int PresentBase::numMissing() const
{
    return num_missing_;
}

/***************************************************************************
 * SinglePresentBase
 ***************************************************************************/

/**
*/
SinglePresentBase::SinglePresentBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer,
                                     unsigned int utn, 
                                     const EvaluationTargetData* target, 
                                     EvaluationManager& eval_man,
                                     const EvaluationDetails& details,
                                     int num_updates, 
                                     int num_no_ref_pos, 
                                     int num_pos_outside, 
                                     int num_pos_inside,
                                     int num_no_ref_val, 
                                     int num_present, 
                                     int num_missing,
                                     const std::string& no_ref_value_name)
:   PresentBase(num_updates, num_no_ref_pos, num_pos_outside, num_pos_inside, num_no_ref_val, num_present, num_missing, no_ref_value_name)
,   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   no_ref_value_name_(no_ref_value_name)
{
}

/**
*/
boost::optional<double> SinglePresentBase::computeResult_impl() const
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_ + num_present_ + num_missing_);

    unsigned int total = num_no_ref_val_ + num_present_ + num_missing_;

    if (total == 0)
        return {};

    return (double)(num_no_ref_val_ + num_present_) / (double)total;
}

/**
*/
unsigned int SinglePresentBase::numIssues() const
{
    return num_missing_;
}

/**
*/
std::vector<std::string> SinglePresentBase::targetTableHeadersCustom() const
{
    return { "#Up", "#NoRef", no_ref_value_name_, "#Present", "#Missing" };
}

/**
*/
std::vector<QVariant> SinglePresentBase::targetTableValuesCustom() const
{
    return { num_updates_, num_no_ref_pos_, num_no_ref_val_, num_present_, num_missing_ };
}

/**
*/
std::vector<Single::TargetInfo> SinglePresentBase::targetInfos() const
{
    QString nrvn = QString::fromStdString(no_ref_value_name_);

    return { { "#Up [1]"        , "Number of updates"                        , num_updates_     },
             { "#NoRef [1]"     , "Number of updates w/o reference position" , num_no_ref_pos_  },
             { "#NoRefPos [1]"  , "Number of updates w/o reference position ", num_no_ref_pos_  },
             { "#PosInside [1]" , "Number of updates inside sector"          , num_pos_inside_  }, 
             { "#PosOutside [1]", "Number of updates outside sector"         , num_pos_outside_ }, 
             { nrvn + " [1]"    , "Number of updates without reference code" , num_no_ref_val_  }, 
             { "#Present [1]"   , "Number of updates with present tst code"  , num_present_     }, 
             { "#Missing [1]"   , "Number of updates with missing tst code"  , num_missing_     } };
}

/**
*/
std::vector<std::string> SinglePresentBase::detailHeaders() const
{
    return { "ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", no_ref_value_name_, "#Present", "#Missing", "Comment" };
}

/**
*/
std::vector<QVariant> SinglePresentBase::detailValues(const EvaluationDetail& detail,
                                                      const EvaluationDetail* parent_detail) const
{
    return { Utils::Time::toString(detail.timestamp()).c_str(),
             detail.getValue(DetailKey::RefExists),
            !detail.getValue(DetailKey::IsNotOk).toBool(),
             detail.getValue(DetailKey::NumUpdates),
             detail.getValue(DetailKey::NumNoRef),
             detail.getValue(DetailKey::NumInside),
             detail.getValue(DetailKey::NumOutside),
             detail.getValue(DetailKey::NumNoRefVal),
             detail.getValue(DetailKey::NumPresent),
             detail.getValue(DetailKey::NumMissing),
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SinglePresentBase::detailIsOk(const EvaluationDetail& detail) const
{
    auto is_not_ok = detail.getValueAs<bool>(DetailKey::IsNotOk);
    assert(is_not_ok.has_value());

    return !is_not_ok.value();
}

/**
*/
void SinglePresentBase::addAnnotationForDetail(nlohmann::json& annotations_json, 
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
std::map<std::string, std::vector<Single::LayerDefinition>> SinglePresentBase::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SinglePresentBase::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, DetailKey::IsNotOk, true);
    }
}

/***************************************************************************
 * JoinedPresentBase
 ***************************************************************************/

/**
*/
JoinedPresentBase::JoinedPresentBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer, 
                                     EvaluationManager& eval_man,
                                     const std::string& no_ref_value_name)
:   PresentBase(no_ref_value_name)
,   JoinedProbabilityBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
unsigned int JoinedPresentBase::numIssues() const
{
    return num_missing_;
}

/**
*/
unsigned int JoinedPresentBase::numUpdates() const
{
    return num_no_ref_val_ + num_present_ + num_missing_;
}

/**
*/
void JoinedPresentBase::clearResults_impl() 
{
    num_updates_     = 0;
    num_no_ref_pos_  = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_no_ref_val_  = 0;
    num_present_     = 0;
    num_missing_     = 0;
}

/**
*/
void JoinedPresentBase::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SinglePresentBase> single_present = std::static_pointer_cast<SinglePresentBase>(single_result);

    num_updates_     += single_present->numUpdates();
    num_no_ref_pos_  += single_present->numNoRefPos();
    num_pos_outside_ += single_present->numPosOutside();
    num_pos_inside_  += single_present->numPosInside();
    num_no_ref_val_  += single_present->numNoRefValue();
    num_present_     += single_present->numPresent();
    num_missing_     += single_present->numMissing();
}

/**
*/
boost::optional<double> JoinedPresentBase::computeResult_impl() const
{
    loginf << "JoinedPresentBase: computeResult_impl:" << type()
            << " num_updates " << num_updates_
            << " num_no_ref_pos " << num_no_ref_pos_
            << " num_no_ref_val " << num_no_ref_val_
            << " num_present_id " << num_present_
            << " num_missing_id " << num_missing_;

    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_ + num_present_ + num_missing_);

    unsigned int total = num_no_ref_val_ + num_present_ + num_missing_;

    if (total == 0)
        return {};

    return (double)(num_no_ref_val_ + num_present_) / (double)total;
}

/**
*/
std::vector<Joined::SectorInfo> JoinedPresentBase::sectorInfos() const
{
    QString nrvn = QString::fromStdString(no_ref_value_name_);
    
    return { { "#Up [1]"        , "Number of updates"                        , num_updates_    },
             { "#NoRef [1]"     , "Number of updates w/o reference position" , num_no_ref_pos_ }, 
             { "#NoRefPos [1]"  , "Number of updates w/o reference position ", num_no_ref_pos_ },
             { "#PosInside [1]" , "Number of updates inside sector"          , num_pos_inside_ },
             { "#PosOutside [1]", "Number of updates outside sector"         , num_pos_outside_},
             { nrvn + " [1]"    , "Number of updates without reference code" , num_no_ref_val_ },
             { "#Present [1]"   , "Number of updates with present tst code"  , num_present_    },
             { "#Missing [1]"   , "Number of updates with missing tst code"  , num_missing_    } };
}

}

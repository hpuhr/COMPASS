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

#include "eval/results/base/falsebase.h"
#include "eval/results/base/featuredefinitions.h"

namespace EvaluationRequirementResult
{

/***************************************************************************
 * FalseBase
 ***************************************************************************/

/**
*/
FalseBase::FalseBase(const std::string& false_value_name)
:   false_value_name_(false_value_name)
{
}

/**
*/
FalseBase::FalseBase(int num_updates, 
                     int num_no_ref_pos, 
                     int num_no_ref_val, 
                     int num_pos_outside, 
                     int num_pos_inside,
                     int num_unknown, 
                     int num_correct, 
                     int num_false,
                     const std::string& false_value_name)
:   num_updates_     (num_updates)
,   num_no_ref_pos_  (num_no_ref_pos)
,   num_no_ref_val_  (num_no_ref_val)
,   num_pos_outside_ (num_pos_outside)
,   num_pos_inside_  (num_pos_inside)
,   num_unknown_     (num_unknown)
,   num_correct_     (num_correct)
,   num_false_       (num_false)
,   false_value_name_(false_value_name)
{
}

/**
*/
int FalseBase::numNoRefPos() const
{
    return num_no_ref_pos_;
}

/**
*/
int FalseBase::numNoRefValue() const
{
    return num_no_ref_val_;
}

/**
*/
int FalseBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
int FalseBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
int FalseBase::numUpdates() const
{
    return num_updates_;
}

/**
*/
int FalseBase::numUnknown() const
{
    return num_unknown_;
}

/**
*/
int FalseBase::numCorrect() const
{
    return num_correct_;
}

/**
*/
int FalseBase::numFalse() const
{
    return num_false_;
}

/***************************************************************************
 * SingleFalseBase
 ***************************************************************************/

/**
*/
SingleFalseBase::SingleFalseBase(const std::string& result_type,
                                 const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer,
                                 unsigned int utn, 
                                 const EvaluationTargetData* target, 
                                 EvaluationManager& eval_man,
                                 const EvaluationDetails& details,
                                 int num_updates, 
                                 int num_no_ref_pos, 
                                 int num_no_ref_val, 
                                 int num_pos_outside, 
                                 int num_pos_inside,
                                 int num_unknown, 
                                 int num_correct, 
                                 int num_false,
                                 const std::string& false_value_name)
:   FalseBase(num_updates, num_no_ref_pos, num_no_ref_val, num_pos_outside, num_pos_inside, num_unknown, num_correct, num_false, false_value_name)
,   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
{
}

/**
*/
boost::optional<double> SingleFalseBase::computeResult_impl(const EvaluationDetails& details) const
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_ + num_unknown_ + num_correct_ + num_false_);

    unsigned int num_total = num_correct_ + num_false_;

    if (num_total == 0)
        return {};
    
    return (double)num_false_ / (double)num_total;
}

/**
*/
unsigned int SingleFalseBase::numIssues() const
{
    return num_false_;
}

/**
*/
std::vector<std::string> SingleFalseBase::targetTableHeadersCustom() const
{
    return { "#Up", "#NoRef", "#Unknown", "#Correct", "#False" };
}

/**
*/
std::vector<QVariant> SingleFalseBase::targetTableValuesCustom() const
{
    return { num_updates_, num_no_ref_pos_ + num_no_ref_val_, num_unknown_, num_correct_, num_false_ };
}

/**
*/
std::vector<Single::TargetInfo> SingleFalseBase::targetInfos() const
{
    QString name = QString::fromStdString(false_value_name_);

    return { { "#Up [1]"        , "Number of updates"                                  , num_updates_                     }, 
             { "#NoRef [1]"     , "Number of updates w/o reference position or " + name, num_no_ref_pos_ + num_no_ref_val_}, 
             { "#NoRefPos [1]"  , "Number of updates w/o reference position "          , num_no_ref_pos_                  }, 
             { "#NoRef [1]"     , "Number of updates w/o reference " + name            , num_no_ref_val_                  }, 
             { "#PosInside [1]" , "Number of updates inside sector"                    , num_pos_inside_                  }, 
             { "#PosOutside [1]", "Number of updates outside sector"                   , num_pos_outside_                 }, 
             { "#Unknown [1]"   , "Number of updates unknown " + name                  , num_unknown_                     }, 
             { "#Correct [1]"   , "Number of updates with correct " + name             , num_correct_                     }, 
             { "#False [1]"     , "Number of updates with false " + name               , num_false_                       } };
}

/**
*/
std::vector<std::string> SingleFalseBase::detailHeaders() const
{
    return { "ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#Unknown", "#Correct", "#False", "Comment" };
}

/**
*/
std::vector<QVariant> SingleFalseBase::detailValues(const EvaluationDetail& detail,
                                                    const EvaluationDetail* parent_detail) const
{
    return { Utils::Time::toString(detail.timestamp()).c_str(),
             detail.getValue(DetailKey::RefExists),
            !detail.getValue(DetailKey::IsNotOk).toBool(),
             detail.getValue(DetailKey::NumUpdates),
             detail.getValue(DetailKey::NumNoRef),
             detail.getValue(DetailKey::NumInside),
             detail.getValue(DetailKey::NumOutside),
             detail.getValue(DetailKey::NumUnknownID),
             detail.getValue(DetailKey::NumCorrectID),
             detail.getValue(DetailKey::NumFalseID),
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SingleFalseBase::detailIsOk(const EvaluationDetail& detail) const
{
    auto is_not_ok = detail.getValueAs<bool>(DetailKey::IsNotOk);
    assert(is_not_ok.has_value());

    return !is_not_ok.value();
}

/**
*/
void SingleFalseBase::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                             const EvaluationDetail& detail, 
                                             TargetAnnotationType type,
                                             bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationPos(annotations_json, detail.position(0), AnnotationArrayType::TypeHighlight);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationPos(annotations_json, detail.position(0), is_ok ? AnnotationArrayType::TypeOk : AnnotationArrayType::TypeError);
    }
}

/***************************************************************************
 * JoinedFalseBase
 ***************************************************************************/

/**
*/
JoinedFalseBase::JoinedFalseBase(const std::string& result_type,
                                 const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer, 
                                 EvaluationManager& eval_man,
                                 const std::string& false_value_name)
:   FalseBase(false_value_name)
,   JoinedProbabilityBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
unsigned int JoinedFalseBase::numIssues() const
{
    return num_false_;
}

/**
*/
unsigned int JoinedFalseBase::numUpdates() const
{
    return num_correct_ + num_false_;
}

/**
*/
void JoinedFalseBase::clearResults_impl() 
{
    num_updates_     = 0;
    num_no_ref_pos_  = 0;
    num_no_ref_val_  = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_unknown_     = 0;
    num_correct_     = 0;
    num_false_       = 0;
}

/**
*/
void JoinedFalseBase::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SingleFalseBase> single_false = std::static_pointer_cast<SingleFalseBase>(single_result);

    num_updates_     += single_false->numUpdates();
    num_no_ref_pos_  += single_false->numNoRefPos();
    num_no_ref_val_  += single_false->numNoRefValue();
    num_pos_outside_ += single_false->numPosOutside();
    num_pos_inside_  += single_false->numPosInside();
    num_unknown_     += single_false->numUnknown();
    num_correct_     += single_false->numCorrect();
    num_false_       += single_false->numFalse();
}

/**
*/
boost::optional<double> JoinedFalseBase::computeResult_impl() const
{
    loginf << "JoinedFalseBase: computeResult_impl:" << type()
            << " num_updates " << num_updates_
            << " num_no_ref_pos " << num_no_ref_pos_
            << " num_no_ref_val " << num_no_ref_val_
            << " num_unknown " << num_unknown_
            << " num_correct " << num_correct_
            << " num_false " << num_false_;

    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

    unsigned int total = num_correct_ + num_false_;

    if (total == 0)
        return {};

    return (double)num_false_ / (double)total;
}

/**
*/
std::vector<Joined::SectorInfo> JoinedFalseBase::sectorInfos() const
{
    QString name = QString::fromStdString(false_value_name_);

    return { { "#Up [1]"        , "Number of updates"                                  , num_updates_                    },
             { "#NoRef [1]"     , "Number of updates w/o reference position or " + name, num_no_ref_pos_+num_no_ref_val_ },
             { "#NoRefPos [1]"  , "Number of updates w/o reference position "          , num_no_ref_pos_                 }, 
             { "#NoRef [1]"     , "Number of updates w/o reference " + name            , num_no_ref_val_                 }, 
             { "#PosInside [1]" , "Number of updates inside sector"                    , num_pos_inside_                 }, 
             { "#PosOutside [1]", "Number of updates outside sector"                   , num_pos_outside_                }, 
             { "#Unknown [1]"   , "Number of updates unknown " + name                  , num_unknown_                    }, 
             { "#Correct [1]"   , "Number of updates with correct " + name             , num_correct_                    }, 
             { "#False [1]"     , "Number of updates with false " + name               , num_false_                      } };
}

/**
*/
FeatureDefinitions JoinedFalseBase::getCustomAnnotationDefinitions() const
{
    FeatureDefinitions defs; 

    // return AnnotationDefinitions().addBinaryGrid("", 
    //                                              requirement_->name(), 
    //                                              DetailValueSource(SingleFalseBase::DetailKey::IsNotOk),
    //                                              GridAddDetailMode::AddEvtRefPosition,
    //                                              true,
    //                                              Qt::green,
    //                                              Qt::red);

    return defs;
}

}

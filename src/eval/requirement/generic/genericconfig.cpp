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

#include "eval/requirement/generic/genericconfig.h"
#include "eval/requirement/generic/genericconfigwidget.h"
#include "eval/requirement/generic/generic.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/mom/momcorrect.h"
#include "eval/requirement/mode_c/rocdcorrect.h"
#include "eval/requirement/speed/accelerationcorrect.h"

//using namespace Utils;
using namespace EvaluationResultsReport;
using namespace std;

namespace EvaluationRequirement
{

GenericIntegerConfig::GenericIntegerConfig(const std::string& class_id, const std::string& instance_id, const std::string& variant,
                                   Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, eval_man), variant_(variant)
{
    assert (variant_.size());
}

std::shared_ptr<Base> GenericIntegerConfig::createRequirement()
{
    //shared_ptr<Generic> req = make_shared<Generic>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    if (variant_ == "MomLongAccCorrect")
        return make_shared<MomLongAccCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);
    else if (variant_ == "MomTransAccCorrect")
        return make_shared<MomTransAccCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);
    else if (variant_ == "MomVertRateCorrect")
        return make_shared<MomVertRateCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    assert (false);
}

void GenericIntegerConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new GenericIntegerConfigWidget(*this));
    assert (widget_);
}

void GenericIntegerConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Probability [1]", "Probability",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);

}

//---------------------------------------------

GenericDoubleConfig::GenericDoubleConfig(const std::string& class_id, const std::string& instance_id, const std::string& variant,
                                           Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, eval_man), variant_(variant)
{
    assert (variant_.size());

    registerParameter("threshold", &threshold_, 0.0);
}

std::shared_ptr<Base> GenericDoubleConfig::createRequirement()
{
    //shared_ptr<Generic> req = make_shared<Generic>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    if (variant_ == "ROCDCorrect") // ft / min
        return make_shared<ROCDCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, threshold_, eval_man_);
    else if (variant_ == "AccelerationCorrect")
        return make_shared<AccelerationCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, threshold_, eval_man_);
//    else if (variant_ == "MomVertRateCorrect")
//        return make_shared<MomVertRateCorrect>(name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    assert (false);
}

void GenericDoubleConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new GenericDoubleConfigWidget(*this));
    assert (widget_);
}

void GenericDoubleConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Threshold [1]", "Threshold", threshold_}, nullptr);
    table.addRow({"Probability [1]", "Probability",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);

}

double GenericDoubleConfig::threshold() const
{
    return threshold_;
}
void GenericDoubleConfig::threshold(double value)
{
    threshold_ = value;
}

}

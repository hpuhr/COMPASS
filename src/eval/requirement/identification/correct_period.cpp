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

#include "eval/requirement/identification/correct_period.h"
#include "eval/results/identification/correct_period.h"
#include "eval/requirement/group.h"

#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

/********************************************************************************************************
 * IdentificationCorrectPeriod
 ********************************************************************************************************/

/**
 */
IdentificationCorrectPeriod::IdentificationCorrectPeriod(const std::string& name, 
                                                         const std::string& short_name, 
                                                         const std::string& group_name,
                                                         float prob, 
                                                         COMPARISON_TYPE prob_check_type, 
                                                         EvaluationManager& eval_man,
                                                         float update_interval_s, 
                                                         bool  use_miss_tolerance,
                                                         float miss_tolerance_s,
                                                         IdentificationType identification_type)
:   IntervalBase(name, 
                 short_name, 
                 group_name, 
                 prob, 
                 prob_check_type, 
                 eval_man, 
                 update_interval_s, 
                 {}, 
                 {},
                 use_miss_tolerance ? boost::optional<float>(miss_tolerance_s) : boost::optional<float>())
,   identification_type_(identification_type)
{
}

/**
 */
IdentificationCorrectPeriod::Validity IdentificationCorrectPeriod::isValid(const dbContent::TargetReport::DataID& data_id,
                                                                           const EvaluationTargetData& target_data,
                                                                           const SectorLayer& sector_layer,
                                                                           const boost::posix_time::time_duration& max_ref_time_diff) const
{
    auto cmp_res = identification_type_ == IdentificationType::AircraftID ? 
                            compareTi(data_id, target_data, max_ref_time_diff) :
                            compareTa(data_id, target_data, max_ref_time_diff);

    bool no_ref = cmp_res.first == ValueComparisonResult::Unknown_NoRefData;
    bool failed = cmp_res.first == ValueComparisonResult::Unknown_NoTstData || 
                  cmp_res.first == ValueComparisonResult::Different;

    Validity v;

    v.value   = no_ref ? Validity::Value::DataMissing : (failed ? Validity::Value::Invalid : Validity::Value::Valid);
    v.comment = no_ref ? "Reference Mode3A missing"   : (failed ? "M3A failed (" + cmp_res.second + ")" : "");

    return v;
}

/**
 */
std::shared_ptr<EvaluationRequirementResult::Single> IdentificationCorrectPeriod::createResult(
                                        const std::string& result_id,
                                        std::shared_ptr<Base> instance, 
                                        const EvaluationTargetData& target_data,
                                        const SectorLayer& sector_layer, 
                                        const std::vector<EvaluationDetail>& details,
                                        const TimePeriodCollection& periods,
                                        unsigned int sum_uis,
                                        unsigned int misses_total)
{
    return std::make_shared<EvaluationRequirementResult::SingleIdentificationCorrectPeriod>(
                                        "SingleIdentificationCorrectPeriod", 
                                        result_id, 
                                        instance, 
                                        sector_layer, 
                                        target_data.utn_, 
                                        &target_data, 
                                        eval_man_, 
                                        details, 
                                        sum_uis, 
                                        misses_total, 
                                        periods);
}

/**
 */
std::string IdentificationCorrectPeriod::probabilityName(IdentificationType identification_type)
{
    if (identification_type == IdentificationType::AircraftID)
        return "PCAIDD [%]";

    return  "PCAAD [%]";
}

/**
 */
std::string IdentificationCorrectPeriod::probabilityDescription(IdentificationType identification_type)
{
    return "Probability of Correct " + identificationName(identification_type) + " Detection";
}

/**
 */
std::string IdentificationCorrectPeriod::identificationName(IdentificationType identification_type)
{
    if (identification_type == IdentificationType::AircraftID)
        return "Aircraft ID";
    
    return "Aircraft Address";
}

/********************************************************************************************************
 * IdentificationCorrectPeriodConfig
 ********************************************************************************************************/

/**
 */
IdentificationCorrectPeriodConfig::IdentificationCorrectPeriodConfig(const std::string& class_id, 
                                                                     const std::string& instance_id,
                                                                     Group& group, 
                                                                     EvaluationStandard& standard,
                                                                     EvaluationManager& eval_man)
:   IntervalBaseConfig(class_id, instance_id, group, standard, eval_man)
{
    configure(UseMissTol);

    registerParameter("identification_type", reinterpret_cast<int*>(&identification_type_), (int)IdentificationType::AircraftAddress);
}

/**
*/
std::shared_ptr<Base> IdentificationCorrectPeriodConfig::createRequirement()
{
    shared_ptr<IdentificationCorrectPeriod> req = make_shared<IdentificationCorrectPeriod>(
                name_, 
                short_name_, 
                group_.name(), 
                prob_, 
                prob_check_type_, 
                eval_man_,
                update_interval_s_,
                use_miss_tolerance_, 
                miss_tolerance_s_,
                identification_type_);
    return req;
}

/**
*/
BaseConfigWidget* IdentificationCorrectPeriodConfig::createWidget_impl()
{
    return new IdentificationCorrectPeriodConfigWidget(*this);
}

/**
*/
std::string IdentificationCorrectPeriodConfig::probabilityDescription() const
{
    return IdentificationCorrectPeriod::probabilityDescription(identification_type_);
}

/********************************************************************************************************
 * IdentificationCorrectPeriodConfigWidget
 ********************************************************************************************************/

/**
*/
IdentificationCorrectPeriodConfigWidget::IdentificationCorrectPeriodConfigWidget(IdentificationCorrectPeriodConfig& cfg)
:   IntervalBaseConfigWidget(cfg)
{
    prob_edit_->setToolTip(QString::fromStdString(IdentificationCorrectPeriod::probabilityDescription(config().identificationType())));

    identification_type_combo_ = new QComboBox;
    identification_type_combo_->addItem("Aircraft Address");
    identification_type_combo_->addItem("Aircraft ID");
    identification_type_combo_->setCurrentIndex((int)config().identificationType());

    connect(identification_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IdentificationCorrectPeriodConfigWidget::identificationTypeChanged);

    form_layout_->addRow("Identification Type", identification_type_combo_);
}

/**
*/
IdentificationCorrectPeriodConfig& IdentificationCorrectPeriodConfigWidget::config()
{
    return static_cast<IdentificationCorrectPeriodConfig&>(config_);
}

/**
*/
void IdentificationCorrectPeriodConfigWidget::identificationTypeChanged()
{
    loginf << "IdentificationCorrectPeriodConfigWidget: identificationTypeChanged: value " << identification_type_combo_->currentIndex();

    auto id_type = (IdentificationCorrectPeriodConfig::IdentificationType)identification_type_combo_->currentIndex();

    config().identificationType(id_type);

    prob_edit_->setToolTip(QString::fromStdString(IdentificationCorrectPeriod::probabilityDescription(config().identificationType())));
}

} // namespace EvaluationRequirement

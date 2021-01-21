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

#include "eval/requirement/identification/correctconfigwidget.h"
#include "eval/requirement/identification/correctconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    IdentificationCorrectConfigWidget::IdentificationCorrectConfigWidget(IdentificationCorrectConfig& cfg)
        : BaseConfigWidget(cfg)
    {
        //        QCheckBox* require_correctness_of_all_check_{nullptr};
        require_correctness_of_all_check_ = new QCheckBox ();
        require_correctness_of_all_check_->setChecked(config().requireCorrectnessOfAll());
        connect(require_correctness_of_all_check_, &QCheckBox::clicked,
                this, &IdentificationCorrectConfigWidget::toggleRequireCorrectnessOfAllSlot);
        form_layout_->addRow("Require Correctness of All", require_correctness_of_all_check_);

        // mode a ssr code
        use_mode_a_check_ = new QCheckBox ();
        use_mode_a_check_->setChecked(config().useModeA());
        connect(use_mode_a_check_, &QCheckBox::clicked,
                this, &IdentificationCorrectConfigWidget::toggleUseModeASlot);
        form_layout_->addRow("Use Mode 3/A Code", use_mode_a_check_);

        // 24-bit mode s address
        //QCheckBox* use_ms_ta_check_{nullptr};
        use_ms_ta_check_ = new QCheckBox ();
        use_ms_ta_check_->setChecked(config().useMsTa());
        connect(use_ms_ta_check_, &QCheckBox::clicked,
                this, &IdentificationCorrectConfigWidget::toggleUseMsTaSlot);
        form_layout_->addRow("Use Mode S Target Address", use_ms_ta_check_);


        // downlinked aircraft identification
        //QCheckBox* use_ms_ti_check_{nullptr};
        use_ms_ti_check_ = new QCheckBox ();
        use_ms_ti_check_->setChecked(config().useMsTi());
        connect(use_ms_ti_check_, &QCheckBox::clicked,
                this, &IdentificationCorrectConfigWidget::toggleUseMsTiSlot);
        form_layout_->addRow("Use Mode S Target Identification", use_ms_ti_check_);


    }

    void IdentificationCorrectConfigWidget::toggleRequireCorrectnessOfAllSlot()
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: toggleRequireCorrectnessOfAllSlot";

        assert (require_correctness_of_all_check_);
        config().requireCorrectnessOfAll(require_correctness_of_all_check_->checkState() == Qt::Checked);

    }

    void IdentificationCorrectConfigWidget::toggleUseModeASlot()
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: toggleUseModeASlot";

        assert (use_mode_a_check_);
        config().useModeA(use_mode_a_check_->checkState() == Qt::Checked);
    }

    void IdentificationCorrectConfigWidget::toggleUseMsTaSlot()
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: toggleUseMsTaSlot";

        assert (use_ms_ta_check_);
        config().useMsTa(use_ms_ta_check_->checkState() == Qt::Checked);

    }

    void IdentificationCorrectConfigWidget::toggleUseMsTiSlot()
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: toggleUseMsTiSlot";

        assert (use_ms_ti_check_);
        config().useMsTi(use_ms_ti_check_->checkState() == Qt::Checked);

    }

    IdentificationCorrectConfig& IdentificationCorrectConfigWidget::config()
    {
        IdentificationCorrectConfig* config = dynamic_cast<IdentificationCorrectConfig*>(&config_);
        assert (config);

        return *config;
    }

}

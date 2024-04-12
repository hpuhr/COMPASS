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

#include "eval/requirement/generic/genericconfigwidget.h"
#include "eval/requirement/generic/genericconfig.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

GenericIntegerConfigWidget::GenericIntegerConfigWidget(GenericIntegerConfig& cfg)
    : ProbabilityBaseConfigWidget(cfg)
{
    assert (prob_edit_);
    prob_edit_->setToolTip("Probability");

    assert (check_type_box_);
}


GenericIntegerConfig& GenericIntegerConfigWidget::config()
{
    GenericIntegerConfig* config = dynamic_cast<GenericIntegerConfig*>(&config_);
    assert (config);

    return *config;
}


// ---------------------------------------------

GenericDoubleConfigWidget::GenericDoubleConfigWidget(GenericDoubleConfig& cfg)
    : ProbabilityBaseConfigWidget(cfg)
{
    assert (prob_edit_);
    prob_edit_->setToolTip("Probability");

    assert (check_type_box_);
}


GenericDoubleConfig& GenericDoubleConfigWidget::config()
{
    GenericDoubleConfig* config = dynamic_cast<GenericDoubleConfig*>(&config_);
    assert (config);

    return *config;
}

}

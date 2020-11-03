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

#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"
#include "logger.h"

using namespace std;

EvaluationStandardComboBox::EvaluationStandardComboBox(EvaluationManager& eval_man, QWidget* parent)
    : QComboBox(parent), eval_man_(eval_man)
{
    updateStandards();

    connect(this, SIGNAL(activated(const QString&)),
            this, SLOT(changedStandardSlot(const QString&)));
}

EvaluationStandardComboBox::~EvaluationStandardComboBox()
{
}

void EvaluationStandardComboBox::changedStandardSlot(const QString& standard_name)
{
    string std_name = standard_name.toStdString();

    loginf << "EvaluationStandardComboBox: changedStandardSlot: standard '" << std_name << "'";

    if (eval_man_.currentStandardName() != std_name)
    {
        loginf << "EvaluationStandardComboBox: changedStandardSlot: setting standard '" << std_name << "'";
        eval_man_.currentStandardName(std_name);
    }
}

void EvaluationStandardComboBox::setStandardName(const std::string& value)
{
    loginf << "EvaluationStandardComboBox: setStandardName: standard '" << value << "'";

    int index = findText(QString(value.c_str()));
    assert(index >= 0);
    setCurrentIndex(index);
}

void EvaluationStandardComboBox::updateStandards()
{
    loginf << "EvaluationStandardComboBox: updateStandards";

    clear();

    addItem("");

    for (auto std_it = eval_man_.standardsBegin(); std_it != eval_man_.standardsEnd(); ++std_it)
    {
        addItem(std_it->first.c_str());
    }

    if (eval_man_.hasCurrentStandard())
    {
        int index = findText(eval_man_.currentStandardName().c_str());

        if (index >= 0)
            setCurrentIndex(index);
        else
            setCurrentIndex(0);
    }
    else
        setCurrentIndex(0);
}

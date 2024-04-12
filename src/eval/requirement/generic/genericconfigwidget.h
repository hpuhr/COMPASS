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

#pragma once

#include "eval/requirement/base/probabilitybaseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{

class GenericIntegerConfig;
class GenericDoubleConfig;

class GenericIntegerConfigWidget : public ProbabilityBaseConfigWidget
{
    Q_OBJECT

public slots:

public:
    GenericIntegerConfigWidget(GenericIntegerConfig& cfg);

protected:

    GenericIntegerConfig& config();
};


class GenericDoubleConfigWidget : public ProbabilityBaseConfigWidget
{
    Q_OBJECT

  public slots:
    void thresholdValueEditSlot(QString value);

  public:
    GenericDoubleConfigWidget(GenericDoubleConfig& cfg);

  protected:
    QLineEdit* threshold_value_edit_{nullptr};

    GenericDoubleConfig& config();
};



}

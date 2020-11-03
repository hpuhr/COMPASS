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

#ifndef CREATEASSOCIATIONSTASKWIDGET_H
#define CREATEASSOCIATIONSTASKWIDGET_H

#include <taskwidget.h>

class CreateAssociationsTask;

class CreateAssociationsTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot();

public:
    CreateAssociationsTaskWidget(CreateAssociationsTask& task, QWidget* parent = 0,
                                 Qt::WindowFlags f = 0);

    virtual ~CreateAssociationsTaskWidget();

protected:
    CreateAssociationsTask& task_;
};

#endif // CREATEASSOCIATIONSTASKWIDGET_H

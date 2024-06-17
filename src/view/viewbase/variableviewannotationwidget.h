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

#include <QWidget>

class VariableView;

class QComboBox;

/**
*/
class VariableViewAnnotationWidget : public QWidget
{
    Q_OBJECT
public:
    VariableViewAnnotationWidget(const VariableView* view, QWidget* parent = nullptr);
    virtual ~VariableViewAnnotationWidget();

    void updateContent();

    std::string currentID() const;

signals:
    void idChanged();

private:
    void createUI();

    const VariableView* view_ = nullptr;

    QComboBox* combo_ = nullptr;
};

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
class QLabel;

/**
*/
class VariableViewAnnotationWidget : public QWidget
{
    Q_OBJECT
public:
    VariableViewAnnotationWidget(const VariableView* view, QWidget* parent = nullptr);
    virtual ~VariableViewAnnotationWidget();

    void updateContent();

    bool hasCurrentAnnotation() const;
    int currentGroupIdx() const;
    int currentAnnotationIdx() const;

signals:
    void currentAnnotationChanged();

private:
    void createUI();

    void groupChanged();
    void annotationChanged();

    void updateGroups(int current_idx = -1);
    void updateAnnotations(int current_idx = -1);

    const VariableView* view_ = nullptr;

    QLabel*    group_label_ = nullptr;
    QComboBox* group_combo_ = nullptr;
    QComboBox* anno_combo_  = nullptr;
};

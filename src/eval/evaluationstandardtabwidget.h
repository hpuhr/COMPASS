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

#ifndef EVALUATIONSTANDARDTABWIDGET_H
#define EVALUATIONSTANDARDTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerSettings;
class EvaluationManagerWidget;
class EvaluationStandardComboBox;

class QPushButton;
class QStackedWidget;
class QLineEdit;


class EvaluationStandardTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void changedStandardsSlot(); // eval man
    void changedCurrentStandardSlot(); // eval man

    void addStandardSlot ();
    void renameStandardSlot ();
    void copyStandardSlot ();
    void removeStandardSlot ();

    void maxRefTimeDiffEditSlot(QString value);

public:
    EvaluationStandardTabWidget(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerSettings& eval_settings_;

    std::unique_ptr<EvaluationStandardComboBox> standard_box_ {nullptr};

    QPushButton* add_button_ {nullptr};
    QPushButton* rename_button_ {nullptr};
    QPushButton* copy_button_ {nullptr};
    QPushButton* remove_button_ {nullptr};

    QStackedWidget* standards_widget_{nullptr};

    QLineEdit* max_ref_time_diff_edit_{nullptr};

    void updateButtons();
    void updateStandardStack();
};

#endif // EVALUATIONSTANDARDTABWIDGET_H

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

#ifndef EVALUATIONDATASOURCEWIDGET_H
#define EVALUATIONDATASOURCEWIDGET_H

#include <QFrame>

class ActiveDataSource;
class DBObjectComboBox;

class QCheckBox;
class QGridLayout;

class EvaluationDataSourceWidget : public QFrame
{
    Q_OBJECT

signals:
    void dboNameChangedSignal(const std::string& dbo_name);

protected slots:
    void dboNameChangedSlot();
    /// @brief Updates the sensor active checkboxes
    void toggleDataSourceSlot();


public:
    EvaluationDataSourceWidget(const std::string& title, const std::string& dbo_name,
                               std::map<int, ActiveDataSource>& data_sources,
                               QWidget* parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    virtual ~EvaluationDataSourceWidget();

protected:
    std::string title_;
    std::string dbo_name_;

    /// Container with checkboxes for all sensors (sensor number -> checkbox)
    std::map<int, ActiveDataSource>& data_sources_;

    DBObjectComboBox* dbo_combo_ {nullptr};

    QGridLayout* data_source_layout_ {nullptr};
    std::map<int, QCheckBox*> data_sources_checkboxes_;

    void updateDataSources();
    void updateCheckboxesChecked();
    void updateCheckboxesDisabled();
};

#endif // EVALUATIONDATASOURCEWIDGET_H

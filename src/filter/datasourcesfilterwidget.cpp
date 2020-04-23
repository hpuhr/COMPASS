/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datasourcesfilterwidget.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>

#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "filtermanager.h"

/**
 * Initializes members, creates GUI elements.
 */
DataSourcesFilterWidget::DataSourcesFilterWidget(DataSourcesFilter& filter,
                                                 const std::string& class_id,
                                                 const std::string& instance_id)
    : DBFilterWidget(class_id, instance_id, filter),
      filter_(filter),
      data_sources_(filter_.dataSources())
{
    QGridLayout* sensorboxlay = new QGridLayout();

    child_layout_->addLayout(sensorboxlay);

    unsigned int col, row;
    unsigned int cnt = 0;

    for (auto& it : data_sources_)
    {
        QCheckBox* radar_checkbox = new QCheckBox(tr(it.second.getName().c_str()));
        radar_checkbox->setChecked(true);
        connect(radar_checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSource()));

        assert(data_sources_checkboxes_.find(radar_checkbox) == data_sources_checkboxes_.end());
        data_sources_checkboxes_[radar_checkbox] = it.first;

        logdbg << "DataSourcesFilterWidget: createGUIElements: got sensor " << it.first << " name "
               << it.second.getName() << " active " << radar_checkbox->isChecked();

        row = 1 + cnt / 2;
        col = cnt % 2;

        sensorboxlay->addWidget(radar_checkbox, row, col);
        cnt++;
    }

    updateCheckboxesChecked();
    updateCheckboxesDisabled();
}

DataSourcesFilterWidget::~DataSourcesFilterWidget() {}

void DataSourcesFilterWidget::selectSensorsAll()
{
    logdbg << "DataSourcesFilterWidget: selectSensorsAll";

    for (auto& it : data_sources_)
        it.second.setActiveInFilter(true);

    filter_.setChanged(true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}

void DataSourcesFilterWidget::selectSensorsNone()
{
    logdbg << "DataSourcesFilterWidget: selectSensorsNone";

    for (auto& it : data_sources_)
        it.second.setActiveInFilter(false);

    filter_.setChanged(true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}

void DataSourcesFilterWidget::update()
{
    logdbg << "DataSourcesFilterWidget: update";

    updateCheckboxesChecked();
    updateCheckboxesDisabled();
}

void DataSourcesFilterWidget::updateCheckboxesChecked()
{
    logdbg << "DataSourcesFilterWidget: updateCheckboxesChecked";

    for (auto& checkit : data_sources_checkboxes_)
    {
        assert(data_sources_.find(checkit.second) != data_sources_.end());
        DataSourcesFilterDataSource& src = data_sources_.at(checkit.second);
        checkit.first->setChecked(src.isActiveInFilter());
        logdbg << "DataSourcesFilterWidget: updateCheckboxesChecked: name " << src.getName()
               << " active " << src.isActiveInFilter();
    }
}

void DataSourcesFilterWidget::updateCheckboxesDisabled()
{
    logdbg << "DataSourcesFilterWidget: updateCheckboxesDisabled: checkboxes "
           << data_sources_checkboxes_.size();


    for (auto& checkit : data_sources_checkboxes_)
    {
        assert(data_sources_.find(checkit.second) != data_sources_.end());
        DataSourcesFilterDataSource& src = data_sources_.at(checkit.second);
        checkit.first->setEnabled(src.isActiveInData());
        logdbg << "DataSourcesFilterWidget: updateCheckboxesDisabled: src " << src.getName()
               << " active " << src.isActiveInData();
    }
}

void DataSourcesFilterWidget::toggleDataSource()
{
    logdbg << "DataSourcesFilterWidget: toggleDataSource";
    QCheckBox* check = (QCheckBox*)sender();
    assert(data_sources_checkboxes_.find(check) != data_sources_checkboxes_.end());
    int number = data_sources_checkboxes_[check];

    assert(data_sources_.find(number) != data_sources_.end());
    data_sources_.at(number).setActiveInFilter(check->checkState() == Qt::Checked);

    filter_.setChanged(true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}

void DataSourcesFilterWidget::setSourcesInactive()
{
    logdbg << "DataSourcesFilterWidget: setSourcesInactive";

    createMenu(true);
}

void DataSourcesFilterWidget::createMenu(bool inactive_disabled)
{
    logdbg << "DataSourcesFilterWidget: createMenu";

    menu_.clear();

    QAction* select_all = menu_.addAction(tr("Select all"));
    connect(select_all, SIGNAL(triggered()), this, SLOT(selectSensorsAll()));

    QAction* select_none = menu_.addAction(tr("Select none"));
    connect(select_none, SIGNAL(triggered()), this, SLOT(selectSensorsNone()));

    if (!inactive_disabled)
    {
        QAction* disable_inactive = menu_.addAction(tr("Disable inactive"));
        connect(disable_inactive, SIGNAL(triggered()), this, SLOT(setSourcesInactive()));
    }
}

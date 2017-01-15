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

/*
 * SensorFilterWidget.cpp
 *
 *  Created on: Nov 4, 2012
 *      Author: sk
 */

#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>

#include "SensorFilterWidget.h"
#include "FilterManager.h"
#include "ATSDB.h"
#include "DBObjectManager.h"
#include "DBObject.h"

/**
 * Initializes members, creates GUI elements.
 */
SensorFilterWidget::SensorFilterWidget(SensorFilter &filter, std::string class_id, std::string instance_id)
: DBFilterWidget (filter, "SensorFilterWidget", instance_id), filter_ (filter), data_sources_(filter_.getDataSources())
{
    dbo_type_ = filter_.getDBOType();

    if (!ATSDB::getInstance().contains(dbo_type_))
        throw std::runtime_error ("SensorFilterWidget: constructor: type "+dbo_type_+" not contained");

    createGUIElements();

    assert (false);
    // TODO fix sources observer
    //createMenu (DBObjectManager::getInstance().getDBObject(dbo_type_)->hasActiveDataSourcesInfo());

    updateCheckboxesChecked ();
    updateCheckboxesDisabled ();
}

SensorFilterWidget::~SensorFilterWidget()
{
}

void SensorFilterWidget::createGUIElements ()
{
    logdbg  << "SensorFilterWidget: createGUIElements";

    //    QPushButton *radar_select_all = new QPushButton(tr("All"));
    //    connect(radar_select_all, SIGNAL( clicked() ), this, SLOT( selectSensorsAll() ));
    //    QPushButton *radar_select_none = new QPushButton(tr("None"));
    //    connect(radar_select_none, SIGNAL( clicked() ), this, SLOT( selectSensorsNone() ));
    //    QPushButton *dis = new QPushButton(tr("Dis"));
    //    connect(dis, SIGNAL( clicked() ), this, SLOT( setSourcesInactive() ));

    QGridLayout *sensorboxlay = new QGridLayout();
    //    sensorboxlay->addWidget(radar_select_all, 0, 0);
    //    sensorboxlay->addWidget(radar_select_none, 0, 1);
    //    sensorboxlay->addWidget(dis, 0, 2);

    child_layout_->addLayout (sensorboxlay);

    std::map<int, SensorFilterDataSource>::iterator it;

    unsigned int col, row;
    unsigned int cnt = 0;

    for (it = data_sources_.begin(); it != data_sources_.end(); it++)
    {
        QCheckBox *radar_checkbox = new QCheckBox(tr(it->second.getName().c_str()));
        radar_checkbox->setChecked(true);
        connect(radar_checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSource ()));

        assert (data_sources_checkboxes_.find (radar_checkbox) == data_sources_checkboxes_.end());
        data_sources_checkboxes_ [radar_checkbox] = it->first;

        logdbg << "SensorFilterWidget: createGUIElements: got sensor " << it->first << " name " <<  it->second.getName()
                        << " active " << radar_checkbox->isChecked();

        row = 1 + cnt / 2;
        col = cnt % 2;

        sensorboxlay->addWidget(radar_checkbox, row, col);
        cnt++;
    }
}

void SensorFilterWidget::selectSensorsAll()
{
    logdbg  << "SensorFilterWidget: selectSensorsAll";

    std::map<int, SensorFilterDataSource>::iterator it;

    for (it = data_sources_.begin(); it != data_sources_.end(); it++)
    {
        it->second.setActiveInFilter(true);
    }

    filter_.setChanged (true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}
void SensorFilterWidget::selectSensorsNone()
{
    logdbg  << "SensorFilterWidget: selectSensorsNone";

    std::map<int, SensorFilterDataSource>::iterator it;

    for (it = data_sources_.begin(); it != data_sources_.end(); it++)
    {
        it->second.setActiveInFilter(false);
    }

    filter_.setChanged (true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}

void SensorFilterWidget::update ()
{
    logdbg << "SensorFilterWidget: update";

    updateCheckboxesChecked();
    updateCheckboxesDisabled();
}

void SensorFilterWidget::updateCheckboxesChecked ()
{
    logdbg << "SensorFilterWidget: updateCheckboxesChecked";
    std::map<QCheckBox*, int>::iterator checkit;

    for (checkit = data_sources_checkboxes_.begin(); checkit != data_sources_checkboxes_.end(); checkit++)
    {
        assert (data_sources_.find (checkit->second) != data_sources_.end());
        SensorFilterDataSource &src = data_sources_[checkit->second];
        checkit->first->setChecked(src.isActiveInFilter());
    }
}


void SensorFilterWidget::updateCheckboxesDisabled ()
{
    logdbg << "SensorFilterWidget: updateCheckboxesDisabled: checkboxes " << data_sources_checkboxes_.size();
    std::map<QCheckBox*, int>::iterator checkit;

    for (checkit = data_sources_checkboxes_.begin(); checkit != data_sources_checkboxes_.end(); checkit++)
    {
        assert (data_sources_.find (checkit->second) != data_sources_.end());
        SensorFilterDataSource &src = data_sources_[checkit->second];
        checkit->first->setEnabled(src.isActiveInData());
        logdbg << "SensorFilterWidget: updateCheckboxesDisabled: src " << src.getName() << " active "
                << src.isActiveInData();
    }

}

void SensorFilterWidget::toggleDataSource ()
{
    logdbg << "SensorFilterWidget: toggleDataSource";
    QCheckBox *check = (QCheckBox *) sender();
    assert (data_sources_checkboxes_.find (check) != data_sources_checkboxes_.end());
    int number = data_sources_checkboxes_[check];

    assert (data_sources_.find (number) != data_sources_.end());
    data_sources_[number].setActiveInFilter(check->checkState() == Qt::Checked);

    filter_.setChanged (true);

    updateCheckboxesChecked();

    emit possibleFilterChange();
}

void SensorFilterWidget::setSourcesInactive ()
{
    logdbg << "SensorFilterWidget: setSourcesInactive";
    DBObject *object = DBObjectManager::getInstance().getDBObject (dbo_type_);

    assert (false);
    // TODO fix sources observer

//    assert (!object->hasActiveDataSourcesInfo());
//    object->buildActiveDataSourcesInfo();
    createMenu(true);
}

void SensorFilterWidget::createMenu (bool inactive_disabled)
{
    loginf << "SensorFilterWidget: createMenu";

    menu_.clear();

    //    QPushButton *radar_select_all = new QPushButton(tr("All"));
    //    connect(radar_select_all, SIGNAL( clicked() ), this, SLOT( selectSensorsAll() ));
    //    QPushButton *radar_select_none = new QPushButton(tr("None"));
    //    connect(radar_select_none, SIGNAL( clicked() ), this, SLOT( selectSensorsNone() ));
    //    QPushButton *dis = new QPushButton(tr("Dis"));
    //    connect(dis, SIGNAL( clicked() ), this, SLOT( setSourcesInactive() ));

    QAction *select_all = menu_.addAction(tr("Select all"));
    connect(select_all, SIGNAL(triggered()), this, SLOT(selectSensorsAll()));

    QAction *select_none = menu_.addAction(tr("Select none"));
    connect(select_none, SIGNAL(triggered()), this, SLOT(selectSensorsNone()));

    if (!inactive_disabled)
    {
        QAction *disable_inactive = menu_.addAction(tr("Disable inactive"));
        connect(disable_inactive, SIGNAL(triggered()), this, SLOT(setSourcesInactive()));
    }
}


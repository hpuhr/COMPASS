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

#include "dboeditdatasourceactionoptionswidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>

#include "logger.h"

DBOEditDataSourceActionOptionsWidget::DBOEditDataSourceActionOptionsWidget(
    DBOEditDataSourceActionOptions& options, QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), options_(&options)
{
    setContentsMargins(1, 1, 1, 1);

    QHBoxLayout* main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(1, 1, 1, 1);

    perform_check_ = new QCheckBox(options_->sourceId().c_str());
    connect(perform_check_, SIGNAL(clicked()), this, SLOT(changedPerformSlot()));
    main_layout->addWidget(perform_check_);

    action_box_ = new QComboBox();
    connect(action_box_, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
            &DBOEditDataSourceActionOptionsWidget::changedActionSlot);
    main_layout->addWidget(action_box_);

    update();

    setLayout(main_layout);
}

void DBOEditDataSourceActionOptionsWidget::update()
{
    assert(options_);
    assert(perform_check_);
    perform_check_->setChecked(options_->performFlag());

    assert(action_box_);
    action_box_->clear();

    for (auto& opt_it : *options_)
    {
        action_box_->addItem(opt_it.second.getActionString().c_str());
    }

    action_box_->setCurrentIndex(options_->currentActionId());

    perform_check_->setDisabled(options_->currentAction().getActionString() == "None");
}

void DBOEditDataSourceActionOptionsWidget::changedPerformSlot()
{
    assert(options_);
    assert(perform_check_);
    options_->performFlag(perform_check_->checkState() == Qt::Checked);
    loginf << "DBOEditDataSourceActionOptionsWidget: changedPerformSlot: "
           << options_->performFlag();
}

void DBOEditDataSourceActionOptionsWidget::changedActionSlot(int index)
{
    loginf << "DBOEditDataSourceActionOptionsWidget: changedActionSlot: index " << index;
    assert(options_);
    assert(index >= 0);
    options_->currentActionId(index);

    assert(perform_check_);
    if (index == 0)
    {
        perform_check_->setChecked(false);
        changedPerformSlot();
    }
    else if (perform_check_->checkState() != Qt::Checked)
    {
        perform_check_->setChecked(true);
        changedPerformSlot();
    }

    perform_check_->setDisabled(options_->currentAction().getActionString() == "None");
}

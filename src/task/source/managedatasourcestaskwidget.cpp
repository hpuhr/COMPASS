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

#include "managedatasourcestaskwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dboeditdatasourceswidget.h"
#include "managedatasourcestask.h"

ManageDataSourcesTaskWidget::ManageDataSourcesTaskWidget(ManageDataSourcesTask& task,
                                                         QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout_ = new QVBoxLayout();

    tab_widget_ = new QTabWidget();
    main_layout_->addWidget(tab_widget_);

    for (auto& dbo_it : COMPASS::instance().objectManager())
    {
        tab_widget_->addTab(task_.editDataSourcesWidget(dbo_it.first), dbo_it.first.c_str());
        connect(task_.editDataSourcesWidget(dbo_it.first),
                &DBOEditDataSourcesWidget::dbItemChangedSignal, this,
                &ManageDataSourcesTaskWidget::dbItemChangedSlot);
    }

    expertModeChangedSlot();

    // bottom buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        QLabel* button_label = new QLabel("Configuration Data Sources");
        button_label->setFont(font_bold);
        button_layout->addWidget(button_label);

        QPushButton* export_button_ = new QPushButton("Export All");
        connect(export_button_, &QPushButton::clicked, this,
                &ManageDataSourcesTaskWidget::exportConfigDataSourcesSlot);
        button_layout->addWidget(export_button_);

        QPushButton* clear_button_ = new QPushButton("Clear All");
        connect(clear_button_, &QPushButton::clicked, this,
                &ManageDataSourcesTaskWidget::clearConfigDataSourcesSlot);
        button_layout->addWidget(clear_button_);

        QPushButton* import_button_ = new QPushButton("Import");
        connect(import_button_, &QPushButton::clicked, this,
                &ManageDataSourcesTaskWidget::importConfigDataSourcesSlot);
        button_layout->addWidget(import_button_);

        QPushButton* auto_sync_all_button_ = new QPushButton("Auto Sync All to DB");
        connect(auto_sync_all_button_, &QPushButton::clicked, this,
                &ManageDataSourcesTaskWidget::autoSyncAllConfigDataSourcesToDB);
        button_layout->addWidget(auto_sync_all_button_);

        main_layout_->addLayout(button_layout);
    }

    setLayout(main_layout_);
}

void ManageDataSourcesTaskWidget::setCurrentWidget(DBOEditDataSourcesWidget* widget)
{
    assert(widget);
    assert(tab_widget_->indexOf(widget) != -1);
    tab_widget_->setCurrentWidget(widget);
}

void ManageDataSourcesTaskWidget::expertModeChangedSlot() {}

void ManageDataSourcesTaskWidget::exportConfigDataSourcesSlot()
{
    loginf << "ManageDataSourcesTaskWidget: exportConfigDataSourcesSlot";
    task_.exportConfigDataSources();
}

void ManageDataSourcesTaskWidget::clearConfigDataSourcesSlot()
{
    loginf << "ManageDataSourcesTaskWidget: clearConfigDataSourcesSlot";
    task_.clearConfigDataSources();
}

void ManageDataSourcesTaskWidget::importConfigDataSourcesSlot()
{
    loginf << "ManageDataSourcesTaskWidget: importConfigDataSourcesSlot";
    task_.importConfigDataSources();
}

void ManageDataSourcesTaskWidget::autoSyncAllConfigDataSourcesToDB()
{
    loginf << "ManageDataSourcesTaskWidget: autoSyncAllConfigDataSourcesToDB";
    task_.autoSyncAllConfigDataSourcesToDB();

    emit task_.statusChangedSignal(task_.name());
}

void ManageDataSourcesTaskWidget::dbItemChangedSlot()
{
    loginf << "ManageDataSourcesTaskWidget: dbItemChangedSlot";
    emit task_.statusChangedSignal(task_.name());
}

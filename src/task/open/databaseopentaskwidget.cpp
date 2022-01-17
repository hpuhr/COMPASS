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

#include "databaseopentaskwidget.h"
#include "compass.h"
#include "databaseopentask.h"
#include "sqliteconnection.h"
#include "dbinterface.h"
#include "global.h"
#include "logger.h"
#include "stringconv.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>

DatabaseOpenTaskWidget::DatabaseOpenTaskWidget(DatabaseOpenTask& task, DBInterface& db_interface,
                                               QWidget* parent)
    : TaskWidget(parent), task_(task), db_interface_(db_interface)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout_ = new QVBoxLayout();

    connection_stack_ = new QStackedWidget();

    //SQLiteConnection& connection = db_interface_.connection();

//    connection_stack_->addWidget(connection.widget());
//    connect(connection.widget(), SIGNAL(databaseOpenedSignal()), this,
//            SLOT(databaseOpenedSlot()), Qt::UniqueConnection);

    main_layout_->addWidget(connection_stack_);

    expertModeChangedSlot();

    setLayout(main_layout_);

    updateUsedConnection();
}

DatabaseOpenTaskWidget::~DatabaseOpenTaskWidget()
{
    logdbg << "DatabaseOpenTaskWidget: destructor";

    while (connection_stack_->count())
        connection_stack_->removeWidget(connection_stack_->widget(0));
}

void DatabaseOpenTaskWidget::updateUsedConnection()
{
    assert(connection_stack_);

    //connection_stack_->setCurrentWidget(db_interface_.connectionWidget());
}

void DatabaseOpenTaskWidget::databaseOpenedSlot()
{
    logdbg << "DatabaseOpenTaskWidget: databaseOpenedSlot";
    emit databaseOpenedSignal();
}

void DatabaseOpenTaskWidget::expertModeChangedSlot() {}

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

#include "dbschemamanagerwidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "compass.h"
#include "dbinterface.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbschemawidget.h"
#include "dbtable.h"
#include "global.h"
#include "logger.h"
#include "metadbtable.h"

DBSchemaManagerWidget::DBSchemaManagerWidget(DBSchemaManager& manager, QWidget* parent,
                                             Qt::WindowFlags f)
    : QWidget(parent), manager_(manager)
{
    // unsigned int frame_width = FRAME_SIZE;
    setContentsMargins(0, 0, 0, 0);

    QFont font_bold;
    font_bold.setBold(true);

    //    setFrameStyle(QFrame::Panel | QFrame::Raised);
    //    setLineWidth(frame_width);

    QVBoxLayout* layout = new QVBoxLayout();

    QHBoxLayout* button_layout = new QHBoxLayout();

    lock_button_ = new QPushButton(tr("Lock"));
    connect(lock_button_, &QPushButton::clicked, this, &DBSchemaManagerWidget::lockSchemaSlot);
    button_layout->addWidget(lock_button_);

    layout->addLayout(button_layout);
    layout->addStretch();

    schema_widgets_ = new QStackedWidget();
    layout->addWidget(schema_widgets_);

    setLayout(layout);

    // setDisabled(true);
}

DBSchemaManagerWidget::~DBSchemaManagerWidget() {}


void DBSchemaManagerWidget::lockSchemaSlot()
{
    logdbg << "DBSchemaManagerWidget: lockSchemaSlot";

    manager_.lock();
    emit schemaLockedSignal();
}

void DBSchemaManagerWidget::lock()
{
    locked_ = true;

    assert(lock_button_);
    lock_button_->setDisabled(true);
}

void DBSchemaManagerWidget::databaseOpenedSlot()
{
    setDisabled(false);

    DBSchemaWidget* widget = manager_.getCurrentSchema().widget();
    schema_widgets_->addWidget(widget);
}

// and who are you? the proud lord said...
// void DBSchemaWidget::addRDLSchema ()
//{
//    assert (new_schema_name_edit_);
//    manager_.addRDLSchema(new_schema_name_edit_->text().toStdString());
//    updateSchemaCombo();
//}

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

#include "dbschemamanagerwidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "atsdb.h"
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

    QLabel* db_schema_label = new QLabel(tr("Database Schema"));
    db_schema_label->setFont(font_bold);
    layout->addWidget(db_schema_label);

    schema_select_ = new QComboBox();
    connect(schema_select_, SIGNAL(activated(const QString&)), this,
            SLOT(schemaSelectedSlot(const QString&)));

    layout->addWidget(schema_select_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    add_button_ = new QPushButton(tr("Add"));
    connect(add_button_, &QPushButton::clicked, this, &DBSchemaManagerWidget::addSchemaSlot);
    button_layout->addWidget(add_button_);

    delete_button_ = new QPushButton(tr("Delete"));
    connect(delete_button_, &QPushButton::clicked, this, &DBSchemaManagerWidget::deleteSchemaSlot);
    button_layout->addWidget(delete_button_);

    lock_button_ = new QPushButton(tr("Lock"));
    connect(lock_button_, &QPushButton::clicked, this, &DBSchemaManagerWidget::lockSchemaSlot);
    button_layout->addWidget(lock_button_);

    layout->addLayout(button_layout);
    layout->addStretch();

    schema_widgets_ = new QStackedWidget();
    layout->addWidget(schema_widgets_);

    updateSchemas();

    setLayout(layout);

    // setDisabled(true);
}

DBSchemaManagerWidget::~DBSchemaManagerWidget() {}

void DBSchemaManagerWidget::addSchemaSlot()
{
    logdbg << "DBSchemaManagerWidget: addSchemaSlot";

    bool ok;
    QString text = QInputDialog::getText(
        this, tr("Schema Name"), tr("Specify a (unique) schema name:"), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        manager_.addEmptySchema(text.toStdString());
        updateSchemas();
    }
}

void DBSchemaManagerWidget::deleteSchemaSlot()
{
    logdbg << "DBSchemaManagerWidget: deleteSchemaSlot";

    manager_.deleteCurrentSchema();

    updateSchemas();
}

void DBSchemaManagerWidget::lockSchemaSlot()
{
    logdbg << "DBSchemaManagerWidget: lockSchemaSlot";

    if (!manager_.hasCurrentSchema())
    {
        QMessageBox msgBox;
        msgBox.setText("A schema needs to be selected.");
        msgBox.exec();
        return;
    }

    manager_.lock();
    emit schemaLockedSignal();
}

void DBSchemaManagerWidget::schemaSelectedSlot(const QString& value)
{
    loginf << "DBSchemaManagerWidget: schemaSelectedSlot: '" << value.toStdString() << "'";

    if (value.size() > 0)
    {
        manager_.setCurrentSchema(value.toStdString());

        if (!manager_.getCurrentSchema().existsInDB())
            schema_select_->setStyleSheet(
                "QComboBox { background: rgb(255, 100, 100); selection-background-color:"
                " rgb(255, 200, 200); }");
        else
            schema_select_->setStyleSheet(
                "QComboBox { background: rgb(255, 255, 255); selection-background-color:"
                " rgb(200, 200, 200); }");

        showCurrentSchemaWidget();

        delete_button_->setDisabled(false);
    }
    else
        delete_button_->setDisabled(true);
}

void DBSchemaManagerWidget::lock()
{
    locked_ = true;

    assert(schema_select_);
    schema_select_->setDisabled(true);

    assert(add_button_);
    add_button_->setDisabled(true);

    assert(delete_button_);
    delete_button_->setDisabled(true);

    assert(lock_button_);
    lock_button_->setDisabled(true);
}

void DBSchemaManagerWidget::updateSchemas()
{
    assert(schema_select_);

    schema_select_->clear();

    bool schema_wrong = false;

    for (auto it : manager_.getSchemas())
    {
        schema_select_->addItem(it.first.c_str());
    }

    if (manager_.hasCurrentSchema())
    {
        std::string current_schema = manager_.getCurrentSchemaName();
        int index = schema_select_->findText(current_schema.c_str());
        if (index != -1)  // -1 for not found
        {
            schema_select_->setCurrentIndex(index);
            showCurrentSchemaWidget();
        }
        else
            schema_wrong = true;

        if (!manager_.getCurrentSchema().existsInDB())
            schema_wrong = true;
    }
    else
        schema_wrong = true;

    if (schema_wrong && ATSDB::instance().interface().ready())
        schema_select_->setStyleSheet(
            "QComboBox { background: rgb(255, 100, 100); selection-background-color:"
            " rgb(255, 200, 200); }");
    else
        schema_select_->setStyleSheet(
            "QComboBox { background: rgb(255, 255, 255); selection-background-color:"
            " rgb(200, 200, 200); }");
}

void DBSchemaManagerWidget::showCurrentSchemaWidget()
{
    assert(schema_widgets_);
    while (schema_widgets_->count() > 0)
        schema_widgets_->removeWidget(schema_widgets_->widget(0));

    if (manager_.hasCurrentSchema())
    {
        DBSchemaWidget* widget = manager_.getCurrentSchema().widget();
        schema_widgets_->addWidget(widget);
    }
}

void DBSchemaManagerWidget::databaseOpenedSlot()
{
    setDisabled(false);
    updateSchemas();
}

// and who are you? the proud lord said...
// void DBSchemaWidget::addRDLSchema ()
//{
//    assert (new_schema_name_edit_);
//    manager_.addRDLSchema(new_schema_name_edit_->text().toStdString());
//    updateSchemaCombo();
//}

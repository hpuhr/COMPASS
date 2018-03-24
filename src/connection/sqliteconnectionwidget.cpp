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

#include "sqliteconnectionwidget.h"
#include "sqliteconnection.h"
#include "mysqlserver.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>

SQLiteConnectionWidget::SQLiteConnectionWidget(SQLiteConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *files_label = new QLabel ("File Selection");
    files_label->setFont(font_bold);
    layout->addWidget(files_label);

    file_list_ = new QListWidget ();
    file_list_->setWordWrap(true);
    file_list_->setTextElideMode (Qt::ElideNone);
    file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
    file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
    layout->addWidget(file_list_);

    add_button_ = new QPushButton ("Add");
    connect (add_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
    layout->addWidget(add_button_);

    delete_button_ = new QPushButton ("Remove");
    connect (delete_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
    layout->addWidget(delete_button_);
    layout->addStretch();

    open_button_ = new QPushButton ("Open");
    connect (open_button_, SIGNAL(clicked()), this, SLOT(openFileSlot()));
    layout->addWidget(open_button_);

    updateFileListSlot ();

    setLayout (layout);
}

void SQLiteConnectionWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add SQLite3 File"));

    if (filename.size() > 0)
    {
        if (!connection_.hasFile(filename.toStdString()))
            connection_.addFile(filename.toStdString());
    }
}

void SQLiteConnectionWidget::deleteFileSlot ()
{
    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "SQLite3 File Deletion Failed",
                                 "Please select a file in the list.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();

    if (filename.size() > 0)
    {
        assert (connection_.hasFile(filename.toStdString()));
        connection_.removeFile (filename.toStdString());
    }
}

void SQLiteConnectionWidget::openFileSlot ()
{
    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "SQLite3 Database Open Failed",
                                 "Please select a file in the list.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }


    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (connection_.hasFile(filename.toStdString()));
        connection_.openFile(filename.toStdString());

        open_button_->setDisabled(true);

        emit databaseOpenedSignal();
    }

}

void SQLiteConnectionWidget::updateFileListSlot ()
{
    file_list_->clear();

    for (auto it : connection_.fileList())
    {
        QListWidgetItem *item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == connection_.lastFilename())
            file_list_->setCurrentItem(item);
    }
}


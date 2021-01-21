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

#include "sqliteconnectionwidget.h"
#include "logger.h"
#include "mysqlserver.h"
#include "sqliteconnection.h"
#include "files.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

using namespace Utils;
using namespace std;

SQLiteConnectionWidget::SQLiteConnectionWidget(SQLiteConnection& connection, QWidget* parent)
    : QWidget(parent), connection_(connection)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* files_label = new QLabel("File Selection");
    files_label->setFont(font_bold);
    layout->addWidget(files_label);

    file_list_ = new QListWidget();
    file_list_->setWordWrap(true);
    file_list_->setTextElideMode(Qt::ElideNone);
    file_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
    file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(file_list_);

    QHBoxLayout* file_button_layout = new QHBoxLayout();

    new_button_ = new QPushButton("New");
    connect(new_button_, &QPushButton::clicked, this, &SQLiteConnectionWidget::newFileSlot);
    file_button_layout->addWidget(new_button_);

    add_button_ = new QPushButton("Add");
    connect(add_button_, &QPushButton::clicked, this, &SQLiteConnectionWidget::addFileSlot);
    file_button_layout->addWidget(add_button_);

    delete_button_ = new QPushButton("Remove");
    connect(delete_button_, &QPushButton::clicked, this, &SQLiteConnectionWidget::deleteFileSlot);
    file_button_layout->addWidget(delete_button_);

    delete_all_button_ = new QPushButton("Remove All");
    connect(delete_all_button_, &QPushButton::clicked, this, &SQLiteConnectionWidget::deleteAllFilesSlot);
    file_button_layout->addWidget(delete_all_button_);

    layout->addLayout(file_button_layout);

    layout->addSpacing(50);

    open_button_ = new QPushButton("Open");
    connect(open_button_, &QPushButton::clicked, this, &SQLiteConnectionWidget::openFileSlot);
    layout->addWidget(open_button_);

    updateFileListSlot();

    setLayout(layout);
}

SQLiteConnectionWidget::~SQLiteConnectionWidget()
{
    logdbg << "SQLiteConnectionWidget: destructor";
}

void SQLiteConnectionWidget::newFileSlot()
{
    string filename = QFileDialog::getSaveFileName(this, "New SQLite3 File").toStdString();

    if (filename.size() > 0)
    {
        if (Files::fileExists(filename))
        {
            // confirmation already done by dialog
            loginf << "SQLiteConnectionWidget: newFileSlot: deleting pre-existing file '" << filename << "'";
            Files::deleteFile(filename);
        }

        if (!connection_.hasFile(filename))
            connection_.addFile(filename);
    }
}

void SQLiteConnectionWidget::addFileSlot()
{
    QString filename = QFileDialog::getOpenFileName(this, "Add SQLite3 File");

    if (filename.size() > 0)
        addFile(filename.toStdString());
}

void SQLiteConnectionWidget::addFile(const std::string& filename)
{
    if (!connection_.hasFile(filename))
    {
        loginf << "SQLiteConnectionWidget: addFile: filename '" << filename << "'";
        connection_.addFile(filename);
    }
}

void SQLiteConnectionWidget::selectFile(const std::string& filename)
{
    assert (connection_.hasFile(filename));
    QList<QListWidgetItem*> items = file_list_->findItems(filename.c_str(), Qt::MatchExactly);
    assert (items.size() == 1);
    file_list_->setCurrentItem(items.at(0));
}

void SQLiteConnectionWidget::deleteFileSlot()
{
    if (!file_list_->currentItem())
    {
        QMessageBox m_warning(QMessageBox::Warning, "SQLite3 File Deletion Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();

    if (filename.size() > 0)
    {
        assert(connection_.hasFile(filename.toStdString()));
        connection_.removeFile(filename.toStdString());
    }
}

void SQLiteConnectionWidget::deleteAllFilesSlot()
{
    loginf << "SQLiteConnectionWidget: deleteAllFilesSlot";
    connection_.removeAllFiles();
}

void SQLiteConnectionWidget::openFileSlot()
{
    loginf << "SQLiteConnectionWidget: openFileSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning(QMessageBox::Warning, "SQLite3 Database Open Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    // QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        open_button_->setDisabled(true);

        assert(connection_.hasFile(filename.toStdString()));
        connection_.openFile(filename.toStdString());

        emit databaseOpenedSignal();
    }
    // QApplication::restoreOverrideCursor();
    loginf << "SQLiteConnectionWidget: openFileSlot: done";
}

void SQLiteConnectionWidget::updateFileListSlot()
{
    file_list_->clear();

    for (auto it : connection_.fileList())
    {
        QListWidgetItem* item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == connection_.lastFilename())
            file_list_->setCurrentItem(item);
    }
}

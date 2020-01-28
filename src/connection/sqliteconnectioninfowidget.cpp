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

#include "sqliteconnectioninfowidget.h"
#include "sqliteconnection.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

SQLiteConnectionInfoWidget::SQLiteConnectionInfoWidget(SQLiteConnection& connection, QWidget* parent)
    : QWidget(parent), connection_(connection)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* layout = new QVBoxLayout ();

    QLabel* main_label = new QLabel ("SQLite Database");
    main_label->setFont(font_bold);
    layout->addWidget(main_label);

    QGridLayout* grid = new QGridLayout ();

    grid->addWidget(new QLabel ("Database"), 0, 0);

    database_label_ = new QLabel ();
    database_label_->setWordWrap(true);
    grid->addWidget(database_label_, 0, 1);

    grid->addWidget(new QLabel ("Status"), 1, 0);

    status_label_ = new QLabel ();
    grid->addWidget(status_label_, 1, 1);

    layout->addLayout(grid);

    setLayout (layout);

    updateSlot();
}

void SQLiteConnectionInfoWidget::updateSlot()
{
    loginf << "SQLiteConnectionInfoWidget: updateSlot";

    assert (database_label_);
    assert (status_label_);

    if (connection_.ready())
        database_label_->setText (connection_.identifier().c_str());
    else
        database_label_->setText ("");

    status_label_->setText(connection_.status().c_str());
}



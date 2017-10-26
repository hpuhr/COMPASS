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

SQLiteConnectionInfoWidget::SQLiteConnectionInfoWidget(SQLiteConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection), database_(nullptr), status_(nullptr)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("SQLite Database");
    main_label->setFont(font_bold);
    layout->addWidget(main_label);

    QGridLayout *grid = new QGridLayout ();

//    QLabel *server_label = new QLabel ("Server");
//    grid->addWidget(server_label, 0, 0);

//    server_ = new QLabel ();
//    grid->addWidget(server_, 0, 1);

    QLabel *database_label = new QLabel ("Database");
    grid->addWidget(database_label, 0, 0);

    database_ = new QLabel ();
    grid->addWidget(database_, 0, 1);

    QLabel *status_label = new QLabel ("Status");
    grid->addWidget(status_label, 1, 0);

    status_ = new QLabel ();
    grid->addWidget(status_, 1, 1);

    layout->addLayout(grid);

    setLayout (layout);
}

void SQLiteConnectionInfoWidget::updateSlot()
{
    logdbg << "SQLiteConnectionInfoWidget: updateSlot";

//    assert (server_);
    assert (database_);
    assert (status_);

//    if (connection_.ready())
//    {
//        MySQLServer &server = connection_.connectedServer();

//        server_->setText(server.host().c_str());
//        database_->setText(server.database().c_str());
//    }
//    else
//    {
//        server_->setText("Unknown");
//        database_->setText("Unknown");
//    }
    status_->setText(connection_.status().c_str());
}



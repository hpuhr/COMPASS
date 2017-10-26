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

#ifndef MYSQLPPCONNECTIONINFOWIDGET_H
#define MYSQLPPCONNECTIONINFOWIDGET_H

#include <QWidget>

class MySQLppConnection;
class QLabel;

class MySQLppConnectionInfoWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateSlot ();

public:
    explicit MySQLppConnectionInfoWidget(MySQLppConnection &connection, QWidget *parent = 0);

protected:
    MySQLppConnection &connection_;

    QLabel *server_;
    QLabel *database_;
    QLabel *status_;
};

#endif // MYSQLPPCONNECTIONINFOWIDGET_H

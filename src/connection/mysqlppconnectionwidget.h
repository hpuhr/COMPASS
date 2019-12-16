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

#ifndef MYSQLPPCONNECTIONWIDGET_H
#define MYSQLPPCONNECTIONWIDGET_H

#include <QWidget>
#include <QMenu>

class MySQLppConnection;
class QComboBox;
class QVBoxLayout;
class QPushButton;
class QStackedWidget;

class MySQLppConnectionWidget : public QWidget
{
    Q_OBJECT

signals:
    void databaseOpenedSignal ();

public slots:
    void addServerSlot ();
    void deleteServerSlot ();
    void serverSelectedSlot (const QString &value);
    void serverConnectedSlot ();
    void databaseOpenedSlot ();

    void showImportMenuSlot ();
    void importSQLTextSlot();
    void importSQLTextFromArchiveSlot();

public:
    explicit MySQLppConnectionWidget(MySQLppConnection& connection, QWidget* parent = 0);
    virtual ~MySQLppConnectionWidget();

protected:
    MySQLppConnection& connection_;

    QComboBox* server_select_ {nullptr};
    QPushButton* add_button_ {nullptr};
    QPushButton* delete_button_ {nullptr};

    QPushButton* import_button_ {nullptr};
    QMenu import_menu_;

    QStackedWidget* server_widgets_ {nullptr};

    void updateServers();
};

#endif // MYSQLPPCONNECTIONWIDGET_H

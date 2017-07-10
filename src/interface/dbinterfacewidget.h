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

#ifndef DBINTERFACEWIDGET_H_
#define DBINTERFACEWIDGET_H_

#include <QFrame>
#include <QComboBox>

class DBInterface;
class QStackedWidget;

/**
 * @brief Widget for choosing a database system and parameters
 */
class DBInterfaceWidget : public QFrame
{
    Q_OBJECT

public slots:
    void databaseTypeSelectSlot ();
    void databaseOpenedSlot ();

signals:
    void databaseOpenedSignal ();

public:
    /// @brief Constructor
    explicit DBInterfaceWidget(DBInterface &interface, QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBInterfaceWidget();

protected:
    DBInterface &interface_;

    //QVBoxLayout *connection_layout_;
    QStackedWidget *connection_stack_;

    void useConnection (std::string connection_type);
};

#endif /* DBINTERFACEWIDGET_H_ */

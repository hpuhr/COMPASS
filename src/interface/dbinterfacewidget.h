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

#ifndef DBINTERFACEWIDGET_H_
#define DBINTERFACEWIDGET_H_

#include <QComboBox>
#include <QWidget>

class DBInterface;
class QStackedWidget;

/**
 * @brief Widget for choosing a database system and parameters
 */
class DBInterfaceWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void databaseTypeSelectSlot();
    void databaseOpenedSlot();

  signals:
    void databaseOpenedSignal();

  public:
    /// @brief Constructor
    explicit DBInterfaceWidget(DBInterface& interface, QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBInterfaceWidget();

  protected:
    DBInterface& interface_;

    QStackedWidget* connection_stack_;

    void useConnection(std::string connection_type);
};

#endif /* DBINTERFACEWIDGET_H_ */

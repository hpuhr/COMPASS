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

#ifndef DBSCHEMAMANAGERWIDGET_H
#define DBSCHEMAMANAGERWIDGET_H

#include <QWidget>

class DBSchemaManager;

class QLineEdit;
class QComboBox;
class QPushButton;
class QStackedWidget;

class DBSchemaManagerWidget : public QWidget
{
    Q_OBJECT

  signals:
    void schemaLockedSignal();

  public:
    explicit DBSchemaManagerWidget(DBSchemaManager& manager, QWidget* parent = 0,
                                   Qt::WindowFlags f = 0);
    virtual ~DBSchemaManagerWidget();

  public slots:
    void databaseOpenedSlot();

    void lockSchemaSlot();
    /// @brief Sets the schema

  public:
    void lock();

  protected:
    DBSchemaManager& manager_;

    QPushButton* lock_button_{nullptr};

    QStackedWidget* schema_widgets_{nullptr};

    bool locked_{false};
};

#endif  // DBSCHEMAMANAGERWIDGET_H

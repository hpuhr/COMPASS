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

#ifndef DBOBJECTINFOWIDGET_H_
#define DBOBJECTINFOWIDGET_H_

#include <QWidget>
#include <map>

class QLabel;
class QCheckBox;
class QPushButton;
class QGridLayout;

class DBObject;

/**
 * @brief Edit widget for a DBObject
 */
class DBObjectInfoWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadChangedSlot();
    void updateSlot();

public:
    /// @brief Constructor
    DBObjectInfoWidget(DBObject& object, QWidget* parent=0, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~DBObjectInfoWidget();

private:
    /// @brief DBObject to be managed
    DBObject &object_;

    QGridLayout* main_layout_ {nullptr};
    QCheckBox* main_check_ {nullptr};

    QLabel* status_label_ {nullptr};
    QLabel* loaded_count_label_ {nullptr};
};

#endif /* DBOBJECTINFOWIDGET_H_ */

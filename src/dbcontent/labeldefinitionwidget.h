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

#ifndef DBOLABELDEFINITIONWIDGET_H
#define DBOLABELDEFINITIONWIDGET_H

#include <QStringList>
#include <QWidget>

#include "dbcontent/labeldefinition.h"

class QTableWidget;
class QTableWidgetItem;

namespace dbContent
{

class LabelDefinitionWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void cellChangedSlot(int row, int column);

  public:
    LabelDefinitionWidget(LabelDefinition* definition);
    virtual ~LabelDefinitionWidget();

  private:
    bool seperate_window_;
    // QWidget *target_;
    LabelDefinition* definition_;
    QTableWidget* table_;
    // std::map <std::string, DBOLabelEntry *> &entries_;
    QStringList list_;

    // std::vector<std::vector <QTableWidgetItem*>> table_items_;

    void setTable();
};

}

#endif  // DBOLABELDEFINITIONWIDGET_H

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

#ifndef DBCONTENT_VARIABLEORDEREDSETWIDGET_H_
#define DBCONTENT_VARIABLEORDEREDSETWIDGET_H_

class QListWidget;

#include <QMenu>
#include <QWidget>

#include "dbcontent/variable/variableorderedset.h"
#include "dbcontent/variable/variableselectionwidget.h"

namespace dbContent
{

class VariableOrderedSetWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateVariableListSlot();
    void removeSlot();
    void moveUpSlot();
    void moveDownSlot();

  protected slots:
    void triggerSlot(QAction* action);
    void showMenuSlot();

  public:
    VariableOrderedSetWidget(VariableOrderedSet& set, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~VariableOrderedSetWidget();

  protected:
    VariableOrderedSet& set_;
    QMenu menu_;

    QListWidget* list_widget_{nullptr};
    int current_index_{-1};

    void updateMenuEntries();
};

}

#endif /* DBCONTENT_VARIABLEORDEREDSETWIDGET_H_ */

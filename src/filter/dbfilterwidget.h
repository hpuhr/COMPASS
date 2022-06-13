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

#ifndef DBFILTERWIDGET_H_
#define DBFILTERWIDGET_H_

#include <QFrame>
#include <QMenu>

class DBFilter;

class QWidget;
class QCheckBox;
class QVBoxLayout;
class QPushButton;

class DBFilterWidget : public QFrame
{
    Q_OBJECT

private slots:
    void toggleVisible();

    void toggleAnd(); // not used

    void toggleActive();

    void possibleSubFilterChange();
    void reset();
    void deleteFilter();
    void filterEditSlot();

protected slots:
    void showMenuSlot();

signals:
    void possibleFilterChange();
    void filterEdit(DBFilter* filter);
    void deleteFilterSignal(DBFilter* filter);

public:
    DBFilterWidget(DBFilter& filter);
    virtual ~DBFilterWidget();

    void addChildWidget(QWidget* widget);
    void updateChildWidget();

    virtual void update(void);

    void setInvisible();

protected:
    DBFilter& filter_;

    QWidget* child_ {nullptr}; // Child widget from DBFilter

    QCheckBox* visible_checkbox_ {nullptr};
    QCheckBox* active_checkbox_ {nullptr};

    QPushButton* manage_button_ {nullptr};

    QVBoxLayout* child_layout_ {nullptr};

    QMenu menu_;

    void createMenu();
};

#endif /* DBFILTERWIDGET_H_ */

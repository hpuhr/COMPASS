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

#ifndef METADBOVARIABLEWIDGET_H_
#define METADBOVARIABLEWIDGET_H_

#include <QWidget>
#include <map>

class QLineEdit;
class QComboBox;
class QPushButton;
class QTextEdit;

class MetaDBOVariable;
class QGridLayout;
class DBContentVariableSelectionWidget;

class MetaDBOVariableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void metaVariableChangedSignal();

  public slots:
    void editNameSlot();
    void subVariableChangedSlot();

    void updateSlot();

  public:
    MetaDBOVariableWidget(MetaDBOVariable& variable, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~MetaDBOVariableWidget();

    void lock();
    void unlock();

  private:
    MetaDBOVariable& variable_;

    QLineEdit* name_edit_{nullptr};
    QLineEdit* description_edit_{nullptr};

    QGridLayout* grid_layout_{nullptr};

    bool locked_{false};

    std::map<DBContentVariableSelectionWidget*, std::string> selection_widgets_;
};

#endif /* METADBOVARIABLEWIDGET_H_ */

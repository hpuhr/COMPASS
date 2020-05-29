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

#ifndef FILTERMANAGERWIDGET_H
#define FILTERMANAGERWIDGET_H

#include <QFrame>

class FilterManager;
class FilterGeneratorWidget;

class QPushButton;
class QVBoxLayout;

class FilterManagerWidget : public QFrame
{
    Q_OBJECT

  signals:

  public slots:
    void addFilterSlot();
    void updateFiltersSlot();
    void filterWidgetActionSlot(bool result);

    void databaseOpenedSlot();

  public:
    explicit FilterManagerWidget(FilterManager& manager, QWidget* parent = 0,
                                 Qt::WindowFlags f = 0);
    virtual ~FilterManagerWidget();

  protected:
    FilterManager& filter_manager_;

    FilterGeneratorWidget* filter_generator_widget_;

    QVBoxLayout* filter_layout_;

    QPushButton* add_button_;
};

#endif  // FILTERMANAGERWIDGET_H

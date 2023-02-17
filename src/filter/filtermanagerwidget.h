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

#ifndef FILTERMANAGERWIDGET_H
#define FILTERMANAGERWIDGET_H

#include <QFrame>
#include <QMenu>

class FilterManager;
class FilterGeneratorWidget;

class QPushButton;
class QVBoxLayout;
class QCheckBox;

class FilterManagerWidget : public QWidget
{
    Q_OBJECT

  signals:

  public slots:
    void toggleUseFilters();

    void editClickedSlot();

    void addFilterSlot();
    void updateFiltersSlot();
    void filterWidgetActionSlot(bool result);

    void databaseOpenedSlot();

    void updateUseFilters ();

  public:
    explicit FilterManagerWidget(FilterManager& manager, QWidget* parent = 0,
                                 Qt::WindowFlags f = 0);
    virtual ~FilterManagerWidget();

    QCheckBox* filtersCheckBox() const;

protected:
    FilterManager& filter_manager_;

    QMenu edit_menu_;

    FilterGeneratorWidget* filter_generator_widget_;

    QCheckBox* filters_check_ {nullptr};
    QVBoxLayout* ds_filter_layout0_ {nullptr};
    QVBoxLayout* ds_filter_layout1_ {nullptr};
};

#endif  // FILTERMANAGERWIDGET_H

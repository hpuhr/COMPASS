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

#ifndef DATASOURCESLOADWIDGET_H_
#define DATASOURCESLOADWIDGET_H_

#include <QWidget>
#include <QMenu>

#include <map>

class DataSourceManager;

class QGridLayout;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;

namespace dbContent
{
    class DBDataSourceWidget;
}

class DataSourcesLoadWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadDSTypeChangedSlot();
    void loadDSChangedSlot();

    void editClickedSlot();

    void selectAllDSTypesSlot();
    void deselectAllDSTypesSlot();

    void selectAllDataSourcesSlot();
    void deselectAllDataSourcesSlot();

    void toogleShowCountsSlot();

public:
    DataSourcesLoadWidget(DataSourceManager& ds_man);
    virtual ~DataSourcesLoadWidget();

    void updateContent();
    void loadingDone();

private:
    DataSourceManager& ds_man_;

    QMenu edit_menu_;

    QGridLayout* type_layout_{nullptr};

    QLabel* associations_label_{nullptr};

    std::map<std::string, QCheckBox*> ds_type_boxes_;

    std::map<std::string, dbContent::DBDataSourceWidget*> ds_widgets_;

    void clearAndCreateContent();

    void clear();
    void arrangeSourceWidgetWidths();
};

#endif /* DATASOURCESLOADWIDGET_H_ */

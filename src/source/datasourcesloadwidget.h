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

#pragma once

#include <QWidget>
#include <QMenu>

#include <map>

class DataSourceManager;

class QGridLayout;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

namespace dbContent
{
    class DBDataSourceWidget;
}

/**
 */
class DataSourcesLoadWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadDSTypeChangedSlot();
    //void loadDSChangedSlot();

    void editClickedSlot();

    void selectAllDSTypesSlot();
    void deselectAllDSTypesSlot();

    void selectAllDataSourcesSlot();
    void deselectAllDataSourcesSlot();
    void selectDSTypeSpecificDataSourcesSlot();
    void deselectDSTypeSpecificDataSourcesSlot();

    void deselectAllLinesSlot();
    void selectSpecificLineSlot();

    void toogleShowCountsSlot();

public:
    DataSourcesLoadWidget(DataSourceManager& ds_man);
    virtual ~DataSourcesLoadWidget();

    void updateContent(bool recreate_required = false);
    void loadingDone();

private:
    DataSourceManager& ds_man_;

    QMenu edit_menu_;

    QGridLayout* type_layout_{nullptr};

    QLabel* ts_min_label_{nullptr};
    QLabel* ts_max_label_{nullptr};
    QLabel* associations_label_{nullptr};

    std::map<std::string, QCheckBox*> ds_type_boxes_;

    std::map<std::string, dbContent::DBDataSourceWidget*> ds_widgets_;

    void clearAndCreateContent();

    void clear();
    void arrangeSourceWidgetWidths();
};

/**
 */
class DataSourcesLoadTreeWidget : public QWidget
{
    Q_OBJECT
public:
    DataSourcesLoadTreeWidget(DataSourceManager& ds_man);
    virtual ~DataSourcesLoadTreeWidget();

    void updateContent(bool recreate_required = false);
    void loadingDone();

public slots:
    void loadDSTypeChangedSlot();
    //void loadDSChangedSlot();

    void editClickedSlot();

    void selectAllDSTypesSlot();
    void deselectAllDSTypesSlot();

    void selectAllDataSourcesSlot();
    void deselectAllDataSourcesSlot();
    void selectDSTypeSpecificDataSourcesSlot();
    void deselectDSTypeSpecificDataSourcesSlot();

    void deselectAllLinesSlot();
    void selectSpecificLineSlot();

    void toogleShowCountsSlot();

private:
    enum class ItemType
    {
        DataSourceType = 0,
        DataSource,
        DBContent
    };

    struct ItemInfo
    {
        int         id = -1;
        ItemType    type;
        std::string name;
    };

    void createUI();
    void createMenu();

    void clear();
    void createContent();
    void createDataSourceType(const std::string& ds_type_name);
    void createDataSource(QTreeWidgetItem* parent_item, const dbContent::DBDataSource& data_source);
    void createDBContent(QTreeWidgetItem* parent_item);

    DataSourceManager& ds_man_;

    QMenu edit_menu_;

    QLabel* ts_min_label_{nullptr};
    QLabel* ts_max_label_{nullptr};
    QLabel* associations_label_{nullptr};

    QTreeWidget* tree_widget_ = nullptr;

    std::map<std::string, QTreeWidgetItem*> ds_type_items_;
    std::map<std::string, QTreeWidgetItem*> ds_items_;

    std::map<int, ItemInfo> item_infos_;

    int item_ids_ = 0;
};

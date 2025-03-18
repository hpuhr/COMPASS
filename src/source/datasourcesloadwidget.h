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
#include <QPushButton>

#include <map>

class DataSourceManager;

class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

namespace dbContent
{
    class DBDataSource;
}

/**
 */
class DataSourcesWidget : public QWidget
{
    Q_OBJECT
public:
    DataSourcesWidget(DataSourceManager& ds_man, bool show_counts);
    virtual ~DataSourcesWidget();

    void updateContent(bool recreate_required = false);
    void loadingDone();

    virtual void setUseDSType(const std::string& ds_type_name, bool use);
    virtual bool getUseDSType(const std::string& ds_type_name) const;
    virtual void setUseDS(unsigned int ds_id, bool use);
    virtual bool getUseDS(unsigned int ds_id) const;
    virtual void setUseDSLine(unsigned int ds_id, unsigned int ds_line, bool use);
    virtual bool getUseDSLine(unsigned int ds_id, unsigned int ds_line) const;

    DataSourceManager& dsManager() { return ds_man_; }

public slots:
    void loadDSTypeChangedSlot();

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

    QWidget* createLinesWidget(unsigned int ds_id);

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

    bool show_counts_ = true;
};

/**
 */
class DataSourceLineButton : public QPushButton
{
public:
    DataSourceLineButton(DataSourcesWidget* widget,
                         unsigned int ds_id, 
                         unsigned int line_id,
                         unsigned int button_size_px);

    void updateContent();

    unsigned int dsID() const { return ds_id_; }
    unsigned int lineID() const { return line_id_; }

private:
    DataSourcesWidget* widget_ = nullptr;
    unsigned int       ds_id_;
    unsigned int       line_id_;
    std::string        line_str_;
};

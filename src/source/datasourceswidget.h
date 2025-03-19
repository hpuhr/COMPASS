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
#include <QTreeWidgetItem>

#include <map>

class DataSourceManager;

class QLabel;
class QTreeWidget;

namespace dbContent
{
    class DBDataSource;
}

class DataSourcesWidget;

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

    const dbContent::DBDataSource* ds_ = nullptr;
};

/**
 */
class DataSourcesWidgetItem : public QTreeWidgetItem
{
public:
    enum class Type
    {
        DataSourceType = 0,
        DataSource,
        DataSourceCount
    };

    DataSourcesWidgetItem(DataSourcesWidget* widget,
                          DataSourcesWidgetItem* parent,
                          Type type);
    virtual ~DataSourcesWidgetItem() = default;

    Type type() const { return type_; }

    virtual void init() 
    { 
        init_impl(); 
        updateContent(); 
    }
    virtual void updateContent() = 0;

protected:
    virtual void init_impl() {}

    void setItemWidget(int column, QWidget* w);

    DataSourcesWidget*     widget_ = nullptr;
    DataSourcesWidgetItem* parent_ = nullptr;

    Type type_;
};

/**
 */
class DataSourceTypeItem : public DataSourcesWidgetItem
{
public:
    DataSourceTypeItem(DataSourcesWidget* widget,
                       DataSourcesWidgetItem* parent,
                       const std::string& ds_type);
    virtual ~DataSourceTypeItem() = default;

    const std::string& dsType() const { return ds_type_; }

    void updateContent() override final;

private:
    std::string ds_type_;
};

/**
 */
class DataSourceItem : public DataSourcesWidgetItem
{
public:
    DataSourceItem(DataSourcesWidget* widget,
                   DataSourcesWidgetItem* parent,
                   unsigned int ds_id);

    virtual ~DataSourceItem() = default;

    unsigned int dsID() const { return ds_id_; }
    const dbContent::DBDataSource* dataSource() const { return ds_; }

    void updateContent() override final;

protected:
    void init_impl() override final;

private:
    QWidget* createLinesWidget();

    unsigned int                   ds_id_;
    const dbContent::DBDataSource* ds_ = nullptr;

    std::vector<DataSourceLineButton*> line_buttons_;
};

/**
 */
class DataSourceCountItem : public DataSourcesWidgetItem
{
public:
    DataSourceCountItem(DataSourcesWidget* widget,
                        DataSourcesWidgetItem* parent,
                        unsigned int ds_id,
                        const std::string& dbc_name);

    virtual ~DataSourceCountItem() = default;

    unsigned int dsID() const { return ds_id_; }
    const std::string& dbContentName() const { return dbc_name_; }

    void updateContent() override final;

private:
    QWidget* createLinesWidget();

    unsigned int                   ds_id_;
    std::string                    dbc_name_;
    const dbContent::DBDataSource* ds_ = nullptr;
};

/**
 */
class DataSourcesWidget : public QWidget
{
    Q_OBJECT
public:
    DataSourcesWidget(DataSourceManager& ds_man);
    virtual ~DataSourcesWidget();

    void updateContent(bool recreate_required = false);
    void loadingDone();

    virtual void setUseDSType(const std::string& ds_type_name, bool use);
    virtual bool getUseDSType(const std::string& ds_type_name) const;
    virtual void setUseDS(unsigned int ds_id, bool use);
    virtual bool getUseDS(unsigned int ds_id) const;
    virtual void setUseDSLine(unsigned int ds_id, unsigned int ds_line, bool use);
    virtual bool getUseDSLine(unsigned int ds_id, unsigned int ds_line) const;
    virtual void setShowCounts(bool show) const;
    virtual bool getShowCounts() const;

    DataSourceManager& dsManager() { return ds_man_; }

    static const int LineButtonSize;

private:
    friend class DataSourcesWidgetItem;

    void createUI();
    void createMenu();

    void clear();
    void createContent();
    void createDataSourceType(const std::string& ds_type_name);
    void createDataSource(DataSourcesWidgetItem* parent_item, 
                          const dbContent::DBDataSource& data_source);
    void createDBContent(DataSourcesWidgetItem* parent_item,
                         const dbContent::DBDataSource& data_source,
                         const std::string& dbc_name);

    void itemChanged(QTreeWidgetItem *item, int column);

    void updateAllContent();
    void updateAdditionalInfo();

    void editClicked();

    void selectAllDSTypes();
    void deselectAllDSTypes();
    void selectAllDataSources();
    void deselectAllDataSources();
    void selectDSTypeSpecificDataSources();
    void deselectDSTypeSpecificDataSources();
    void deselectAllLines();
    void selectSpecificLines();
    void toogleShowCounts();

    DataSourceManager& ds_man_;

    QMenu edit_menu_;

    QLabel* ts_min_label_{nullptr};
    QLabel* ts_max_label_{nullptr};
    QLabel* associations_label_{nullptr};

    QTreeWidget* tree_widget_ = nullptr;

    bool shows_counts_ = false;
};

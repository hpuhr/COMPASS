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

#include "task/result/report/sectioncontent.h"

#include "json.hpp"

#include <QVariant>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QBrush>
#include <QTimer>
#include <QWidget>

#include <vector>
#include <functional>

#include <boost/optional.hpp>

class TaskManager;
class Section;

class ViewableDataConfig;

namespace Utils
{
    class StringTable;
}

class QPushButton;
class QTableView;
class QMenu;

namespace ResultReport
{

/**
 */
class TableQSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    TableQSortFilterProxyModel(QObject* parent=nullptr)
        : QSortFilterProxyModel(parent) {}

    bool showUnused() const
    {
        return show_unused_;
    }

    void showUnused(bool value)
    {
        //emit layoutAboutToBeChanged();

        show_unused_ = value;

        //emit layoutChanged();
    }

protected:
    bool show_unused_ {true};

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        //return sourceModel()->row(source_row)[0].checkState() == Qt::Checked;

        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

        if (show_unused_)
            return true;
        else
            return (sourceModel()->data(index, Qt::BackgroundRole) != QBrush(Qt::lightGray));
    }
};

class SectionContentTableWidget;

/**
 */
class SectionContentTable : public SectionContent
{
public:
    /**
     */
    struct RowInfo
    {
        bool enabled          = true;
        bool has_context_menu = true;
    };

    typedef std::function<RowInfo(unsigned int)>      RowInfoCallback;
    typedef std::function<bool(QMenu*, unsigned int)> RowContextMenuCallback;

    SectionContentTable(unsigned int id,
                        const std::string& name, 
                        unsigned int num_columns,
                        const std::vector<std::string>& headings, 
                        Section* parent_section, 
                        bool sortable=true, 
                        unsigned int sort_column=0, 
                        Qt::SortOrder sort_order=Qt::AscendingOrder);
    SectionContentTable(Section* parent_section);
    virtual ~SectionContentTable();

    void addRow (const nlohmann::json::array_t& row,
                 const SectionContentViewable& viewable = SectionContentViewable(),
                 const std::string& section_link = "",
                 const std::string& section_figure = "", // in section_link section
                 const QVariant& viewable_index = QVariant());
    
    virtual void addToLayout (QVBoxLayout* layout) override;
    virtual void accept(LatexVisitor& v) override;

    size_t numRows() const;
    size_t numColumns() const;
    const std::vector<std::string>& headings() const;
    unsigned int filteredRowCount () const;
    std::vector<std::string> sortedRowStrings(unsigned int row, bool latex=true) const;

    bool hasReference (unsigned int row) const;
    std::string reference (unsigned int row) const;

    bool showUnused() const;
    void showUnused(bool value);

    void registerCallBack(const std::string& name, const std::function<void()>& func);
    void setRowInfoCallback(const RowInfoCallback& func);
    void setRowContextMenuCallback(const RowContextMenuCallback& func);

    QVariant data(const QModelIndex& index, int role) const;
    
    void clicked(unsigned int row);
    void doubleClicked(unsigned int row);
    void customContextMenu(unsigned int row, const QPoint& pos);
    void addActionsToMenu(QMenu* menu);

    Utils::StringTable toStringTable() const;
    nlohmann::json toJSON(bool rowwise = true,
                          const std::vector<int>& cols = std::vector<int>()) const;

    static const std::string FieldHeadings;
    static const std::string FieldSortable;
    static const std::string FieldSortColumn;
    static const std::string FieldSortOrder;
    static const std::string FieldRows;
    static const std::string FieldAnnotations;

    static const std::string FieldAnnoFigureID;
    static const std::string FieldAnnoSectionLink;
    static const std::string FieldAnnoSectionFigure;
    static const std::string FieldAnnoIndex;

protected:
    void toJSON_impl(nlohmann::json& root_node) const override final;
    bool fromJSON_impl(const nlohmann::json& j) override final;

    bool loadOnDemand() override final;

    unsigned int addFigure (const SectionContentViewable& viewable);

    SectionContentTableWidget* tableWidget() const;

    void toggleShowUnused();
    void copyContent();
    
    void executeCallback(const std::string& name);

    RowInfo rowInfo(unsigned int row) const;

    unsigned int num_columns_ {0};
    std::vector<std::string> headings_;

    bool          sortable_     {true};
    unsigned int  sort_column_  {0};
    Qt::SortOrder sort_order_   {Qt::AscendingOrder};
    unsigned int  check_column_ {0};

    bool show_unused_ {false};

    /**
     * Describes figures and links attached to a table row.
     */
    struct RowAnnotation
    {
        boost::optional<unsigned int> figure_id;      //content id of a figure in the containing section
        std::string                   section_link;   //link to another section
        std::string                   section_figure; //figure in the linked section
        QVariant                      index;          //detail index
    };

    mutable std::vector<nlohmann::json> rows_;
    mutable std::vector<RowAnnotation>  annotations_;

    mutable SectionContentTableWidget* table_widget_ {nullptr};

    std::map<std::string, std::function<void()>> callback_map_;
    RowInfoCallback                              row_info_callback_;
    RowContextMenuCallback                       row_contextmenu_callback_;
};

/**
 */
class SectionContentTableModel : public QAbstractItemModel
{
public:
    SectionContentTableModel(SectionContentTable* content_table, QObject* parent = nullptr);
    virtual ~SectionContentTableModel() = default;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void executeAndReset(const std::function<void()>& func);

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    SectionContentTable* content_table_ = nullptr;
};

/**
 */
class SectionContentTableWidget : public QWidget
{
public:
    SectionContentTableWidget(SectionContentTable* content_table, 
                              bool show_unused,
                              int sort_column = -1,
                              Qt::SortOrder sort_order = Qt::AscendingOrder,
                              QWidget* parent = nullptr);
    virtual ~SectionContentTableWidget();

    SectionContentTableModel* itemModel();
    const SectionContentTableModel* itemModel() const;
    TableQSortFilterProxyModel* proxyModel();
    const TableQSortFilterProxyModel* proxyModel() const;
    QTableView* tableView();
    const QTableView* tableView() const;

    void showUnused(bool show);
    void resizeColumns();

    int fromProxy(int proxy_row) const;

    std::vector<std::string> sortedRowStrings(unsigned int row, bool latex) const;

    static const int DoubleClickCheckIntervalMSecs;

private:
    void clicked(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void customContextMenu(const QPoint& p);

    void performClickAction();

    SectionContentTable*        content_table_  = nullptr;
    SectionContentTableModel*   model_          = nullptr;
    TableQSortFilterProxyModel* proxy_model_    = nullptr;
    QTableView*                 table_view_     = nullptr;
    QPushButton*                options_button_ = nullptr;
    QMenu*                      options_menu_   = nullptr;

    QTimer click_action_timer_;
    boost::optional<unsigned int> last_clicked_row_index_;
};

}

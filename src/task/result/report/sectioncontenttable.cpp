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

#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/report.h"

#include "taskmanager.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "latexvisitor.h"

#include "logger.h"
#include "stringconv.h"
#include "stringmat.h"
#include "asynctask.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QInputDialog>
#include <QThread>

#include <cassert>
#include <type_traits>
#include <iostream>

namespace ResultReport
{

/***************************************************************************************************
 * SectionContentTable
 ***************************************************************************************************/

const std::string SectionContentTable::FieldHeadings     = "headings";
const std::string SectionContentTable::FieldSortable     = "sortable";
const std::string SectionContentTable::FieldSortColumn   = "sort_column";
const std::string SectionContentTable::FieldSortOrder    = "order";
const std::string SectionContentTable::FieldRows         = "rows";
const std::string SectionContentTable::FieldAnnotations  = "annotations";

const std::string SectionContentTable::FieldAnnoFigureID      = "figure_id";
const std::string SectionContentTable::FieldAnnoSectionLink   = "section_link";
const std::string SectionContentTable::FieldAnnoSectionFigure = "section_figure";
const std::string SectionContentTable::FieldAnnoIndex         = "index";

/**
 */
SectionContentTable::SectionContentTable(unsigned int id,
                                         const std::string& name,
                                         unsigned int num_columns,
                                         const std::vector<std::string>& headings,
                                         Section* parent_section,
                                         bool sortable,
                                         unsigned int sort_column,
                                         Qt::SortOrder sort_order)
:   SectionContent(Type::Table, id, name, parent_section)
,   num_columns_  (num_columns)
,   headings_     (headings)
,   sortable_     (sortable)
,   sort_column_  (sort_column)
,   sort_order_   (sort_order)
{
}

/**
 */
SectionContentTable::SectionContentTable(Section* parent_section)
:   SectionContent(Type::Table, parent_section)
{
}

/**
 */
SectionContentTable::~SectionContentTable() = default;

/**
 */
void SectionContentTable::addRow (const nlohmann::json::array_t& row,
                                  const SectionContentViewable& viewable,
                                  const std::string& section_link,
                                  const std::string& section_figure,
                                  const QVariant& viewable_index)
{
    assert (row.size() == num_columns_);

    rows_.push_back(row);

    //configure attached annotation
    RowAnnotation anno;
    anno.index          = viewable_index;
    anno.section_link   = section_link;
    anno.section_figure = section_figure;

    logdbg<< "SectionContentTable " << name_ << ": addRow: viewable.valid " << viewable.valid();

    if (viewable.valid())
    {
        //add figure to containing section and remember id
        anno.figure_id = addFigure(viewable);
    }

    annotations_.push_back(anno);
}

/**
 */
unsigned int SectionContentTable::addFigure(const SectionContentViewable& viewable)
{
    assert(parent_section_);
    return parent_section_->addContentFigure(viewable);
}

/**
 */
SectionContentTableWidget* SectionContentTable::tableWidget() const
{
    if (table_widget_)
        return table_widget_;

    //generate widget
    SectionContentTable* tmp = const_cast<SectionContentTable*>(this); // hacky
    table_widget_ = new SectionContentTableWidget(tmp, 
                                                  show_unused_, 
                                                  sortable_ ? sort_column_ : -1, 
                                                  sort_order_);
    return table_widget_;
}

/**
 */
void SectionContentTable::addToLayout(QVBoxLayout* layout)
{
    loginf << "SectionContentTable: addToLayout";

    assert (layout);

    //add widget to layout
    auto widget = tableWidget();
    layout->addWidget(widget);
}

/**
 */
void SectionContentTable::accept(LatexVisitor& v)
{
    loginf << "SectionContentTable: accept";

    createOnDemandIfNeeded();

    //@TODO:
    //v.visit(this);
}

namespace
{
    /**
     * Converts a json value to a QVariant for display purpose.
     */
    QVariant qVariantFromJSON(const nlohmann::json& j)
    {
        if (j.is_boolean())
            return QVariant::fromValue(j.get<bool>());
        else if (j.is_number_integer())
            return QVariant::fromValue(j.get<qint64>());  // or int, depending on your use case
        else if (j.is_number_unsigned())
            return QVariant::fromValue(j.get<quint64>());
        else if (j.is_number_float())
            return QVariant::fromValue(j.get<double>());
        else if (j.is_string())
            return QVariant::fromValue(QString::fromStdString(j.get<std::string>()));
        else if (j.is_null())
            return QVariant();
        else
            return QVariant();
    };
}

/**
 */
QVariant SectionContentTable::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        logdbg << "SectionContentTable: data: display role: row " << index.row() << " col " << index.column();

        assert (index.row() >= 0);
        assert (index.row() < (int)rows_.size());
        assert (index.column() < (int)num_columns_);

        return qVariantFromJSON(rows_.at(index.row()).at(index.column()));
    }
    case Qt::BackgroundRole:
    {
        assert (index.row() >= 0);
        assert (index.row() < (int)rows_.size());

        unsigned int row_index = index.row();

        auto info = rowInfo(row_index);

        if (!info.enabled)
            return QBrush(Qt::lightGray);
        else
            return QVariant();
    }
    case Qt::ForegroundRole:
    {
        assert (index.row() >= 0);
        assert (index.row() < (int)rows_.size());
        assert (index.column() < (int)num_columns_);

        QVariant data = qVariantFromJSON(rows_.at(index.row()).at(index.column()));

        if (data.userType() == QMetaType::QString)
        {
            if (data == "Passed")
                return QVariant(QColor(Qt::darkGreen));
            else if (data == "Failed")
                return QVariant(QColor(Qt::red));
        }
        if (data.userType() == QMetaType::Bool)
        {
            if (data == true)
                return QVariant(QColor(Qt::darkGreen));
            else if (data == false)
                return QVariant(QColor(Qt::red));
        }
        return QVariant(QColor(Qt::black));
    }
    default:
    {
        return QVariant();
    }
    }
}

/**
 */
size_t SectionContentTable::numRows() const
{
    return rows_.size();
}

/**
 */
size_t SectionContentTable::numColumns() const
{
    return num_columns_;
}

/**
 */
const std::vector<std::string>& SectionContentTable::headings() const
{
    return headings_;
}

/**
 */
unsigned int SectionContentTable::filteredRowCount () const
{
    return tableWidget()->proxyModel()->rowCount();
}

/**
 */
std::vector<std::string> SectionContentTable::sortedRowStrings(unsigned int row, bool latex) const
{
    return tableWidget()->sortedRowStrings(row, latex);
}

/**
 */
bool SectionContentTable::hasReference (unsigned int row) const
{
    //obtain original data row
    unsigned int row_index = tableWidget()->fromProxy(row);

    const auto& annotation = annotations_.at(row_index);

    return !annotation.section_link.empty();
}

/**
 */
std::string SectionContentTable::reference(unsigned int row) const
{
    //obtain original data row
    unsigned int row_index = tableWidget()->fromProxy(row);

    const auto& annotation = annotations_.at(row_index);

    return annotation.section_link;
}

/**
 */
bool SectionContentTable::showUnused() const
{
    return show_unused_;
}

/**
 */
void SectionContentTable::showUnused(bool value)
{
    loginf << "SectionContentTable: showUnused: value " << value;

    tableWidget()->showUnused(value);

    show_unused_ = value;
}


/**
 */
void SectionContentTable::registerCallBack (const std::string& name, const std::function<void()>& func)
{
    assert (!callback_map_.count(name));
    callback_map_.emplace(name, func);
}

/**
 */
void SectionContentTable::executeCallback(const std::string& name)
{
    assert (callback_map_.count(name));
    callback_map_.at(name)();
}

/**
 */
void SectionContentTable::setRowInfoCallback(const RowInfoCallback& func)
{
    row_info_callback_ = func;
}

/**
 */
void SectionContentTable::setRowContextMenuCallback(const RowContextMenuCallback& func)
{
    row_contextmenu_callback_ = func;
}

/**
 */
SectionContentTable::RowInfo SectionContentTable::rowInfo(unsigned int row) const
{
    return row_info_callback_ ? row_info_callback_(row) : RowInfo();
}

/**
 */
void SectionContentTable::setCreateOnDemand(std::function<void(void)> create_on_demand_fnc,
                                            bool write_on_demand_content)
{
    create_on_demand_          = true;
    create_on_demand_fnc_      = create_on_demand_fnc;
    already_created_by_demand_ = false;
    write_on_demand_           = write_on_demand_content;   
}

/**
 */
bool SectionContentTable::isOnDemand() const
{
    return create_on_demand_;
}

/**
 */
void SectionContentTable::createOnDemand() const
{
    assert(create_on_demand_fnc_);

    //creation func
    auto func = [ = ] ()
    {
        this->create_on_demand_fnc_();
        this->already_created_by_demand_ = true;
    };

    tableWidget()->itemModel()->executeAndReset(func);
}

/**
 */
bool SectionContentTable::hasBeenCreatedOnDemand() const
{
    return create_on_demand_ && already_created_by_demand_;
}

/**
 */
void SectionContentTable::createOnDemandIfNeeded() const
{
    if (create_on_demand_ && !already_created_by_demand_)
    {
        logdbg << "SectionContentTable: createOnDemandIfNeeded: creating";

        QApplication::setOverrideCursor(Qt::WaitCursor);

        createOnDemand();

        //adapt table widget columns to new content?
        if (table_widget_)
            table_widget_->resizeColumns();

        QApplication::restoreOverrideCursor();
    }
}

/**
 */
void SectionContentTable::resetOnDemandContent() const
{
    rows_.clear();
    annotations_.clear();

    already_created_by_demand_ = false;
}

/**
 */
void SectionContentTable::clicked(unsigned int row)
{
    const auto& annotation = annotations_.at(row);

    //obtain figure from annotation
    bool has_valid_link = false;
    SectionContentFigure* figure = nullptr;

    if (annotation.figure_id.has_value())
    {
        loginf << "SectionContentTable: clicked: index has associated viewable via id " << annotation.figure_id.value();
        has_valid_link = true;

        //figure from content in parent section
        auto c = parent_section_->retrieveContent(annotation.figure_id.value());
        figure = dynamic_cast<SectionContentFigure*>(c.get());
    }
    else if (!annotation.section_link.empty() && !annotation.section_figure.empty())
    {
        loginf << "SectionContentTable: clicked: index has associated viewable via" 
               << " section '" << annotation.section_link << "'"
               << " figure '" << annotation.section_figure << "'";
        has_valid_link = true;

        //figure from section link + figure name
        auto& section = report_->getSection(annotation.section_link);

        if (section.hasFigure(annotation.section_figure))
            figure = &section.getFigure(annotation.section_figure);
    }

    //valid figure link found?
    if (has_valid_link)
    {
        if (figure)
        {
            std::shared_ptr<nlohmann::json::object_t> viewable = figure->viewableContent();

            //TODO: find a pattern to support on-demand computation of viewable data
            // if (result_ptrs_.at(row_index)->viewableDataReady())
            // {
            //     //view data ready, just get it
            //     viewable = result_ptrs_.at(row_index)->viewableData(*this, annotations_.at(row_index));
            // }
            // else
            // {
            //     //recompute async and show wait dialog, this may take a while...
            //     auto func = [ & ] (const AsyncTaskState& state, AsyncTaskProgressWrapper& progress)
            //     {
            //         viewable = result_ptrs_.at(row_index)->viewableData(*this, annotations_.at(row_index));
            //         return Result::succeeded();
            //     };

            //     AsyncFuncTask task(func, "Updating Contents", "Updating contents...", false);
            //     task.runAsyncDialog();
            // }

            assert (viewable);

            //show viewable
            report_->setCurrentViewable(*viewable.get());
        }
        else
        {
            logerr << "SectionContentTable: clicked: figure could not be retrieved";
        }
    }
}

/**
 */
void SectionContentTable::doubleClicked(unsigned int row)
{
    const auto& annotation = annotations_.at(row);

    //link to other section stored in row?
    if (!annotation.section_link.empty())
    {
        loginf << "SectionContentTable: doubleClicked: index has associated reference '"
               << annotation.section_link << "'";

        //jump to row link
        report_->setCurrentSection(annotation.section_link);
    }
    else
    {
        loginf << "SectionContentTable: doubleClicked: index has no associated reference";
    }
}

/**
 */
void SectionContentTable::customContextMenu(unsigned int row, const QPoint& pos)
{
    if (row_contextmenu_callback_ && rowInfo(row).has_context_menu)
    {
        QMenu menu;

        //configure menu via callback
        bool use_menu = row_contextmenu_callback_(&menu, row);

        //show menu if valid
        if (use_menu && menu.actions().size() > 0)
            menu.exec(pos);
    }
}

/**
 */
void SectionContentTable::addActionsToMenu(QMenu* menu)
{
    //add general actions
    QAction* unused_action = menu->addAction("Toggle Show Unused");
    QObject::connect (unused_action, &QAction::triggered, [ this ] { this->toggleShowUnused(); });

    QAction* copy_action = menu->addAction("Copy Content");
    QObject::connect (copy_action, &QAction::triggered, [ this ] { this->copyContent(); });

    //add custom callbacks
    if (callback_map_.size() > 0)
    {
        menu->addSeparator();

        for (auto& cb_it : callback_map_)
        {
            QAction* action = menu->addAction(cb_it.first.c_str());
            QObject::connect(action, &QAction::triggered, [ = ] () { this->executeCallback(cb_it.first); });
        }
    }
}

/**
 */
void SectionContentTable::toggleShowUnused()
{
    showUnused(!show_unused_);
}

/**
 */
void SectionContentTable::copyContent()
{
    loginf << "SectionContentTable: copyContent";

    std::stringstream ss;

    unsigned int num_cols = headings_.size();

    // headings
    for (unsigned int cnt=0; cnt < num_cols; ++cnt)
    {
        if (cnt == 0)
            ss << headings_.at(cnt);
        else
            ss <<  ";" << headings_.at(cnt);
    }
    ss << "\n";

    auto proxy_model = tableWidget()->proxyModel();

    unsigned int num_rows = proxy_model->rowCount();

    std::vector<std::string> row_strings;

    for (unsigned int row=0; row < num_rows; ++row)
    {
        row_strings = sortedRowStrings(row, false);
        assert (row_strings.size() == num_cols);

        for (unsigned int cnt=0; cnt < num_cols; ++cnt)
        {
            if (cnt == 0)
                ss << row_strings.at(cnt);
            else
                ss <<  ";" << row_strings.at(cnt);
        }
        ss << "\n";
    }

    QApplication::clipboard()->setText(ss.str().c_str());
}

/**
 */
Utils::StringTable SectionContentTable::toStringTable() const
{
    return Utils::StringTable(tableWidget()->itemModel());
}

/**
 */
nlohmann::json SectionContentTable::toJSON(bool rowwise,
                                           const std::vector<int>& cols) const
{
    return toStringTable().toJSON(rowwise, cols);
}

/**
 */
void SectionContentTable::toJSON_impl(nlohmann::json& root_node) const
{
    root_node[ FieldHeadings    ] = headings_;
    root_node[ FieldSortable    ] = sortable_;
    root_node[ FieldSortColumn  ] = sort_column_;
    root_node[ FieldSortOrder   ] = sort_order_ == Qt::AscendingOrder ? "ascending" : "descending";
    root_node[ FieldRows        ] = std::vector<nlohmann::json>();
    root_node[ FieldAnnotations ] = nlohmann::json::array();

    //write content if either not cod or we explicitely want to write cod content
    if (!create_on_demand_ || write_on_demand_)
    {
        bool reset_content = false;

        //load on demand content if not yet loaded
        if (create_on_demand_ && write_on_demand_ && !already_created_by_demand_)
        {
            createOnDemandIfNeeded();
            reset_content = true; // we want to forget the loaded content afterwards
        }

        //write rows
        root_node[ FieldRows ] = rows_;

        //write annotations
        nlohmann::json j_annos = nlohmann::json::array();

        for (const auto& a : annotations_)
        {
            nlohmann::json j_anno;

            if (a.figure_id.has_value())
                j_anno[ FieldAnnoFigureID ] = a.figure_id.value();

            j_anno[ FieldAnnoSectionLink   ] = a.section_link;
            j_anno[ FieldAnnoSectionFigure ] = a.section_figure;

            j_annos.push_back(j_anno);
        }

        root_node[ FieldAnnotations ] = j_annos;

        //reset previously cod-loaded content?
        if (reset_content)
            resetOnDemandContent();
    }
}

/**
 */
bool SectionContentTable::fromJSON_impl(const nlohmann::json& j)
{
    if (!j.is_object()               ||
        !j.contains(FieldHeadings)   ||
        !j.contains(FieldSortable)   ||
        !j.contains(FieldSortColumn) ||
        !j.contains(FieldSortOrder)  ||
        !j.contains(FieldRows)       ||
        !j.contains(FieldAnnotations))
    {
        logerr << "SectionContentTable: fromJSON: Error: Section content table does not obtain needed fields";
        return false;
    }

    headings_    = j[ FieldHeadings   ].get<std::vector<std::string>>();
    sortable_    = j[ FieldSortable   ];
    sort_column_ = j[ FieldSortColumn ];

    std::string sort_order = j[ FieldSortOrder ];
    sort_order_ = sort_order == "ascending" ? Qt::AscendingOrder : Qt::DescendingOrder;

    num_columns_ = headings_.size();

    //no create on demand on read
    create_on_demand_          = false;
    already_created_by_demand_ = false;
    write_on_demand_           = false;
    create_on_demand_fnc_      = {};

    rows_ = j[ FieldRows ].get<std::vector<nlohmann::json>>();

    auto& j_annos = j[ FieldAnnotations ];
    if (!j_annos.is_array() || j_annos.size() != rows_.size())
    {
        logerr << "SectionContentTable: fromJSON: Error: Annotation array invalid";
        return false;
    }

    for (const auto& j_anno : j_annos)
    {
        if (!j_anno.contains(FieldAnnoSectionLink) ||
            !j_anno.contains(FieldAnnoSectionFigure))
        {
            logerr << "SectionContentTable: fromJSON: Error: Could not read annotation";
            return false;
        }

        RowAnnotation anno;
        anno.section_link   = j_anno[ FieldAnnoSectionLink   ];
        anno.section_figure = j_anno[ FieldAnnoSectionFigure ];

        if (j_anno.contains(FieldAnnoFigureID))
        {
            unsigned int id = j_anno[ FieldAnnoFigureID ];
            anno.figure_id = id;
        }

        annotations_.push_back(anno);
    }

    assert(rows_.size() == annotations_.size());

    return true;
}

/***************************************************************************************************
 * SectionContentTableModel
 ***************************************************************************************************/

/**
 */
SectionContentTableModel::SectionContentTableModel(SectionContentTable* content_table, QObject* parent)
:   QAbstractItemModel(parent)
,   content_table_    (content_table)
{
    assert(content_table_);
}

/**
 */
QVariant SectionContentTableModel::data(const QModelIndex& index, int role) const
{
    return content_table_->data(index, role);
}

/**
 */
QVariant SectionContentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < (int)content_table_->numColumns());
        return content_table_->headings().at(section).c_str();
    }

    return QVariant();
}

/**
 */
QModelIndex SectionContentTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

/**
 */
int SectionContentTableModel::rowCount(const QModelIndex& parent) const
{
    return (int)content_table_->numRows();
}

/**
 */
int SectionContentTableModel::columnCount(const QModelIndex& parent) const
{
    return (int)content_table_->numColumns();
}

/**
 */
QModelIndex SectionContentTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

/**
 */
Qt::ItemFlags SectionContentTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < (int)content_table_->headings().size());

    return QAbstractItemModel::flags(index);
}

/**
 */
bool SectionContentTableModel::canFetchMore(const QModelIndex &parent) const
{
    return content_table_->isOnDemand() && !content_table_->hasBeenCreatedOnDemand();
}

/**
 */
void SectionContentTableModel::fetchMore(const QModelIndex &parent)
{
    return content_table_->createOnDemandIfNeeded();
}

/**
 */
void SectionContentTableModel::executeAndReset(const std::function<void()>& func)
{
    if (!func)
        return;

    beginResetModel();

    func();

    endResetModel();
}

/***************************************************************************************************
 * SectionContentTableWidget
 ***************************************************************************************************/

const int SectionContentTableWidget::DoubleClickCheckIntervalMSecs = 300;

/**
 */
SectionContentTableWidget::SectionContentTableWidget(SectionContentTable* content_table, 
                                                     bool show_unused,
                                                     int sort_column,
                                                     Qt::SortOrder sort_order,
                                                     QWidget* parent)
:   QWidget       (parent       )
,   content_table_(content_table)
{
    assert(content_table_);

    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);

    QHBoxLayout* upper_layout = new QHBoxLayout();

    upper_layout->addWidget(new QLabel(("Table: " + content_table_->name()).c_str()));
    upper_layout->addStretch();

    options_menu_ = new QMenu(options_button_);
    content_table_->addActionsToMenu(options_menu_);

    options_button_ = new QPushButton("Options");
    options_button_->setMenu(options_menu_);

    upper_layout->addWidget(options_button_);

    main_layout->addLayout(upper_layout);

    table_view_ = new QTableView;

    model_ = new SectionContentTableModel(content_table, this);

    proxy_model_ = new TableQSortFilterProxyModel(this);
    proxy_model_->showUnused(show_unused);
    proxy_model_->setSourceModel(model_);

    table_view_->setModel(proxy_model_);

    if (sort_column >= 0)
    {
        table_view_->setSortingEnabled(true);
        table_view_->sortByColumn(sort_column, sort_order);
    }

    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);

    table_view_->reset();

    connect(table_view_, &QTableView::customContextMenuRequested,
            this, &SectionContentTableWidget::customContextMenu);
    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &SectionContentTableWidget::clicked);
    connect(table_view_, &QTableView::pressed,
            this, &SectionContentTableWidget::clicked);
    connect(table_view_, &QTableView::doubleClicked,
            this, &SectionContentTableWidget::doubleClicked);
        
    //    if (num_columns_ > 5)
    //        table_view_->horizontalHeader()->setMaximumSectionSize(150);
    
    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();

    main_layout->addWidget(table_view_);

    click_action_timer_.setSingleShot(true);
    click_action_timer_.setInterval(DoubleClickCheckIntervalMSecs);

    QObject::connect(&click_action_timer_, &QTimer::timeout, this, &SectionContentTableWidget::performClickAction);
}

/**
 */
SectionContentTableWidget::~SectionContentTableWidget()
{
}

/**
 */
void SectionContentTableWidget::showUnused(bool show)
{
    auto func = [ this, show ] ()
    {
        this->proxy_model_->showUnused(show);
    };

    model_->executeAndReset(func);
}

/**
 */
void SectionContentTableWidget::resizeColumns()
{
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

/**
 */
SectionContentTableModel* SectionContentTableWidget::itemModel()
{
    return model_;
}

/**
 */
const SectionContentTableModel* SectionContentTableWidget::itemModel() const
{
    return model_;
}

/**
 */
TableQSortFilterProxyModel* SectionContentTableWidget::proxyModel()
{
    return proxy_model_;
}

/**
 */
const TableQSortFilterProxyModel* SectionContentTableWidget::proxyModel() const
{
    return proxy_model_;
}

/**
 */
QTableView* SectionContentTableWidget::tableView()
{
    return table_view_;
}

/**
 */
const QTableView* SectionContentTableWidget::tableView() const
{
    return table_view_;
}

/**
 * Returns the original data row from the given proxy model's row.
 */
int SectionContentTableWidget::fromProxy(int proxy_row) const
{
    assert (proxy_row < proxy_model_->rowCount());
    assert (proxy_row < (int)content_table_->numRows());

    QModelIndex index = proxy_model_->index(proxy_row, 0);
    assert (index.isValid());

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    unsigned int row_index = source_index.row();

    return row_index;
}

/**
 */
void SectionContentTableWidget::clicked(const QModelIndex& index)
{
    loginf << "SectionContentTableWidget: clicked";

    if (!index.isValid())
    {
        loginf << "SectionContentTableWidget: clicked: invalid index";
        return;
    }

    if (QApplication::mouseButtons() & Qt::RightButton)
    {
        loginf << "SectionContentTableWidget: clicked: RMB click ignored";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    last_clicked_row_index_ = source_index.row();

    //fire timer to perform delayed click action
    click_action_timer_.start();
}

/**
 */
void SectionContentTableWidget::performClickAction()
{
    loginf << "SectionContentTableWidget: performClickAction";

    //double click did not interrupt click action => perform
    if (!last_clicked_row_index_.has_value())
        return;

    unsigned int row_index = last_clicked_row_index_.value();
    last_clicked_row_index_.reset();

    //pass row to table
    content_table_->clicked(row_index);
}

/**
 */
void SectionContentTableWidget::doubleClicked(const QModelIndex& index)
{
    loginf << "SectionContentTableWidget: doubleClicked";

    //double click detected => interrupt any previously triggered click action
    click_action_timer_.stop();

    if (!index.isValid())
    {
        loginf << "SectionContentTableWidget: doubleClicked: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    loginf << "SectionContentTableWidget: doubleClicked: row " << source_index.row();

    unsigned int row_index = source_index.row();

    //pass row to table
    content_table_->doubleClicked(row_index);
}

/**
 */
void SectionContentTableWidget::customContextMenu(const QPoint& p)
{
    logdbg << "SectionContentTableWidget: customContextMenu";

    QModelIndex index = table_view_->indexAt(p);
    if (!index.isValid())
        return;

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    loginf << "SectionContentTableWidget: customContextMenu: row " << index.row() << " src " << source_index.row();

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    unsigned int row_index = source_index.row();

    auto pos = table_view_->viewport()->mapToGlobal(p);

    //pass row to table
    content_table_->customContextMenu(row_index, pos);
}

/**
 */
std::vector<std::string> SectionContentTableWidget::sortedRowStrings(unsigned int row, bool latex) const
{
    logdbg << "SectionContentTableWidget: sortedRowStrings: row " << row << " rows " << proxy_model_->rowCount()
           << " data rows " << content_table_->numRows();
    
    assert ((int)row < proxy_model_->rowCount());
    assert (row < content_table_->numRows());

    std::vector<std::string> result;

    size_t nc = content_table_->numColumns();

    for (size_t col=0; col < nc; ++col)
    {
        QModelIndex index = proxy_model_->index(row, col);
        assert (index.isValid());

        // get string can convert to latex
        if (latex)
            result.push_back(Utils::String::latexString(proxy_model_->data(index).toString().toStdString()));
        else
            result.push_back(proxy_model_->data(index).toString().toStdString());
    }

    return result;
}

}
